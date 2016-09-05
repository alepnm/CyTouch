/*******************************************************************************
* File Name: app_UART.h
*
* Description:
*  Contains the function prototypes and constants available to the example
*  project.
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#ifndef APP_UART_H
    
#define APP_UART_H

#include <project.h>  
#include <stdbool.h> 
//#include "common.h"
#include "mb.h"
#include "mb_m.h"

/***************************************
*       Constants
***************************************/
#define UART_IDLE_TIMEOUT   1000

typedef enum { SCB_OFF = 0, SCB_MBUS, SCB_BLE, SCB_I2C } SCB_OP_MODE;

#pragma pack(push,1)
typedef struct{
    SCB_OP_MODE Mode;          // UARTo darbo rezimas: BLE arba Modbus
    union{
        uint8_t         data[7];
        struct{
            union{
                uint8_t     data[4];
                uint32_t    val;
            }Baudrate;
            union{
                uint8_t     data[1];
                uint8_t     val;
            }Parity;
            union{
                uint8_t     data[1];
                uint8_t     val;
            }StopBits;
            union{
                uint8_t     data[1];
                uint8_t     val;
            }DataBits;
        };
    }Settings;
}_portdat;
#pragma pack(pop)


/***************************************
*       Function Prototypes
***************************************/
CYBLE_API_RESULT_T SCB_ConfigurationChange( SCB_OP_MODE opMode );

void HandleUartRxTraffic( );
void HandleUartTxTraffic( CYBLE_GATTS_WRITE_REQ_PARAM_T *uartTxDataWrReq );

CY_ISR_PROTO( BleSCB_IsrHandler );

CYBLE_API_RESULT_T UartConfig( uint32_t baudrate, uint8_t parity, uint8_t stopbits, uint8_t databits );
CYBLE_API_RESULT_T UartStart( );
CYBLE_API_RESULT_T UartStop( void );
CYBLE_API_RESULT_T I2CStart(void);

    
#endif

/* [] END OF FILE */
