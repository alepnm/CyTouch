/* ========================================
 *
 * Copyright VENTMATIKA, 2016
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF Ventmatika.
 *
 * ========================================
*/
#include "systick.h"
#include "rtos.h"
#include "capsen.h"

/* ------------------------------- Defines ----------------------------------*/
#define SYSTICK_EACH_10_HZ  (1000u)
#define SYSTICK_RELOAD      (CYDEV_BCLK__SYSCLK__HZ / SYSTICK_EACH_10_HZ)

/* -------------------------------- Statics ---------------------------------*/
//static volatile uint32_t ticks;
register uint32_t ticks asm("r0");

/* Prototype of SysTick ISR */
static CY_ISR_PROTO(SysTickHandler);


/* ------------------------- Start implementation ---------------------------*/
void SysTickInit()
{
    /* Setup SysTick Timer */
    CySysTickStart();
    CySysTickSetReload(SYSTICK_RELOAD);
    CyIntSetSysVector((SysTick_IRQn + 16), SysTickHandler); 
}

uint32_t GetTicks()
{
    uint8_t isr;
    volatile uint32_t _ticks;
    
    isr = CyEnterCriticalSection();
    _ticks = ticks;
    CyExitCriticalSection(isr);
    
    return _ticks;
}


void SetTicks(uint32_t _ticks)
{
    uint8_t isr;

    isr = CyEnterCriticalSection();
    ticks = _ticks;
    CyExitCriticalSection(isr);
}


/*****************************************************************************/
/*                          SysTick Interrupt Handler                        */
/*****************************************************************************/
CY_ISR(SysTickHandler)
{
    ticks++;
    
    Shed_Tick();
    
    CapSenseProcess();
    
    Sound();
    Backlight();
}


/* [] END OF FILE */
