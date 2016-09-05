/*******************************************************************************
* File Name: app_Ble.h
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

#if !defined(APP_BLE_H) 
#define APP_BLE_H

#include <project.h>
//#include "common.h"
#include "main.h"
#include "BleUART.h"
#include "systick.h"

    
/*************************Macro Definitions**********************************/
#define CONN_PARAM_UPDATE_MIN_CONN_INTERVAL     ( 70u )     //Minimum connection interval
#define CONN_PARAM_UPDATE_MAX_CONN_INTERVAL     ( 100u )    //Maximum connection interval
#define CONN_PARAM_UPDATE_SLAVE_LATENCY         ( 0u )      //Slave latency
#define CONN_PARAM_UPDATE_SUPRV_TIMEOUT         ( 100u )    //Supervision timeout
    
    
/***************************************
*       Constants
***************************************/
#define NOTIFICATON_DISABLED    ( 0u )
#define NOTIFICATON_ENABLED     ( 1u )
    
#define INDICATION_DISABLED     ( 0u )
#define INDICATION_ENABLED      ( 1u )
    
    
/* Client Characteristic Configuration descriptor data length. This is defined
* as per BLE spec. */
#define CCCD_DATA_LEN           ( 2u )
    
#define NTF_FLAG                ( 0u )
#define IND_FLAG                ( 1u )       
    
    
/***************************************
*       Function Prototypes
***************************************/
    
/* Global functions */
void HandleBleProcess(void);
void StackEvents( uint32 event, void *eventParam );

CYBLE_API_RESULT_T UpdateAllCharacteristics(void);
CYBLE_API_RESULT_T GetCharacteristicValue(CYBLE_GATT_DB_ATTR_HANDLE_T handle, uint8_t *attr, uint8_t len);
CYBLE_API_RESULT_T SetCharacteristicValue(CYBLE_GATT_DB_ATTR_HANDLE_T charIndex, uint8_t *attrValue, uint8_t attrSize);

CYBLE_API_RESULT_T PrvDataOverNotification(void);
CYBLE_API_RESULT_T CyTouchErrorOverNotification( CYBLE_API_RESULT_T  apiResult );
CYBLE_API_RESULT_T DebugDataOverNotification( void );

CYBLE_API_RESULT_T BleDataPrepare(void);

#endif
/* [] END OF FILE */
