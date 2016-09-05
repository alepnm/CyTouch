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

#ifndef _PWM_H
#define _PWM_H
#include <project.h>
#include "stdbool.h"
    
    
#define BL_MIN      (1000u)
#define BL_MAX      (20000u)
#define BL_TIMEOUT  (20000u)
    
#define NONE        (0u)
#define LOW         (2u)
#define MIDLE       (6u)
#define HIGH        (20u)
    
/* -------------------------- Global functions ------------------------------*/
void SoundInit( uint8_t enable, uint8_t volume );
void Sound( void );
void SetSoundTone( uint8_t _tone );
bool GetSoundTone( void );
void SetSoundVolume( uint8_t volume );

void BacklightInit(uint8_t enable, uint16_t timeout);
void Backlight(void);
void BlStartTimeout(void);
void BlBlancEnable(uint8_t enable);
uint16_t BlGetValue(void);
void BlSetValue(uint16_t bl);
void BlResetCounter(void);
uint16_t BlGetCounter(void);
bool BlGetState(void);
    
#endif /* _PWM_H */ 

/* [] END OF FILE */
