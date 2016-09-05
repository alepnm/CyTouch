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
#include "mspi.h"
#include "common.h"

void SpiInterruptHandler(void);

#define SPI_LATCH()     SPI_LATCH_Write(1);     \
                        CyDelayUs(50);          \
                        SPI_LATCH_Write(0);

/*******************************************************************************
* SPI Configuration
*******************************************************************************/
#define SPI_OVERSAMPLE 16

#define SPI_RX_BUFFER_SIZE 8
#define SPI_TX_BUFFER_SIZE 8

uint8_t SPI_BufRx[SPI_RX_BUFFER_SIZE];
uint8_t SPI_BufTx[SPI_TX_BUFFER_SIZE];

#define SPI_RX_BUFFER_PTR SPI_BufRx
#define SPI_TX_BUFFER_PTR SPI_BufTx

#define SPI_CLK_DIVIDER         (32u)
#define SPI_CLK_FRACTIONAL_DIVIDER  (0u)

const SCBSPI_SPI_INIT_STRUCT ConfSPI =
{
    SCBSPI_SPI_MASTER,
    SCBSPI_SPI_MODE_MOTOROLA,
    SCBSPI_SPI_SCLK_CPHA0_CPOL0,
    SPI_OVERSAMPLE,
    DISABLED,                       /* enableMedianFilter: disabled */
    DISABLED,                       /* enableLateSampling: disabled */
    NON_APPLICABLE,                 /* enableWake: N/A */
    SPI_RX_DATA_BITS,
    SPI_TX_DATA_BITS,
    SCBSPI_BITS_ORDER_LSB_FIRST,       /* bitOrder: LSB first */
    SCBSPI_SPI_TRANSFER_CONTINUOUS,    /* transferSeperation: */
    SPI_RX_BUFFER_SIZE,
    SPI_RX_BUFFER_PTR,
    SPI_TX_BUFFER_SIZE,
    SPI_TX_BUFFER_PTR,
    ENABLED,                        /* enableInterrupt: disabled */
    DISABLED,                       /* rxInterruptMask: N/A */
    DISABLED,                       /* rxTriggerLevel: N/A */
    SCBSPI_INTR_MASTER_SPI_DONE,       /* txInterruptMask: N/A */
    DISABLED,                       /* txTriggerLevel: N/A */
    DISABLED,                       /* enableByteMode: disabled */
    DISABLED,                       /* enableFreeRunSclk: disabled */
    SCBSPI_SPI_SS_ACTIVE_HIGH          /* polaritySs: high */    
};

void SPIM_Init()
{
    /* Disable component before re-configuration */
    SCBSPI_Stop();

    /* Set clock divider to provide clock frequency to the SCB component
    * to operated with desired data rate.
    */
    SCB_Clock_SetFractionalDividerRegister(SPI_CLK_DIVIDER, SPI_CLK_FRACTIONAL_DIVIDER);
    
    /* Configure SCB component. The configuration is stored in the SPI
    * configuration structure.
    */
    SCBSPI_SpiInit(&ConfSPI);

    /* Start component after re-configuration is complete */
    SCBSPI_Start();  
    
    SCBSPI_SetCustomInterruptHandler(&SpiInterruptHandler);
}


void SpiWriteByte(uint8_t data)
{
    SCBSPI_SpiUartWriteTxData(data);
}


void SpiWriteWord(uint16_t data)
{    
    SCBSPI_SpiUartWriteTxData(HI8(data));
    SCBSPI_SpiUartWriteTxData(LO8(data));
}

void SpiWriteArray(uint8_t *data, uint8_t len)
{    
    uint8_t i = 0u;
    
    do{
        SCBSPI_SpiUartWriteTxData( data[i] );
    }while( ++i < len );
}


/*
* User interrupt handler to insert into SCB interrupt handler.
* Note: SCB interrupt set to Internal in GUI.
*/
void SpiInterruptHandler(void)
{       
    if(SCBSPI_GetMasterInterruptSourceMasked() & SCBSPI_INTR_MASTER_SPI_DONE)
    {    
        SCBSPI_ClearMasterInterruptSource(SCBSPI_INTR_MASTER_SPI_DONE);
        
        while(SCBSPI_SpiIsBusBusy());
   
        SPI_LATCH()
    }
}

/* [] END OF FILE */
