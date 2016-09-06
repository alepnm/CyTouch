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
#ifndef _LEDS_H
#define _LEDS_H
#include <project.h>
#include <stdbool.h>

/*  led defines  */
enum{
    LEDT1  = (1 << 4u),
    LEDT2  = (1 << 5u),
    LEDT3  = (1 << 6u),
    LEDT4  = (1 << 7u),
    LEDT5  = 1u,
    LEDT6  = (1 << 1u),
    LEDT7  = (1 << 2u),
    LEDT8  = (1 << 3u),
    LEDT9  = (1 << 12u),
    LEDT10 = (1 << 13u),
    LEDT11 = (1 << 14u),
    LEDT12 = (1 << 15u),
    LEDT13 = (1 << 8u),
    LEDT14 = (1 << 9u),
    LEDT15 = (1 << 10u),
    LEDT16 = (1 << 11u)
};  


#pragma pack(push,1)
struct _leds{
    struct{
        bool     State      :1;
        uint32_t Counter;
    }OnOffLed;
    struct{
        union{
            uint8_t data[2];
            uint16_t Value;
        };
        uint32_t Counter;
    }TLED;
    struct{
        bool     State      :1;
        uint32_t Counter;
    }Sp1Led;
    struct{
        bool     State      :1;
        uint32_t Counter;
    }Sp2Led;
    struct{
        bool     State      :1;
        uint32_t Counter;
    }Sp3Led;
    struct{
        bool     State      :1;
        uint32_t Counter;
    }BoostLed;
    struct{
        bool     State      :1;
        uint32_t Counter;
        uint16_t counter_val;
    }BleLed;
    struct{
        bool     State      :1;
        uint32_t Counter;
    }ServiceLed;
    struct{
        bool     State      :1;
        uint32_t Counter;
    }FilterLed;
};
#pragma pack(pop)
    
void InitLEDS( void );
void CheckLeds( void );

bool RunLeds_1(void);
bool RunLeds_2(void);
bool RunLeds_3(void);

#endif
/* [] END OF FILE */
