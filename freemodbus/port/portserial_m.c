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
 * File: $Id: portserial_m.c,v 1.60 2013/08/13 15:07:05 Armink add Master Functions $
 */

#include "port.h"

/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"
#include "mb_m.h"
#include "mbport.h"
#include "BleUART.h"

#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED
    
extern _portdat MbPort;
    
/* ----------------------- static functions ---------------------------------*/
static void prvvUARTTxReadyISR(void);
static void prvvUARTRxISR(void);


/* ----------------------- Start implementation -----------------------------*/
void vMBMasterPortSerialEnable(BOOL xRxEnable, BOOL xTxEnable)
{
    if( xRxEnable )
    {
        MASTER_RS485_RECEIVE_MODE;
        CyDelayUs(10);
        SCB_ENABLE_INTR_RX(SCB_INTR_RX_NOT_EMPTY);
    }
    else
    {
        MASTER_RS485_SEND_MODE;
        CyDelayUs(10);
        SCB_DISABLE_INTR_RX(SCB_INTR_RX_NOT_EMPTY);
    }

    if( xTxEnable )
    {
        SCB_ENABLE_INTR_TX(SCB_INTR_TX_UART_DONE);       
        SCB_SetTxInterrupt(SCB_INTR_TX_UART_DONE);
    }
    else
    {        
        SCB_DISABLE_INTR_TX(SCB_INTR_TX_UART_DONE);
    }    
}

void vMBMasterPortClose(void)
{
    SCB_DISABLE_INTR_TX(SCB_INTR_TX_UART_DONE);
    SCB_DISABLE_INTR_RX(SCB_INTR_RX_NOT_EMPTY);
    
    SCB_UartStop();
}

BOOL xMBMasterPortSerialInit(UCHAR ucPORT, ULONG ulBaudRate, eMBParity eParity )
{    
    ucPORT = ucPORT;
    ulBaudRate = ulBaudRate;
    eParity = eParity;
    
    if( UartStart( ) != CYBLE_ERROR_OK ) return FALSE;
    
    return TRUE;   
}

BOOL xMBMasterPortSerialPutByte(CHAR ucByte)
{
	SCB_UartPutChar(ucByte);
	return TRUE;
}

BOOL xMBMasterPortSerialGetByte(CHAR * pucByte)
{
	*pucByte = SCB_UartGetByte();
	return TRUE;
}

/* 
 * Create an interrupt handler for the transmit buffer empty interrupt
 * (or an equivalent) for your target processor. This function should then
 * call pxMBFrameCBTransmitterEmpty( ) which tells the protocol stack that
 * a new character can be sent. The protocol stack will then call 
 * xMBPortSerialPutByte( ) to send the character.
 */
void prvvUARTTxReadyISR(void)
{
	pxMBMasterFrameCBTransmitterEmpty();    // -> xMBMasterRTUTransmitFSM
}

/* 
 * Create an interrupt handler for the receive interrupt for your target
 * processor. This function should then call pxMBFrameCBByteReceived( ). The
 * protocol stack will then call xMBPortSerialGetByte( ) to retrieve the
 * character.
 */
void prvvUARTRxISR(void)
{
	pxMBMasterFrameCBByteReceived();        // -> xMBMasterRTUReceiveFSM
}


void MbSCB_IsrHandler(){  
    switch(SCB_GetInterruptCause())
    {        
        case SCB_INTR_CAUSE_TX: 
            if( SCB_GetTxInterruptSourceMasked() & SCB_INTR_TX_UART_DONE )
            {
                prvvUARTTxReadyISR();
            }
            
            SCB_ClearTxInterruptSource( SCB_GetTxInterruptSourceMasked() );
            break;
        case SCB_INTR_CAUSE_RX:         
            if( SCB_GetRxInterruptSourceMasked() & SCB_INTR_RX_NOT_EMPTY )
            {
                prvvUARTRxISR();
            }
            
            SCB_ClearRxInterruptSource( SCB_GetRxInterruptSourceMasked() ); 
            break;
    }    
}

#endif  //#if MB_MASTER_RTU_ENABLED > 0 || MB_MASTER_ASCII_ENABLED