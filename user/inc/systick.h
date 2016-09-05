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
#ifndef _SYSTICK_H
#define _SYSTICK_H

/* --------------------------- System includes ------------------------------*/
#include <project.h>
#include "mpwm.h"
    

/* --------------------------- Global functions -----------------------------*/    
void SysTickInit(void);
uint32_t GetTicks(void);
    
#endif /* _SYSTICK_H */ 
/* [] END OF FILE */
