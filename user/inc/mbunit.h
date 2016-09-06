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
#if !defined(MBUNIT_H)
#define MBUNIT_H

#include <project.h>
#include "mb_m.h"
#include "user_mb_app.h"
    
    
#define MB_ADDRESS_BROADCAST    ( 0 )   /*! Modbus broadcast address. */
#define MB_ADDRESS_MIN          ( 1 )   /*! Smallest possible slave address. */
#define MB_ADDRESS_MAX          ( 247 ) /*! Biggest possible slave address. */
    
#define MB_M_CONNECT_NTEST      (6u)    //kiek kartu bandom konnektintis prie slave'o pries ismetant konnekto klaida
#define MB_READ_ID_DELAY        (5000u)
#define MB_READ_DELAY           (100u)
#define MB_WRITE_DELAY          (3000u)
    
typedef enum { UNKNOWN = 0, PRVE, PRVW, VENTIKE, VENTIKW, REGULITE }stype_t;

#pragma pack(push, 1)
typedef struct
{
    struct{
        uint8_t             addr;           // PRV plokstes Modbus adresas
        uint32_t            BoostCounter;   // PRV Boost skaitliuko registras (reiksme gaunam is PRV plokstes)
    }Data;
    union{
        uint8_t             data[3];
        struct{
            struct{
                stype_t         Type                    :4;
                uint8_t         IsAlive                 :1;
                uint8_t         IsRecognized            :1;
                uint8_t         ConnectionStateCounter  :4;
            };
            struct{
                uint8_t         StateOnOff              :1;
                uint8_t         SpeedLowActive          :1;    
                uint8_t         BoostActive             :1;
            }Res;
        };
    }Status;
    struct{
        union SlaveID       *SlaveId;
        USHORT              *HoldingRegisters;      // Holding Regiter buferio pointeris
        UCHAR               *Coils;                 // Coil buferio pointeris
        UCHAR               *DiscreteInputs;        // Discrete Input buferio pointeris
        USHORT              *InputRegisters;        // Input Register buferiu pointeris
    }Buffers;
    struct{      // buferiai, kur saugomi pavieniai (nuskaityti is PRV plokstes po viena) registrai
        struct _hregs       *HoldingRegisters;         // Holding Regiter buferio pointeris
        struct _coils       *Coils;
        struct _dinputs     *DiscreteInputs;
        struct _iregs       *InputRegisters;
    }ShortBuffers;
    struct{     // intrfeiso metodai
        bool (*ptrCmdWriteSpeedReg)( uint8_t addr, uint8_t value );
        bool (*ptrCmdWriteTSetReg)( uint8_t addr, uint8_t value );
        bool (*ptrCmdWriteBoostCoil)( uint8_t addr, bool value );
        bool (*ptrCmdWriteBoostCounterReg)( uint8_t addr, uint8_t value );
        bool (*ptrMBusCmdWriteReboot)( uint8_t addr, uint8_t value );
        bool (*ptrMBusCmdWritePrvSettings)( uint8_t addr, uint8_t value );
    }IFace;
}mbdev_t;
#pragma pack(pop)
    

mbdev_t *CreateSlave( uint8_t addr );
mbdev_t *GetSlave(uint8_t addr);

void CheckPrvType(mbdev_t *dev);
void ClearSlaveData( mbdev_t *dev );
    
   
/*****************************************************************************/
#endif
/* [] END OF FILE */
