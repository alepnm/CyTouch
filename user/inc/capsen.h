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
#ifndef CAPSEN_H
#define CAPSEN_H
#include <project.h>
#include <stdbool.h>

#define LOCK_TIMER_DEF      ( 300000u )
    
void CapSenseInit( void );
void CapSenseProcess( void );
bool CapSenseIsActive( void );
void ResetLockTimer( void );

#endif /* CAPSEN_H */
/* [] END OF FILE */
