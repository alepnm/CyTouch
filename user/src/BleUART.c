/*****************************************************************************************
* File Name: BleUART.c
*
* Description:
*  Common BLE application code for client devices.
*
*****************************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*****************************************************************************************/

#include "BleUART.h"


//request: 01:03:00:00:00:78:45:E8 Read HR from 0x00 count = 0x78
//request: 01:03:00:78:00:78:C5:F1 Read HR from 0x78 count = 0x78
//request: 01:03:00:F0:00:78:45:DB Read HR from 0xF0 count = 0x78
//request: 01:03:01:68:00:58:C4:10 Read HR from 0x168 count = 0x58

//request: 01:03:00:00:00:10:44:06 Read HR from 0x00 count = 0x10
//request: 01:05:00:12:FF:00:2C:3F Reset PRV

/* MTU size to be used by Client and Server after MTU exchange */
extern uint16_t     mtuSize;
extern bool      BLEStackStatus;

/* Global variables to manage current operation mode and initialization state */
//uint8_t mode = OP_MODE_UART;

/*******************************************************************************
* Common Definitions
*******************************************************************************/
/* Common RX and TX buffers for I2C and UART operation */

#define COMMON_BUFFER_SIZE     (127u)
static uint8 bufferTx[COMMON_BUFFER_SIZE];

/* UART RX buffer requires one extra element for proper operation. One element
* remains empty while operation. Keeping this element empty simplifies
* circular buffer operation.
*/
static uint8 bufferRx[COMMON_BUFFER_SIZE + 1u];


/*******************************************************************************
* I2C Configuration
*******************************************************************************/
#define PACKET_SIZE             (3u)

/* Byte position within the packet */
#define PACKET_SOP_POS          (0u)
#define PACKET_CMD_POS          (1u)

/* Command and status share the same offset */
#define PACKET_STS_POS          (PACKET_CMD_POS)
#define PACKET_EOP_POS          (2u)

/* Start and end of the packet markers */
#define PACKET_SOP              (0x01u)
#define PACKET_EOP              (0x17u)

#define I2C_SLAVE_ADDRESS       (0x08u)
#define I2C_SLAVE_ADDRESS_MASK  (0xFEu)
#define I2C_STANDARD_MODE_MAX   (100u)

#define I2C_RX_BUFFER_SIZE      (PACKET_SIZE)
#define I2C_TX_BUFFER_SIZE      (PACKET_SIZE)
#define I2C_RX_BUFER_PTR        bufferRx
#define I2C_TX_BUFER_PTR        bufferTx

/* I2C slave desired data rate is 100 kbps. The datasheet Table 1 provides a
* range of possible clock values 1.55 - 12.8 MHz. The CommCLK = 1.6 MHz is
* selected from this range. The clock divider has to be calculated to control
* clock frequency as clock component provides interface to it.
* Divider = (HFCLK / CommCLK) = (24MHz / 1.6 MHz) = 15. But the value written
* into the register has to decremented by 1. The end result is 14.
*/
#define I2C_CLK_DIVIDER         (14u)

/* Comm_I2C_INIT_STRUCT provides the fields which match the selections available
* in the customizer. Refer to the I2C customizer for detailed description of
* the settings.
*/
static SCB_I2C_INIT_STRUCT configI2C =
{
    SCB_I2C_MODE_SLAVE,    /* mode: slave */
    NON_APPLICABLE,         /* oversampleLow: N/A for slave */
    NON_APPLICABLE,         /* oversampleHigh: N/A for slave */
    NON_APPLICABLE,         /* enableMedianFilter: N/A */
    I2C_SLAVE_ADDRESS,      /* slaveAddr: slave address */
    I2C_SLAVE_ADDRESS_MASK, /* slaveAddrMask: single slave address */
    DISABLED,               /* acceptAddr: disabled */
    DISABLED,               /* enableWake: disabled */
    DISABLED,               /* enableByteMode: disabled */
    I2C_STANDARD_MODE_MAX,  /* dataRate: 100 kbps */
    DISABLED,               /* acceptGeneralAddr */
};


/*******************************************************************************
* UART Configuration
*******************************************************************************/
#define INTR_NONE               0x00

#define UART_OVERSAMPLING       (16u)
#define UART_DATA_WIDTH         (8u)
#define UART_RX_INTR_MASK       SCB_INTR_RX_NOT_EMPTY
#define UART_TX_INTR_MASK       SCB_INTR_TX_UART_DONE

#define UART_RX_BUFFER_SIZE     COMMON_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE     COMMON_BUFFER_SIZE
#define UART_RX_BUFER_PTR       bufferRx
#define UART_TX_BUFER_PTR       bufferTx


/* UART desired baud rate is 115200 bps. The selected Oversampling parameter is
* 16. The CommCLK = Baud rate * Oversampling = 115200 * 16 = 1.843 MHz.
* The clock divider has to be calculated to control clock frequency as clock
* component provides interface to it.
* Divider = (HFCLK / CommCLK) = (24MHz / 1.8432 MHz) = 13. But the value
* written into the register has to decremented by 1. The end result is 12.
* The clock accuracy is important for UART operation. The actual CommCLK equal:
* CommCLK(actual) = (24MHz / 13MHz) = 1.846 MHz
* The deviation of actual CommCLK from desired must be calculated:
* Deviation = (1.843MHz – 1.846 MHz) / 1.843 MHz = ~0.2%
* Taking into account HFCLK accuracy ±2%, the total error is: 0.2 + 2= 2.2%.
* The total error value is less than 5% and it is enough for correct
* UART operation.
* Divider = (HFCLK / Baudrate * Oversampling) - 1;
*/
static const uint32_t baudrates[6u] = {4800u, 9600u, 19200u, 38400u, 57600u, 115200u};

_portdat MbPort;

/* Comm_UART_INIT_STRUCT provides the fields which match the selections
* available in the customizer. Refer to the I2C customizer for detailed
* description of the settings.
*/
static SCB_UART_INIT_STRUCT configUart =
{
    SCB_UART_MODE_STD,      /* mode: Standard */
    SCB_UART_TX_RX,         /* direction: RX + TX */
    UART_DATA_WIDTH,        /* dataBits: 8 bits */
    SCB_UART_PARITY_NONE,   /* parity: None */
    SCB_UART_STOP_BITS_1,   /* stopBits: 1 bit */
    UART_OVERSAMPLING,      /* oversample: 16 */
    DISABLED,               /* enableIrdaLowPower: disabled */
    DISABLED,               /* enableMedianFilter: disabled */
    DISABLED,               /* enableRetryNack: disabled */
    DISABLED,               /* enableInvertedRx: disabled */
    DISABLED,               /* dropOnParityErr: disabled */
    DISABLED,               /* dropOnFrameErr: disabled */
    NON_APPLICABLE,         /* enableWake: disabled */
    UART_RX_BUFFER_SIZE,    /* rxBufferSize: TX software buffer size */
    UART_RX_BUFER_PTR,      /* rxBuffer: pointer to RX software buffer */
    UART_TX_BUFFER_SIZE,    /* txBufferSize: TX software buffer size */
    UART_TX_BUFER_PTR,      /* txBuffer: pointer to TX software buffer */
    DISABLED,               /* enableMultiproc: disabled */
    DISABLED,               /* multiprocAcceptAddr: disabled */
    NON_APPLICABLE,         /* multiprocAddr: N/A */
    NON_APPLICABLE,         /* multiprocAddrMask: N/A */
    ENABLED,                /* enableInterrupt: enable internal interrupt
                             * handler for the software buffer */
    UART_RX_INTR_MASK,      /* rxInterruptMask: enable INTR_RX.NOT_EMPTY to
                             * handle RX software buffer operations */
    NON_APPLICABLE,         /* rxTriggerLevel: N/A */
    UART_TX_INTR_MASK,      /* txInterruptMask: enable INTR_TX.UART_DONE to
                             * handle TX software buffer operations */
    NON_APPLICABLE,         /* txTriggerLevel: N/A */
    ENABLED,                /* enableByteMode: enabled */
    DISABLED,               /* enableCts: disabled */
    DISABLED,               /* ctsPolarity: disabled */
    DISABLED,               /* rtsRxFifoLevel: disabled */
    DISABLED,               /* rtsPolarity: disabled */
};




/*****************************************************************************************
* Function Name: HandleUartRxTraffic
******************************************************************************************
* Slave sesponse traffic (RX) - is plokstes per pulta i Android irengini
*
* Summary:
*  This function takes data from UART RX buffer and pushes it to the server 
*  as Notifications.
*
* Parameters: None.
* Return: None.
*****************************************************************************************/
void HandleUartRxTraffic()
{
    uint8_t   uartRxData[mtuSize - (3u)];
    uint16_t  bleRxDataLen;
    
    static uint16_t uartIdleCount = UART_IDLE_TIMEOUT;
    
    CYBLE_API_RESULT_T                  bleApiResult;
    CYBLE_GATTS_HANDLE_VALUE_NTF_T      uartRxDataNtf;
    
    bleRxDataLen = SCB_SpiUartGetRxBufferSize();
    
    if( bleRxDataLen == (0u) ) return;

    if(bleRxDataLen >= (mtuSize - (3u) )){
        uartIdleCount   = UART_IDLE_TIMEOUT;
        bleRxDataLen    = mtuSize - (3u);
    }
    else{
        if(--uartIdleCount == (0u)) {
            /*uartTxDataLength remains unchanged */;
        }
        else{
            bleRxDataLen = (0u);
        }
    }
    
    if( bleRxDataLen > (0u) ){
        uint8_t index = (0u);
        
        uartIdleCount   = UART_IDLE_TIMEOUT;
        
        for(index = (0u); index < bleRxDataLen; index++){
            uartRxData[index] = (uint8_t)SCB_UartGetByte();
        }
        
        uartRxDataNtf.attrHandle = CYBLE_BLEUART_RXDATA_CHAR_HANDLE;
        uartRxDataNtf.value.val  = uartRxData;
        uartRxDataNtf.value.len  = bleRxDataLen;
        
        do
        {        
            bleApiResult = CyBle_GattsNotification(cyBle_connHandle, &uartRxDataNtf);
            CyBle_ProcessEvents();
        }while(( bleApiResult != CYBLE_ERROR_OK ) && (cyBle_state == CYBLE_STATE_CONNECTED ));
    }
}

/*****************************************************************************************
* Function Name: HandleUartTxTraffic
******************************************************************************************
*Master request traffic (TX) - is Android irenginio per pulteli i plokste
*
* Summary:
*  This function takes data from received "write without response" command from
*  server and, pushes it to the UART TX buffer. 
*
* Parameters:
*  CYBLE_GATTS_WRITE_REQ_PARAM_T * - the "write without response" param as
*                                    recieved by the BLE stack
*
* Return: None.
*****************************************************************************************/
void HandleUartTxTraffic(CYBLE_GATTS_WRITE_REQ_PARAM_T *uartTxDataWrReq)
{
    DE_Write(1);
    CyDelayUs(100);
    
    SCB_SpiUartPutArray( uartTxDataWrReq->handleValPair.value.val, uartTxDataWrReq->handleValPair.value.len );
    
    SCB_ENABLE_INTR_TX( SCB_INTR_TX_UART_DONE );
}

/*****************************************************************************************
* Function Name: UartBleConfig
******************************************************************************************
* Summary:
*  Si funkcija konfiguruoja UART nustatymus BLE rezimui. Patikrina baudrate parametra, ar jis
*  yra leistinas ar ne.
*
* Parametrai:
*   uint32_t    baudrate :  baudraitas is standartiniu (4800 - 115200)
*   uint8_t     parity  :   SCB_UART_PARITY_EVEN    /0x00
*                           SCB_UART_PARITY_ODD     /0x01
*                           SCB_UART_PARITY_NONE    /0x02
*   uint8_t     stopbits :  SCB_UART_STOP_BITS_1    /0x02
*                           SCB_UART_STOP_BITS_1_5  /0x03
*                           SCB_UART_STOP_BITS_2    /0x04
*
* Grazina: cystatus
*****************************************************************************************/
CYBLE_API_RESULT_T UartConfig( uint32_t baudrate, uint8_t parity, uint8_t stopbits, uint8_t databits )
{   
    uint8_t i = 0;
    
    while(baudrate != baudrates[i++]){
        if( i >= ( sizeof(baudrates)/sizeof(baudrate) ) ) return CYBLE_ERROR_INVALID_PARAMETER;
    }
    
    if( parity != SCB_UART_PARITY_EVEN && parity != SCB_UART_PARITY_ODD && 
        parity != SCB_UART_PARITY_NONE ){
        return CYBLE_ERROR_INVALID_PARAMETER;
    }
    
    if( stopbits != SCB_UART_STOP_BITS_1 && stopbits != SCB_UART_STOP_BITS_1_5 && 
        stopbits != SCB_UART_STOP_BITS_2 ){
        return CYBLE_ERROR_INVALID_PARAMETER;
    }
    
    if( databits != 7 && databits != 8 ) return CYBLE_ERROR_INVALID_PARAMETER;
    
    MbPort.Settings.Baudrate.val = baudrate;
    MbPort.Settings.Parity.val = parity;
    MbPort.Settings.StopBits.val = stopbits;
    MbPort.Settings.DataBits.val = databits;
    
    return CYBLE_ERROR_OK;
}

/*****************************************************************************************
* Function Name: UartStart
******************************************************************************************
* Summary:
*  Si funkcija startuoja SCB moduli UART rezimu.
*
* Parametrai: None.
*
* Grazina: cystatus
*****************************************************************************************/
CYBLE_API_RESULT_T UartStart( )
{     
    configUart.dataBits = MbPort.Settings.DataBits.val;
    configUart.parity = MbPort.Settings.Parity.val;
    configUart.stopBits = MbPort.Settings.StopBits.val;

    if( SCB_ConfigurationChange( SCB_MBUS ) != CYRET_SUCCESS ) return CYBLE_ERROR_INVALID_STATE;
    
    return CYBLE_ERROR_OK;
}

/*****************************************************************************************
* Function Name: UartStop
******************************************************************************************
* Summary:
*  Si funkcija stabdo SCB moduli.
*
* Parametrai: None.
* Grazina: None.
*****************************************************************************************/
CYBLE_API_RESULT_T UartStop()
{
    SCB_Stop();
    
    SCB_SpiUartClearTxBuffer();
    SCB_SpiUartClearRxBuffer();
    
    SCB_ClearRxInterruptSource( SCB_INTR_RX_ALL );
    SCB_ClearTxInterruptSource( SCB_INTR_TX_ALL );
    
    SCB_DISABLE_INTR_RX( SCB_INTR_TX_ALL );
    SCB_DISABLE_INTR_TX( SCB_INTR_TX_ALL );
        
    return CYBLE_ERROR_OK;
}


/*****************************************************************************************
* Function Name: I2CStart
******************************************************************************************
* Summary:
*  Si funkcija startoja SCB moduli I2C rezimu.
*
* Parametrai: None.
* Grazina: None.
*****************************************************************************************/
CYBLE_API_RESULT_T I2CStart()
{    
    return SCB_ConfigurationChange( SCB_I2C );
}


/*******************************************************************************
* Function Name: SCB_ConfigurationChange
********************************************************************************
* Summary:
*  This function reconfigures the SCB component between the I2C and UART modes
*  of operation.
*
* Parameters:
*  opMode - mode of operation to which SCB component will be configured.
*
* Return:
*
*******************************************************************************/
CYBLE_API_RESULT_T SCB_ConfigurationChange( SCB_OP_MODE opMode )
{
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    static uint8_t isr;
    
    EnterCriticalSection(&isr);
    
    MbPort.Mode = opMode;
    
    switch( opMode )
    {
        case SCB_MBUS:
            
    /***********************************************************************
    * Configure SCB in UART mode and enable component after completion
    ***********************************************************************/

            /* Disable component before re-configuration */
            SCB_Stop();

            /* Set clock divider to provide clock frequency to the SCB component
            * to operated with desired data rate.
            */
            CommCLK_SetFractionalDividerRegister((( CYDEV_BCLK__HFCLK__HZ /
                ( MbPort.Settings.Baudrate.val * UART_OVERSAMPLING)) - 1 ), 0u);

            /* Configure SCB component. The configuration is stored in the UART
            * configuration structure.
            */
            SCB_UartInit(&configUart);
            
            SCB_SpiUartClearTxBuffer();
            SCB_SpiUartClearRxBuffer(); 
            
            SCB_ClearRxInterruptSource( SCB_INTR_RX_ALL );
            SCB_ClearTxInterruptSource( SCB_INTR_TX_ALL );

            SCB_SetCustomInterruptHandler( MbSCB_IsrHandler );
            
            /* Start component after re-configuration is complete */
            SCB_Start();
            
            ( void )eMBMasterEnable( );
            break;              
        case SCB_BLE:
            
            /***********************************************************************
            * Configure SCB in UART BLE mode and enable component after completion
            ***********************************************************************/
            ( void )eMBMasterDisable( );
            ( void )eMBMasterClose( );
            
            /* Disable component before re-configuration */
            SCB_Stop();

            /* Set clock divider to provide clock frequency to the SCB component
            * to operated with desired data rate.
            */
            CommCLK_SetFractionalDividerRegister((( CYDEV_BCLK__HFCLK__HZ /( MbPort.Settings.Baudrate.val * UART_OVERSAMPLING)) - 1 ), 0u);

            /* Configure SCB component. The configuration is stored in the UART
            * configuration structure.
            */
            SCB_UartInit(&configUart);
            
            SCB_SpiUartClearTxBuffer();
            SCB_SpiUartClearRxBuffer(); 
            
            SCB_ClearRxInterruptSource( SCB_INTR_RX_ALL );
            SCB_ClearTxInterruptSource( SCB_INTR_TX_ALL );            
            
            SCB_SetCustomInterruptHandler( BleSCB_IsrHandler );

            /* Start component after re-configuration is complete */
            SCB_Start();            
            break;            
        case SCB_I2C:
            /***********************************************************************
            * Configure SCB in I2C mode and enable component after completion.
            ***********************************************************************/

            /* Disable component before re-configuration */
            SCB_Stop();

            /* Set clock divider to provide clock frequency to the SCB component
            * to operated with desired data rate.
            */
            CommCLK_SetFractionalDividerRegister(I2C_CLK_DIVIDER, 0u);

            /* Configure SCB component. The configuration is stored in the I2C
            * configuration structure.
            */
            SCB_I2CInit(&configI2C);

            /* Set read and write buffers for the I2C slave */
            SCB_I2CSlaveInitWriteBuf(I2C_RX_BUFER_PTR, I2C_RX_BUFFER_SIZE);
            SCB_I2CSlaveInitReadBuf (I2C_TX_BUFER_PTR, I2C_TX_BUFFER_SIZE);

            /* Put start and end of the packet into the TX buffer */
            bufferTx[PACKET_SOP_POS] = PACKET_SOP;
            bufferTx[PACKET_EOP_POS] = PACKET_EOP;

            /* Start component after re-configuration is complete */
            SCB_Start();
            break;
        case SCB_OFF:
            UartStop( );
            break;
        default:
            apiResult = CYBLE_ERROR_INVALID_PARAMETER; /* Unknown operation mode - no action */
            break;
    }
    
    ExitCriticalSection(&isr);
    
    return ( apiResult );
}


void BleSCB_IsrHandler()
{  
    if( SCB_GetInterruptCause() & SCB_INTR_CAUSE_TX ){         
        if( SCB_GetTxInterruptSourceMasked() & SCB_INTR_TX_UART_DONE )            
        {           
            SCB_ClearTxInterruptSource( SCB_INTR_TX_UART_DONE );
            
            CyDelayUs(100);
            DE_Write(0); 
            
            SCB_DISABLE_INTR_TX( SCB_INTR_TX_UART_DONE );           
        }        
//        SCB_ClearTxInterruptSource( SCB_INTR_TX_ALL );
    }
    
//    if( SCB_GetInterruptCause() & SCB_INTR_CAUSE_RX ){
//        if( SCB_GetRxInterruptSourceMasked() & SCB_INTR_RX_NOT_EMPTY )            
//        { 
//            
//            SCB_ClearRxInterruptSource( SCB_INTR_RX_NOT_EMPTY );
//        }
//    }
}

/* [] END OF FILE */
