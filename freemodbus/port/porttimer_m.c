/*
 * FreeModbus Libary: STM32 Port
 * Copyright (C) 2013 Armink <armink.ztl@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * File: $Id: porttimer_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions$
 */

/* ----------------------- Platform includes --------------------------------*/
#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED
    
/* ----------------------- Variables ----------------------------------------*/
uint16_t RespTimeoutValue = MB_MASTER_TIMEOUT_MS_RESPOND;
static USHORT usT35TimeOut50us;

/* ----------------------- static functions ---------------------------------*/
static void MasterPortTimerCallbackFunction(void);

/* ----------------------- Start implementation -----------------------------*/
BOOL xMBMasterPortTimersInit(USHORT usTimeOut50us)
{
    /* backup T35 ticks */
    usT35TimeOut50us = usTimeOut50us;
    
    mbPortTimer_SetPrescaler(mbPortTimer_PRESCALE_DIVBY128);
    mbPortTimer_WritePeriod(usT35TimeOut50us);    
        
    mbPortTimerISR_StartEx(MasterPortTimerCallbackFunction);
    mbPortTimerISR_ClearPending();
    vMBMasterPortTimersDisable();

    return TRUE;   
}

void vMBMasterPortTimersT35Enable()
{
	/* Set current timer mode,don't change it.*/
	vMBMasterSetCurTimerMode(MB_TMODE_T35);
    
    mbPortTimer_WriteCounter(0);
    mbPortTimer_SetPrescaler(mbPortTimer_PRESCALE_DIVBY128);
    mbPortTimer_WritePeriod(usT35TimeOut50us);
    
    mbPortTimerISR_Enable();
    
    mbPortTimer_Start();
}

void vMBMasterPortTimersConvertDelayEnable()
{
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_CONVERT_DELAY);
    
    mbPortTimer_WriteCounter(0);
    mbPortTimer_SetPrescaler( mbPortTimer_PRESCALE_DIVBY128 );
    mbPortTimer_WritePeriod( MB_MASTER_DELAY_MS_CONVERT * 176 / CommCLK_GetDividerRegister() );   
    
    mbPortTimerISR_Enable();
    
    mbPortTimer_Start();
}

void vMBMasterPortTimersRespondTimeoutEnable()
{
    /* Set current timer mode, don't change it.*/
    vMBMasterSetCurTimerMode(MB_TMODE_RESPOND_TIMEOUT);
    
    mbPortTimer_WriteCounter(0);
    mbPortTimer_SetPrescaler( mbPortTimer_PRESCALE_DIVBY128 );
    mbPortTimer_WritePeriod( RespTimeoutValue * 176 / CommCLK_GetDividerRegister() );    
    
    mbPortTimerISR_Enable();
    
    mbPortTimer_Start();
}

void vMBMasterPortTimersDisable()
{
    mbPortTimerISR_Disable();    
    mbPortTimer_Stop();
}

void prvvTIMERExpiredISR(void)
{
    mbPortTimerISR_Disable();    
	(void) pxMBMasterPortCBTimerExpired();
}

inline void MasterPortTimerCallbackFunction()
{
    mbPortTimerISR_ClearPending();
    prvvTIMERExpiredISR();
}

/* MbPort taimerio busena: true - aktyvus, false - neaktyvus */
bool vMBMasterPortTimerState(){

    if( mbPortTimer_ReadStatus() == mbPortTimer_STATUS_RUNNING ) return true;
    
    return false;
}

#endif