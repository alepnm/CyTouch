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
#ifndef _I2CM_H
#define _I2CM_H

#include <project.h>
#include "stdbool.h"
    
/* EEPROM tipai */ 
#define LC04    4   //4 kbitai
#define LC08    8   //8 kbitai
#define LC16    16  //16 kbitai
    
#define __EEPROM    LC08
    
 
#if defined __EEPROM
    #define I2C_SLAVE_ADDR          0x50
    
    #if ( __EEPROM == LC04 ) 
        #define I2C_NBLOCKS         2u  //24LC04
    #elif( __EEPROM == LC08 ) 
        #define I2C_NBLOCKS         4u  //24LC08
    #elif( __EEPROM == 16 ) 
        #define I2C_NBLOCKS         8u  //24LC16
    #endif   
    
    #define EE_SIZE                 (I2C_NBLOCKS * EE_BLOCK_SIZE)
    #define EE_PAGE_SIZE            (16u)
    #define EE_BLOCK_SIZE           (256u)
    #define EE_EMPTY_BYTE           (0xFF)
#endif

    


/*
                        Fizinis adresas         Ilgis
EE_BASE_ADDR                0x0010                --
EE_DATA_OK_FLAG_ADDR        0x0010                1
EE_MB_DATA_ADDR             0x0012                7
EE_TSET_RANGE_ADDR          0x001A                3
EE_LOCKTIMER_ADDR           0x0020                4
#define EE_PASSWD_ADDR      0x0025                4
*/


#define EE_DATA_OK_FLAG_VAL         0xAA   /* pozymis netuscio EEPROMO ( 1 byte )*/

/* EEPROM adresai */
#define EE_BASE_ADDR                0x0010  /* bazinis adresas */
#define EE_DATA_OK_FLAG_ADDR        0x0011  /* ( 1 byte ) */
#define EE_MB_DATA_ADDR             0x0012  /* ( 7 bytes ) */
#define EE_SLAVE_ADDR               0x001A  /* slaivo adresas ( 1 byte ) */
#define EE_TSET_RANGE_ADDR          0x001C  /* ( TSET nustatymai 3 bytes ) */
#define EE_LOCKTIMER_ADDR           0x0020
#define EE_PASSWD_ADDR              0x0024

#define EE_WTIME_ADDR               0x0029




 
/* Funkciju prototipai */
void I2CM_Init(void);


/* EEPROM 24LCxx API */
void EE_PutByte(uint16_t addr, uint8_t *data);
void EE_PutWord(uint16_t addr, uint16_t *data);
void EE_PutDWord(uint16_t addr, uint32_t *data);
void EE_GetByte( uint16_t addr, uint8_t *data );
void EE_GetWord( uint16_t addr, uint16_t *data );
void EE_GetDWord( uint16_t addr, uint32_t *data );

bool EE_PutArray( uint8_t *buf, uint16_t start, uint16_t len);
bool EE_GetArray( uint8_t *buf, uint16_t start, uint16_t len);
void EE_EraseChip();
void EE_ClearRegion(uint16_t start, uint16_t len);

#endif /* _I2CM_H */
/* [] END OF FILE */
