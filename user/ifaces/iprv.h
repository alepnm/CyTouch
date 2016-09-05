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
#if !defined(IPRV_H)
#define IPRV_H
    
#include <project.h>      
#include "mbunit.h"
    
/*****************************************************************************/
/**                           PRV modbus registrai                          **/
/*****************************************************************************/
/* ********************************* Coils ********************************* */
    #define PRV_C_ANTIFROST_ALRM    0x00        /* A2 iejimas */
    #define PRV_C_FIRE_ALRM         0x01        /* A1 iejimas */
    #define PRV_C_FILTER_ALRM       0x02
    #define PRV_C_FAN_ALRM          0x03        /* A4 iejimas */
    #define PRV_C_TJ_ALRM           0x06      // t. supply
    #define PRV_C_TE_ALRM           0x07      // t. exhaust
    #define PRV_C_TA_ALRM           0x08      // t. extract
    #define PRV_C_RH_ALRM           0x09      // RH
    #define PRV_C_TWAT_ALRM         0x0A
    #define PRV_C_TOUT_ALRM         0x0B
    
    #define PRV_C_PREHEATER         0x0C      // pasildytuvo aktyvumo indikacija
    #define PRV_C_MOTOR             0x0D      // varikliu aktyvumo indikacija
    #define PRV_C_HEATER            0x0E      // sildytuvo aktyvumo indikacija
    
    
    #define PRV_C_BOOST             0x0F      // boostas
    #define PRV_C_STDBY             0x11      // Standby rezimas
    #define PRV_C_RESET             0x12      // Test / Reset
    #define PRV_C_ONOFF             0x14      // On / Off
    #define PRV_C_OLDRC_COMP        0x15
    #define PRV_C_RESET_DEFAULTS    0x16      // reset to defaults settings
    
    
    #define PRV_C_OVERHEAT_ALRM     0x2C
    #define PRV_C_ROTOR_ALRM        0x2D
    
/* **************************** Discreet Inputs **************************** */
//    #define D_xxx
    
/* **************************** Input Registers **************************** */
    // sensoriu paruodymai formatu value*10
    #define PRV_I_TJ                0x0000      // t. supply        formatas: value*10
    #define PRV_I_TA                0x0003      // t. extract       formatas: value*10
    #define PRV_I_TE                0x0006      // t. exhaust       formatas: value*10
    #define PRV_I_TOUT              0x0009      //                  formatas: value*10
    #define PRV_I_TWAT              0x000C      //                  formatas: value*10
    
    #define PRV_I_RH                0x000D      //                  formatas: 0-99
    
/* *************************** Holding Registers *************************** */
    #define PRV_H_SPEED             0x0000
    #define PRV_H_TSET              0x0001      // formatas: 0-30
    #define PRV_H_FAST_BTN          0x0113      // Fast mygtukas: 0-no function, 1-Boost, 2-Standby
    #define PRV_H_BOOST_COUNT       (277u)      // boost funkcijos counteris prv ploksteje
    #define PRV_H_OFF_MODE          (299u)      // 0: isjungtas, 1: aktyvus
    #define PRV_H_MANUAL_MODE       (300u)      // PRV plokstes darbo rezimas: MANUAL / SCHEDULER
    
    
    
    
    bool PrvMBusCmdWriteSpeedReg(uint8_t addr, uint8_t value);
    bool PrvMBusCmdWriteTSetReg(uint8_t addr, uint8_t value);
    bool PrvMBusCmdWriteBoostCoil(uint8_t addr, bool value);
    bool PrvMBusCmdWriteBoostCounterReg( uint8_t addr, uint8_t value );
    bool PrvMBusCmdWriteReboot( uint8_t addr, uint8_t value );
    bool PrvMBusCmdWritePrvSettings( uint8_t addr, uint8_t value );
    
    void PrvGetDataFromSlave( mbdev_t *slave );
    void PrvSendDataToSlave( mbdev_t *slave );
#endif
/* [] END OF FILE */
