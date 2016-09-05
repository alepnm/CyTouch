/******************************************************************************
* Project Name		: PSoC_4_BLE_Pressure_Sensor
* File Name			: common.h
* Version 			: 1.0
* Device Used		: CY8C4247LQI-BL483
* Software Used		: PSoC Creator 3.1
* Compiler    		: ARM GCC 4.8.4
* Related Hardware	: CY8CKIT-042-BLE Bluetooth Low Energy Pioneer Kit
*******************************************************************************/

/***********************************************************************************
* Copyright (2015), Cypress Semiconductor Corporation. All Rights Reserved.
************************************************************************************
* This software, including source code, documentation and related materials
* ("Software") is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and 
* foreign), United States copyright laws and international treaty provisions. 
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the 
* Cypress source code and derivative works for the sole purpose of creating 
* custom software in support of licensee product, such licensee product to be
* used only in conjunction with Cypress's integrated circuit as specified in the
* applicable agreement. Any reproduction, modification, translation, compilation,
* or representation of this Software except as specified above is prohibited 
* without the express written permission of Cypress.
* 
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes to the Software without notice. 
* Cypress does not assume any liability arising out of the application or use
* of Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use as critical components in any products 
* where a malfunction or failure may reasonably be expected to result in 
* significant injury or death ("ACTIVE Risk Product"). By including Cypress's 
* product in a ACTIVE Risk Product, the manufacturer of such system or application
* assumes all risk of such use and in doing so indemnifies Cypress against all
* liability. Use of this Software may be limited by and subject to the applicable
* Cypress software license agreement.
************************************************************************************/

#if !defined(COMMON_H)
#define COMMON_H

#include <project.h>
#include <stdbool.h>
#include "stypes.h"


/*************************Macro Definitions**********************************/
#define die()   while(1);
#define blink() OnOff_LED_Write(!OnOff_LED_Read())
#define test()  TestPin_Write(!TestPin_Read())

/*  Bendri defainai   */
#define TRUE                0x01
#define FALSE               0x00
    
#define ZERO                0x00
#define ONE                 0x01
    
#define PIN_LOW             0x00
#define PIN_HIGH            0x01
    
#define DISABLED            0x00
#define NON_APPLICABLE      0x00
#define ENABLED             0x01
    
#define LED_OFF             0x00
#define LED_ON              0x01
    
#define TLEDS_OFF           0b0000000000000000
#define TLEDS_ON            0b1111111111111111
    
#define BYTE2INT(hi,low)    (int)((hi << 8) | low)
#define INT2DINT(hi,low)    (int)((hi << 16) | low) 
    
#define SETBIT(var,bit)     var |= 1 << bit
#define CLRBIT(var,bit)     var &= ~(1 << bit)
#define TOGGLEBIT(var,bit)  var ^= 1 << bit
#define CHKBIT(var,bit)     (var >> bit) & 1
    
#define N_ELEMENTS(x)       (sizeof(x)/sizeof(*(x)))
    
    
struct _time{
    uint32_t    WTime;
    uint8_t     t1s     :1;
    uint8_t     t60s    :1; 
    
    uint8_t     secsCounter    :6;
    uint8_t     minsCounter    :6;
    uint8_t     hoursCounter   :5;    
}Time;
    
    
    
    
    
/* GATT Db Alarms charakteristikos offsetai  */
#define ALRM_C0         0x00
#define ALRM_C1         0x01 
#define ALRM_C2         0x02
#define ALRM_C3         0x03 
#define ALRM_C4         0x04  
#define ALRM_C5         0x05
#define ALRM_C6         0x06 
#define ALRM_C7         0x07
#define ALRM_C8         0x08 
#define ALRM_C9         0x09
#define ALRM_C10        0x0A
#define ALRM_C11        0x0B 
#define ALRM_C12        0x0C
#define ALRM_C13        0x0D 
#define ALRM_C14        0x0E
#define ALRM_C15        0x0F
    
/* GATT Db TempSensors charakteristikos offsetai  */
#define SENS_C0         0x00
#define SENS_C1         0x01
#define SENS_C2         0x02
#define SENS_C3         0x03
#define SENS_C4         0x04
#define SENS_C5         0x05
#define SENS_C6         0x06
#define SENS_C7         0x07      
    
    
/* PRV plokstes cpecifiniai defainai  */
#define TS_TJ           SENS_C0   // TJ sensorio parodymai
#define TS_TA           SENS_C1   // TA sensorio parodymai
#define TS_TE           SENS_C2   // TE sensorio parodymai
#define TS_HR           SENS_C3   // HR sensorio parodymai
#define TS_WAT          SENS_C5   // TE sensorio parodymai
#define TS_OUT          SENS_C6   // HR sensorio parodymai

/*****************************************************************************/
#endif
/* [] END OF FILE */
