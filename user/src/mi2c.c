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
#include <project.h>
#include "mi2c.h"
#include "mspi.h"
#include "common.h"



/*******************************************************************************
* I2C Configuration
*******************************************************************************/
#define I2C_STANDARD_MODE_MAX   (100u)

#define I2C_OVERSAMPLE_LO  8
#define I2C_OVERSAMPLE_HI  8

/* I2C slave desired data rate is 100 kbps. The datasheet Table 1 provides a
* range of possible clock values 1.55 - 12.8 MHz. The CommCLK = 1.6 MHz is
* selected from this range. The clock divider has to be calculated to control
* clock frequency as clock component provides interface to it.
* Divider = (HFCLK / CommCLK) = (24MHz / 1.6 MHz) = 15. But the value written
* into the register has to decremented by 1. The end result is 14.
*/
#define I2C_CLK_DIVIDER     (14u)

/* Comm_I2C_INIT_STRUCT provides the fields which match the selections available
* in the customizer. Refer to the I2C customizer for detailed description of
* the settings.
*/
const SCBSPI_I2C_INIT_STRUCT ConfI2C =
{
    SCBSPI_I2C_MODE_MASTER, /* mode: master */
    I2C_OVERSAMPLE_LO,      /* oversampleLow: oversample low */
    I2C_OVERSAMPLE_HI,      /* oversampleHigh: oversample hi */
    NON_APPLICABLE,         /* enableMedianFilter: N/A */
    NON_APPLICABLE,         /* slaveAddr: N/A */
    NON_APPLICABLE,         /* slaveAddrMask: N/A */
    DISABLED,               /* acceptAddr: disabled */
    DISABLED,               /* enableWake: disabled */
    DISABLED,               /* enableByteMode: disabled */
    I2C_STANDARD_MODE_MAX,  /* dataRate: 100 kbps */
    DISABLED                /* acceptGeneralAddr */
};

static void Mode_I2C( void );
static void Mode_SPI( void );


void I2CM_Init(){   
    /* Disable component before re-configuration */
    SCBSPI_Stop();

    /* Set clock divider to provide clock frequency to the SCB component
    * to operated with desired data rate.
    */
//    SCB_Clock_SetFractionalDividerRegister(I2C_CLK_DIVIDER, 0u);
    SCB_Clock_SetFractionalDividerRegister( CYDEV_BCLK__HFCLK__MHZ / 1.55 - 1, 0u );

    /* Configure SCB component. The configuration is stored in the I2C
    * configuration structure.
    */
    SCBSPI_I2CInit(&ConfI2C);

    /* Start component after re-configuration is complete */
    SCBSPI_Start();
}

static void Mode_I2C(){
    I2CM_Init();    
    CyDelay(1);
}
static void Mode_SPI(){
    CyDelay(1);    
    SPIM_Init();
}

/* EEPROM API */
void EE_PutByte(uint16_t addr, uint8_t *data){ 
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    ( void )SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
    ( void )SCBSPI_I2CMasterWriteByte(LO8(addr));
    ( void )SCBSPI_I2CMasterWriteByte(*data);
    ( void )SCBSPI_I2CMasterSendStop(); 

    Mode_SPI();
    
    CyExitCriticalSection(isr);
}
void EE_PutWord(uint16_t addr, uint16_t *data){
    
    union{
        uint16_t    val;
        struct{
            uint8_t byte0;
            uint8_t byte1;
        };
    }var = { *data };
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    ( void )SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
    ( void )SCBSPI_I2CMasterWriteByte(LO8(addr)); 
    ( void )SCBSPI_I2CMasterWriteByte( var.byte0 );
    ( void )SCBSPI_I2CMasterWriteByte( var.byte1 ); 
    ( void )SCBSPI_I2CMasterSendStop();
    
    CyDelay(5);
    Mode_SPI();
    
    CyExitCriticalSection(isr);
}
void EE_PutDWord(uint16_t addr, uint32_t *data){
    
    union{
        uint32_t    val;
        struct{
            uint8_t byte0;
            uint8_t byte1;
            uint8_t byte2;
            uint8_t byte3;
        };
    }var = { *data };
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    ( void )SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
    ( void )SCBSPI_I2CMasterWriteByte(LO8(addr));
    ( void )SCBSPI_I2CMasterWriteByte(var.byte0);
    ( void )SCBSPI_I2CMasterWriteByte(var.byte1);
    ( void )SCBSPI_I2CMasterWriteByte(var.byte2);
    ( void )SCBSPI_I2CMasterWriteByte(var.byte3);
   
    ( void )SCBSPI_I2CMasterSendStop(); 
    
    CyDelay(5);
    Mode_SPI();
    
    CyExitCriticalSection(isr);
}
void EE_GetByte( uint16_t addr, uint8_t *data ){
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    ( void )SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
    ( void )SCBSPI_I2CMasterWriteByte(LO8(addr));
        
    ( void )SCBSPI_I2CMasterSendRestart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_READ_XFER_MODE);
    *data = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_NAK_DATA);
    ( void )SCBSPI_I2CMasterSendStop();  
    
    CyDelay(3);
    
    Mode_SPI();
    
    CyExitCriticalSection(isr);
}
void EE_GetWord( uint16_t addr, uint16_t *data ){
    
    union{
        uint16_t    val;
        struct{
            uint8_t byte0;
            uint8_t byte1;
        };
    }var; 
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    ( void )SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
    ( void )SCBSPI_I2CMasterWriteByte(LO8(addr));
        
    ( void )SCBSPI_I2CMasterSendRestart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_READ_XFER_MODE);
    var.byte0 = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_ACK_DATA);
    var.byte1 = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_NAK_DATA);
    
    *data = var.val; 
    
    ( void )SCBSPI_I2CMasterSendStop();  
    
    CyDelay(3);
    
    Mode_SPI();
    
    CyExitCriticalSection(isr);
}
void EE_GetDWord( uint16_t addr, uint32_t *data ){
    
    union{
        uint32_t    val;
        struct{
            uint8_t byte0;
            uint8_t byte1;
            uint8_t byte2;
            uint8_t byte3;
        };
    }var;    
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    ( void )SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
    ( void )SCBSPI_I2CMasterWriteByte(LO8(addr));
        
    ( void )SCBSPI_I2CMasterSendRestart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_READ_XFER_MODE);
    var.byte0 = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_ACK_DATA);
    var.byte1 = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_ACK_DATA);
    var.byte2 = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_ACK_DATA);
    var.byte3 = SCBSPI_I2CMasterReadByte(SCBSPI_I2C_NAK_DATA); 
    
    *data = var.val;
    
    ( void )SCBSPI_I2CMasterSendStop();  
    
    CyDelay(3);
    
    Mode_SPI();
    
    CyExitCriticalSection(isr);
}

bool EE_PutArray( uint8_t *buf, uint16_t start, uint16_t len){ 
    uint8_t i = 0;
    uint16_t offset = 0;
    
    if(len > 256) return false;    
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();    
    
    do{      
        // transaction start        
        SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(start), SCBSPI_I2C_WRITE_XFER_MODE);
        SCBSPI_I2CMasterWriteByte(LO8(start));
        
        i = EE_PAGE_SIZE;
                
        start += EE_PAGE_SIZE;

        do{
            SCBSPI_I2CMasterWriteByte( buf[offset++] );            
            if( (--len) == 0 ) break;
        }while( (--i) > 0 );
        
        SCBSPI_I2CMasterSendStop();
        
        CyDelay( offset );
        // transaction end
    }while( len > 0 );   
    
    Mode_SPI();
    
    CyExitCriticalSection(isr);
    
    return true;
}
bool EE_GetArray( uint8_t *buf, uint16_t start, uint16_t len){ 
    uint16_t offset = 0;
    
    if(len > 256) return false;
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    // transaction start
    SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(start), SCBSPI_I2C_WRITE_XFER_MODE);
    SCBSPI_I2CMasterWriteByte(LO8(start));
    
    SCBSPI_I2CMasterSendRestart(I2C_SLAVE_ADDR + HI8(start), SCBSPI_I2C_READ_XFER_MODE);
    do{            
        if( len > 1 ) buf[offset++] = SCBSPI_I2CMasterReadByte( SCBSPI_I2C_ACK_DATA );
        else buf[offset] = SCBSPI_I2CMasterReadByte( SCBSPI_I2C_NAK_DATA );
    }while( (--len) > 0 );
    
    SCBSPI_I2CMasterSendStop(); 
    // transaction end
    
    CyDelay( offset );
    
    Mode_SPI();
    
    CyExitCriticalSection(isr);
    
    return true;
}
void EE_EraseChip(){
    uint8_t i = 0;
    uint16_t addr = 0;
    uint8_t page = 0;
    uint8_t byte = EE_EMPTY_BYTE;
    
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();
    
    // chip clear
    while( addr < EE_SIZE ){
        
//        if( HI8(addr) == 2 ) byte = 0x00;
//        else byte = EE_EMPTY_BYTE;
        
        // block clear
        do{   
            SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(addr), SCBSPI_I2C_WRITE_XFER_MODE);
            SCBSPI_I2CMasterWriteByte(LO8(addr));
            
            i = EE_PAGE_SIZE;

            do{
                SCBSPI_I2CMasterWriteByte( byte ); 
            }while( (--i) > 0 );
            
            SCBSPI_I2CMasterSendStop();
            
            CyDelay(5);

            addr += EE_PAGE_SIZE;

        }while( (++page) < ( EE_BLOCK_SIZE / EE_PAGE_SIZE ) );
        
        page = 0;
    }
   
    CyDelay(10);
    
    Mode_SPI();
    
    CyExitCriticalSection(isr);
}
void EE_ClearRegion(uint16_t start, uint16_t len){
    uint16_t i = 0;
    uint16_t offset = 0;
    uint8_t qqq = 0;
 
    uint8_t isr = CyEnterCriticalSection();
    
    Mode_I2C();    
    
    
    qqq = start % 0x10;
    
    if(qqq > 0){
        
        SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(start), SCBSPI_I2C_WRITE_XFER_MODE);
        SCBSPI_I2CMasterWriteByte(LO8(start));
        
        do{
            SCBSPI_I2CMasterWriteByte( EE_EMPTY_BYTE ); 
        }while(++qqq < 16);
        
        SCBSPI_I2CMasterSendStop();
        
        CyDelay(5);
        
        start += offset;
    };    
    
    len -= offset;
    
    
    
    do{      
        // transaction start        
        SCBSPI_I2CMasterSendStart(I2C_SLAVE_ADDR + HI8(start), SCBSPI_I2C_WRITE_XFER_MODE);
        SCBSPI_I2CMasterWriteByte(LO8(start));
        
        i = EE_PAGE_SIZE;
        
        start += EE_PAGE_SIZE;

        do{
            SCBSPI_I2CMasterWriteByte( EE_EMPTY_BYTE );  
            if( (--len) == 0) break;
        }while( (--i) > 0 );
        
        SCBSPI_I2CMasterSendStop();
        
        CyDelay(6);
        // transaction end
    }while( len > 0 );     
    
    Mode_SPI(); 
    
    CyExitCriticalSection(isr);  
}

/* [] END OF FILE */
