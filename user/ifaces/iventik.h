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
#if !defined(IVENTIK_H)
#define IVENTIK_H
    
#include <project.h>
#include "mbunit.h"
    
    
    
/*****************************************************************************/
/**                        Ventik modbus registrai                          **/
/*****************************************************************************/
/* ********************************* Coils ********************************* */
   
    #define VENTIK_C_RESET                  0x12    // Test / Reset
//    #define VENTIK_C_DECREASE_SPEED         0x29

//    
/* **************************** Discreet Inputs **************************** */    
    #define VENTIK_D_TJ_ALRM                0x02    /* Ain1 */
    #define VENTIK_D_TWAT_ALRM              0x04    /* Ain2 */
    #define VENTIK_D_TOUT_ALRM              0x05    /* Ain3 */
    #define VENTIK_D_FIRE_ALRM              0x09    /* A1 */
    #define VENTIK_D_TSUP_TOOLOW_ALRM       0x0A    /* A2 */
    #define VENTIK_D_FAN_ALRM               0x0C    /* A4 */
    #define VENTIK_D_ANY_SENSOR_ALRM        0x0D    /* betkokio sensorio alarmas */
    #define VENTIK_D_CRITICAL_RET_ALRM      0x12    /* A5 - griztamo vandens kritine temperatura*/
    #define VENTIK_D_LOWVOLTAGE_ALRM        0x13    /* A7 */
    #define VENTIK_D_FILTER_ALRM            0x17    /* Filter */
    #define VENTIK_D_TSUP_TOOHIGH_ALRM      0x1E    /* A6 */
    #define VENTIK_D_ANY_INPUT_ALRM         0x1F    /* betkokio iejimo alarmas */
    #define VENTIK_D_SPEEDLOW               0x20    /* SpeedLow funkcija aktyvi */
    
/* **************************** Input Registers **************************** */
    #define VENTIK_I_TJ                     0x00    // t. supply (AIN1) formatas: value*10
    #define VENTIK_I_ACT_FAN_SPEED          0x01    // dabartinis ventiliatoriaus greitis
    #define VENTIK_I_ACT_TEMP_VALUE         0x02    // dabartines temperaturos reiksme
    #define VENTIK_I_WATER_HEATER_OUTPUT    0x04    // 0-10 sildytuvo isejimo reiksme 0-100%
    #define VENTIK_I_EC_MOTOR_OUTPUT        0x08    // 0-10 EC variklio isejimo reiksme 0-100%
    
    #define VENTIK_I_TOUT                   0x09    // formatas: value*10
    #define VENTIK_I_TWAT                   0x0C    // formatas: value*10
/* *************************** Holding Registers *************************** */
    #define VENTIK_H_SPEED                  0x00
    #define VENTIK_H_TSET                   0x01    // formatas: 0-30


    
    
    
    bool VentikMBusCmdWriteSpeedReg( uint8_t addr, uint8_t value );
    bool VentikMBusCmdWriteTSetReg( uint8_t addr, uint8_t value );
    bool VentikMBusCmdWriteReboot( uint8_t addr, uint8_t value );
    bool VentikMBusCmdWritePrvSettings( uint8_t addr, uint8_t value );   
    
    
    void VentikGetDataFromSlave( mbdev_t *slave );
    void VentikSendDataToSlave( mbdev_t *slave );
    
#endif
/* [] END OF FILE */
