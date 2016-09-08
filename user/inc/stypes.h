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
#if !defined(CYTYPES_H)
#define CYTYPES_H
    
#include <stdint.h>
#include <stdbool.h>   
    
    
#pragma pack(push,1)   
typedef union{
    uint8_t data[20];
    struct{
        struct{                
            union{
                uint8_t data[1];
                struct{
                    uint8_t Level   :5;
                };   //1
            }Sound;
            struct{
                union{
                    uint8_t     data[2];
                    uint16_t    val;
                }Timeout;
                union{
                    uint8_t     data[2];
                    uint16_t    val;
                }MinValue;
                union{
                    uint8_t     data[2];
                    uint16_t    val;
                }MaxValue;
            }Blank;   //6
            union{
                uint8_t data[3];
                struct{
                    union{
                        uint8_t     data[1];
                        int8_t      val;        // vartotojo minimali temperaturu diapazono reiksme
                    }MinValue;
                    union{
                        uint8_t     data[2];
                        struct{
                            int8_t  valLow;     // minimaliu temperaturu diapazono LOW taskas
                            int8_t  valHigh;    // minimaliu temperaturu diapazono HIGH taskas 
                        };
                    }MinRange; 
                };
            }TSet;     //3
            union{
                uint8_t    data[2];
                struct{
                    uint8_t OnOff       :1;
                    uint8_t SoundEna    :1;     // pypsas ijungtas/isjungtas
                    uint8_t BlankEna    :1;     // prigesimas ijungtas/isjungtas
                    uint8_t Ble2Uart    :1;     // Ble->Uart rezimo ijungimas/isjungimas
                    uint8_t Restart     :1;     // restarto bitas - irasius 1 pultas restartuoja
                    uint8_t Autolock    :1;     // automatinis valdymo blokavimas ijungtas/isjungtas
                    uint8_t Locked      :1;     // valdymo blokavimo busena
                    uint8_t Defaults    :1;     // Defaults atstatymo bitas - irasius 1 atsistato defaultines reiksmes
                };
            }Status;   //2
        };
        uint8_t Passwd[4];      //4
        union{
            uint8_t data[4];
            uint32_t val;
        }LockTimer;
    }; 
} CYTOUCH_CONTROL_T; 
typedef struct{
    uint8_t Manufacture[12];
    uint8_t Model[12];
    uint8_t Serial[12];
    uint8_t HW[5];
    uint8_t SW[5];
} SERVICE_DEVICE_INFO_T;
typedef struct{
    union{
        uint8_t     data[3];
        struct{
            uint16_t    val;
            uint8_t     Filter          :1;
            uint8_t     Service         :1;
        };
    };
} CYTOUCH_ALARMS_CHARACTERISTIC_T;
typedef union{
    uint8_t     data[1u];
    int8_t      val;
} CYTOUCH_TSET_CHARACTERISTIC_T;
typedef union{
    uint8_t         data[8u];
    struct{
        int8_t      sn0;
        int8_t      sn1;
        int8_t      sn2;
        int8_t      sn3;
        int8_t      sn4;
        int8_t      sn5;
        int8_t      sn6;
        int8_t      sn7;
    };
} CYTOUCH_TEMPSENSORS_CHARACTERISTIC_T;
typedef union{
        uint8_t     data[1u];
        uint8_t     val;
} CYTOUCH_FANSPEED_CHARACTERISTIC_T;
typedef struct{
    union{
        uint8_t     data[4];
        struct{
            uint8_t     val;
            uint16_t    Counter;
            uint8_t     Time;
        };
    };
} CYTOUCH_BOOST_CHARACTERISTIC_T;
typedef union{
    uint8_t     data[2];
    struct{
        uint8_t Compatibility   :1;
        uint8_t OnOff           :1;
        uint8_t Mode            :2;
    };
} CYTOUCH_PRVSETTINGS_CHARACTERISTIC_T;

#pragma pack(pop)

typedef struct{
    CYTOUCH_ALARMS_CHARACTERISTIC_T         Alarms;
    CYTOUCH_TEMPSENSORS_CHARACTERISTIC_T    Sensors;
    CYTOUCH_TSET_CHARACTERISTIC_T           TSet;
    CYTOUCH_FANSPEED_CHARACTERISTIC_T       Speed;
    CYTOUCH_BOOST_CHARACTERISTIC_T          Boost;
    CYTOUCH_PRVSETTINGS_CHARACTERISTIC_T    PrvSettings;
}cydata_t;
    
/*****************************************************************************/
#endif
/* [] END OF FILE */
