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
#ifndef _SPIM_H
#define _SPIM_H
    
#include <project.h>

    
#define SPI_RX_DATA_BITS 8
#define SPI_TX_DATA_BITS 8    
    
void SPIM_Init(); 
void SpiWriteByte(uint8_t data);
void SpiWriteWord(uint16_t data);

void SpiWriteArray(uint8_t *data, uint8_t len); // <- nepatikrinta!

#endif /* _SPIM_H */ 
/* [] END OF FILE */
