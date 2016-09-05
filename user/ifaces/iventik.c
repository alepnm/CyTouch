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
#include "iventik.h"
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

extern uint16_t ShowWhatTimer;

bool VentikMBusCmdWriteSpeedReg( uint8_t addr, uint8_t value ){
    return QueueCmdWriteHoldingReg( addr, usMRegHoldStart + VENTIK_H_SPEED, value );    
}
bool VentikMBusCmdWriteTSetReg( uint8_t addr, uint8_t value ){
    return QueueCmdWriteHoldingReg( addr, usMRegHoldStart + VENTIK_H_TSET, value );
}

bool VentikMBusCmdWriteReboot( uint8_t addr, uint8_t value ){
    return QueueCmdWriteCoil( addr, usMCoilStart + VENTIK_C_RESET, (value) ? 0xFF00 : 0x0000 );
}

bool VentikMBusCmdWritePrvSettings( uint8_t addr, uint8_t value ){
//    return QueueCmdWriteCoil( addr, usMCoilStart + PRV_C_RESET, (value) ? 0xFF00 : 0x0000 );
    
    return false;
}


static void VentikGetTempSensValuesFromBuffer(mbdev_t *sl){ 
    
    /* jai skaitom kruvoj */
    DevData.Sensors.sn0 = (uint8_t)( sl->Buffers.InputRegisters[VENTIK_I_TJ] / 10 );
    DevData.Sensors.sn1 = (uint8_t)( sl->Buffers.InputRegisters[VENTIK_I_TOUT] / 10 );
    DevData.Sensors.sn2 = (uint8_t)( sl->Buffers.InputRegisters[VENTIK_I_TWAT] / 10 );   
}
static void VentikGetSpeedValueFromBuffer(mbdev_t *sl){

    /* jai skaitom kruvoj */
    if( sl->Buffers.HoldingRegisters[VENTIK_H_SPEED] > (3u) ){
        sl->Buffers.HoldingRegisters[VENTIK_H_SPEED] = (3u);    
    }  
    
    DevData.Speed.val = sl->Buffers.HoldingRegisters[usMRegHoldStart + VENTIK_H_SPEED];
}
static void VentikGetTSetValueFromBuffer(mbdev_t *sl){
    
    /* jai skaitom kruvoj */
    if(DevData.TSet.val != sl->Buffers.HoldingRegisters[usMRegHoldStart + VENTIK_H_TSET]){
        DevData.TSet.val = sl->Buffers.HoldingRegisters[usMRegHoldStart + VENTIK_H_TSET];
        ShowWhatTimer = (1000u);
    }
    DevData.TSet.val = sl->Buffers.HoldingRegisters[usMRegHoldStart + VENTIK_H_TSET];
}
static void VentikCollectAlarmsFromBuffer(mbdev_t *sl){
   
    DevData.Alarms.val = (0u);
    
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_LOWVOLTAGE_ALRM, 1) )       DevData.Alarms.val |= LEDT1;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_FIRE_ALRM, 1) )             DevData.Alarms.val |= LEDT2;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_CRITICAL_RET_ALRM, 1) )     DevData.Alarms.val |= LEDT3;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_FAN_ALRM, 1) )              DevData.Alarms.val |= LEDT4;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_TJ_ALRM, 1) )               DevData.Alarms.val |= LEDT5;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_TSUP_TOOLOW_ALRM, 1) )      DevData.Alarms.val |= LEDT6;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_TSUP_TOOHIGH_ALRM, 1) )     DevData.Alarms.val |= LEDT7;
    
    
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_TWAT_ALRM, 1) )             DevData.Alarms.val |= LEDT9;
    if( xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_TOUT_ALRM, 1) )             DevData.Alarms.val |= LEDT10;   
    
    
    DevData.Alarms.Service = ( DevData.Alarms.val > 0u ) ? true : false;
    
    DevData.Alarms.Filter = xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_FILTER_ALRM, 1);       
    prv->Status.Res.SpeedLowActive = xMBUtilGetBits(sl->Buffers.DiscreteInputs, VENTIK_D_SPEEDLOW, 1);
    
    prv->Status.Res.BoostActive = false;
}



void VentikGetDataFromSlave( mbdev_t *slave ){
    static uint8_t stage = ( 0u );
    static uint32_t delay = ( 0u );
    
    if(delay > GetTicks() ) return;
    delay = GetTicks() + MB_READ_DELAY;
    
    switch(stage){
        case 0:
            QueueCmdReadCoils( slave->Data.addr, usMCoilStart, 32 );
            QueueCmdReadDescreetInputs( slave->Data.addr, usMDiscInStart, 48 );
            
            
            
            QueueCmdReadInputRegs( slave->Data.addr, usMRegInStart, M_REG_INPUT_NREGS );            
//            QueueCmdReadInputRegs( slave->Data.addr, usMRegInStart + VENTIK_I_TJ, 1 );
//            QueueCmdReadInputRegs( slave->Data.addr, usMRegInStart + VENTIK_I_TOUT, 1 );
//            QueueCmdReadInputRegs( slave->Data.addr, usMRegInStart + VENTIK_I_TWAT, 1 );
            
            
            QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart, M_REG_HOLDING_NREGS );       
//            QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart + VENTIK_H_SPEED, 1 );
//            QueueCmdReadHoldingRegs( slave->Data.addr, usMRegHoldStart + VENTIK_H_TSET, 1 );  
          
            stage++;
            break;
        case 1:        
            VentikGetTSetValueFromBuffer( slave );
            VentikGetSpeedValueFromBuffer( slave );
            VentikGetTempSensValuesFromBuffer( slave );
            VentikCollectAlarmsFromBuffer( slave );
            stage = ( 0u );
            break;
    }   
}
void VentikSendDataToSlave( mbdev_t *slave ){
    static uint32_t delay = ( 0u );
    
    if(delay > GetTicks()) return;
    delay = GetTicks() + MB_WRITE_DELAY;  
   
    if( TSetUpdateRequireBit == true && slave->IFace.ptrCmdWriteTSetReg != NULL ){
        
        ShowWhatTimer = (1000u);
        
        if( slave->IFace.ptrCmdWriteTSetReg( slave->Data.addr, DevData.TSet.val ) == true ){
            TSetUpdateRequireBit = false; 
        }        
    }    
    if( FanSpeedUpdateRequireBit == true && slave->IFace.ptrCmdWriteSpeedReg != NULL ){
        if( slave->IFace.ptrCmdWriteSpeedReg( slave->Data.addr, DevData.Speed.val ) == true ){
            FanSpeedUpdateRequireBit = false;
        }
    } 
    
    if(BoostUpdateRequireBit == true){
        BoostUpdateRequireBit = false;
        DevData.Boost.val = ( 0u );
    }
    
    if( PrvResetRequireBit == true ){
        if( slave->IFace.ptrMBusCmdWriteReboot( slave->Data.addr, true ) == true ){
            PrvResetRequireBit = false;
        }
    }
}

/* [] END OF FILE */
