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
#include "main.h"
#include "wdt.h"


void WdtIsrHandler(void);

/* WDT counter configuration */
#define WDT_COUNT0_MATCH    (60000u)
#define WDT_COUNT1_MATCH    (40000u)


/* --------------------------- Local functions ------------------------------*/



void WdtInit()
{ 
    //CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER1, WdtIsrHandler);
    //CySysWdtSetIsrCallback(CY_SYS_WDT_COUNTER1, WdtIsrHandler);
    
    
    
    /* Set WDT counter 0 to generate reset on match */
    CySysWdtWriteMode(CY_SYS_WDT_COUNTER0, CY_SYS_WDT_MODE_RESET);
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER0, WDT_COUNT0_MATCH);
    CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER0, 1u);
    
    /* Setup ISR for interrupts at WDT counter 1 events. */
    WdtIsr_StartEx(WdtIsrHandler);    
    
#warning Issiaiskinti, kaip konfiguroti WDT1 is kodo, nes nevykdo interraptu
    /* Set WDT counter 1 to generate interrupt on match */
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_INT);
	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, WDT_COUNT1_MATCH);
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1u);
    
    CySysWdtEnableCounterIsr(CY_SYS_WDT_COUNTER1);
    
    /* Enable WDT counters 0 and 1 cascade */
	CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01);
    
    /* Enable WDT counters 0 and 1 */
    CySysWdtEnable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK);
    
    /* Lock WDT registers and try to disable WDT counters 0 and 1 */
	CySysWdtLock();
	CySysWdtDisable(CY_SYS_WDT_COUNTER0_MASK | CY_SYS_WDT_COUNTER1_MASK);
	CySysWdtUnlock();
}


void WdtIsrHandler()
{
    /* Clear interrupts state */
	CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER1_INT);
    WdtIsr_ClearPending();
}
/* [] END OF FILE */
