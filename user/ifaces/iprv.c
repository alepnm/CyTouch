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
#include "iprv.h"
#include "systick.h"
#include "mbqueue.h"
#include "buffers.h"
#include "leds.h"

extern mbdev_t *prv;
extern cydata_t DevData;

extern bool     TSetUpdateRequireBit;
extern bool     FanSpeedUpdateRequireBit;
extern bool     BoostUpdateRequireBit;
extern bool     PrvResetRequireBit;
extern bool     PrvSettingsUpdateRequireBit;

extern uint16_t ShowWhatTimer;


bool PrvMBusCmdWriteSpeedReg( uint8_t addr, uint8_t value ){
    return QueueCmdWriteHoldingReg( addr, usMRegHoldStart + PRV_H_SPEED, value );    
}
bool PrvMBusCmdWriteTSetReg( uint8_t addr, uint8_t value ){
    return QueueCmdWriteHoldingReg( addr, usMRegHoldStart + PRV_H_TSET, value );
}
bool PrvMBusCmdWriteBoostCoil( uint8_t addr, bool value ){
    return QueueCmdWriteCoil( addr, usMCoilStart + PRV_C_BOOST, (value) ? 0xFF00 : 0x0000 );
}
bool PrvMBusCmdWriteBoostCounterReg( uint8_t addr, uint8_t value ){
    return QueueCmdWriteHoldingReg( addr, usMRegHoldStart + PRV_H_BOOST_COUNT, value );
}
bool PrvMBusCmdWriteReboot( uint8_t addr, uint8_t value ){
    return QueueCmdWriteCoil( addr, usMCoilStart + PRV_C_RESET, (value) ? 0xFF00 : 0x0000 );
}

bool PrvMBusCmdWritePrvSettings( uint8_t addr, uint8_t value ){
//    return QueueCmdWriteCoil( addr, usMCoilStart + PRV_C_RESET, (value) ? 0xFF00 : 0x0000 );
    
    return false;
}


static void PrvGetTempSensValuesFromBuffer(mbdev_t *sl){ 
    DevData.Sensors.sn0 = (uint8_t)( sl->Buffers.InputRegisters[PRV_I_TJ] / 10 );
    DevData.Sensors.sn1 = (uint8_t)( sl->Buffers.InputRegisters[PRV_I_TA] / 10 );
    DevData.Sensors.sn2 = (uint8_t)( sl->Buffers.InputRegisters[PRV_I_TE] / 10 );
    DevData.Sensors.sn3 = (uint8_t)( sl->Buffers.InputRegisters[PRV_I_TOUT] / 10 );
    DevData.Sensors.sn4 = (uint8_t)( sl->Buffers.InputRegisters[PRV_I_TWAT] / 10 );
    DevData.Sensors.sn7 = (uint8_t)( sl->Buffers.InputRegisters[PRV_I_RH] );
}
static void PrvGetSpeedValueFromBuffer(mbdev_t *sl){

    if( FanSpeedUpdateRequireBit == false ){
    
        /* jai skaitom kruvoj */
        if( sl->Buffers.HoldingRegisters[PRV_H_SPEED] > (3u) ){
            sl->Buffers.HoldingRegisters[PRV_H_SPEED] = (3u);    
        }      
        DevData.Speed.val = sl->Buffers.HoldingRegisters[usMRegHoldStart + PRV_H_SPEED];

        
        if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_BOOST, 1) ) prv->Status.Res.BoostActive = true;
        else prv->Status.Res.BoostActive = false;
    }
}
static void PrvGetTSetValueFromBuffer(mbdev_t *sl){   
    
    if( TSetUpdateRequireBit == false ){
        /* jai skaitom kruvoj */
        if(DevData.TSet.val != sl->Buffers.HoldingRegisters[usMRegHoldStart + PRV_H_TSET]){
            DevData.TSet.val = sl->Buffers.HoldingRegisters[usMRegHoldStart + PRV_H_TSET];
            ShowWhatTimer = 1000;
        }
    }
}

static void PrvGetPrvModeFromBuffer(mbdev_t *sl){
    
    if( PrvSettingsUpdateRequireBit == false ){
        sl->Data.BoostCounter = (sl->ShortBuffers.HoldingRegisters->Register[0].Value );
        DevData.PrvSettings.OnOff = (sl->ShortBuffers.HoldingRegisters->Register[1].Value );
        DevData.PrvSettings.Mode = (sl->ShortBuffers.HoldingRegisters->Register[2].Value );
    }
}


void PrvCollectAlarmsFromBuffer(mbdev_t *sl){
    
    DevData.Alarms.val = (0u);
    
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_ANTIFROST_ALRM, 1) )    DevData.Alarms.val |= LEDT1;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_FIRE_ALRM, 1) )         DevData.Alarms.val |= LEDT2;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_OVERHEAT_ALRM, 1) )     DevData.Alarms.val |= LEDT3;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_FAN_ALRM, 1) )          DevData.Alarms.val |= LEDT4;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_TJ_ALRM, 1) )           DevData.Alarms.val |= LEDT5;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_TE_ALRM, 1) )           DevData.Alarms.val |= LEDT6;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_TA_ALRM, 1) )           DevData.Alarms.val |= LEDT7;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_RH_ALRM, 1) )           DevData.Alarms.val |= LEDT8;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_TWAT_ALRM, 1) )         DevData.Alarms.val |= LEDT9;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_TOUT_ALRM, 1) )         DevData.Alarms.val |= LEDT10;
    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_ROTOR_ALRM, 1) )        DevData.Alarms.val |= LEDT11;

    if( xMBUtilGetBits(sl->Buffers.Coils, PRV_C_OLDRC_COMP, 1) )        DevData.Alarms.val |= LEDT15;    
    if( DevData.PrvSettings.Mode != 1u || DevData.PrvSettings.OnOff == true )    DevData.Alarms.val |= LEDT16;    
    
    DevData.Alarms.Service = ( DevData.Alarms.val > 0u ) ? true : false;
    DevData.Alarms.Filter = xMBUtilGetBits(sl->Buffers.DiscreteInputs, PRV_C_FILTER_ALRM, 1u);
}



void PrvGetDataFromSlave( mbdev_t *slave ){
    static uint8_t stage = ( 0u );
    static uint32_t delay = ( 0u );
   
    if(delay > GetTicks()) return;
    delay = GetTicks() + MB_READ_DELAY;

    switch(stage){
        case 0:
        QueueCmdReadCoils( slave->Data.addr, usMCoilStart, M_COIL_NCOILS );
        QueueCmdReadDescreetInputs( slave->Data.addr, usMDiscInStart, M_DISCRETE_INPUT_NDISCRETES );
        QueueCmdReadInputRegs( slave->Data.addr, usMRegInStart, M_REG_INPUT_NREGS );
        QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart, M_REG_HOLDING_NREGS );
        
        QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart + PRV_H_BOOST_COUNT, 1u );
        QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart + PRV_H_OFF_MODE, 1u );
        QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart + PRV_H_MANUAL_MODE, 1u );

        stage++;
        break;
        case 1:        
        PrvGetTSetValueFromBuffer( slave );
        PrvGetSpeedValueFromBuffer( slave );
        PrvGetTempSensValuesFromBuffer( slave );
        PrvCollectAlarmsFromBuffer( slave );
        PrvGetPrvModeFromBuffer( slave );
        stage = ( 0u );
        break;
    }   
}
void PrvSendDataToSlave( mbdev_t *slave ){
    static uint32_t delay = ( 0u );
    
    if(delay > GetTicks()) return;
    delay = GetTicks() + MB_WRITE_DELAY;  
   
    if( TSetUpdateRequireBit == true && slave->IFace.ptrCmdWriteTSetReg != NULL ){        
        if( slave->IFace.ptrCmdWriteTSetReg( slave->Data.addr, DevData.TSet.val ) == true ){
            TSetUpdateRequireBit = false; 
        }
    }  
    if( FanSpeedUpdateRequireBit == true && slave->IFace.ptrCmdWriteSpeedReg != NULL ){
        if( slave->IFace.ptrCmdWriteSpeedReg( slave->Data.addr, DevData.Speed.val ) == true ){
            FanSpeedUpdateRequireBit = false;
        }
    }
    if( BoostUpdateRequireBit == true && slave->IFace.ptrCmdWriteBoostCoil != NULL ){
        if( slave->IFace.ptrCmdWriteBoostCoil( slave->Data.addr, DevData.Boost.val ) == true ){
            
            if( DevData.Boost.val == true ){
                if( slave->Data.BoostCounter == ( 0u ) ){
                    DevData.Boost.Counter = ( 60u ) * DevData.Boost.Time;
                }
            }else{
                DevData.Boost.Counter = ( 0u );
            }
            
            BoostUpdateRequireBit = false;
        }
    }
    
    if( PrvResetRequireBit == true && slave->IFace.ptrMBusCmdWriteReboot != NULL ){
        if( slave->IFace.ptrMBusCmdWriteReboot( slave->Data.addr, true ) == true ){
            PrvResetRequireBit = false;
        }
    }
    
    if( PrvSettingsUpdateRequireBit == true ){
        
        PrvSettingsUpdateRequireBit = false;
        
    }
    
}


/* [] END OF FILE */
