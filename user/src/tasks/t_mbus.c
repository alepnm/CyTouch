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
#include <project.h>
#include <stdbool.h>
#include "mb.h"
#include "mb_m.h"
#include "BleUART.h"
#include "systick.h"
#include "user_mb_app.h"
#include "mbunit.h"
#include "mbqueue.h"
#include "mbutils.h"
#include "capsen.h"
#include "mbqueue.h"
#include "leds.h"
#include "buffers.h"


#include "iprv.h"
#include "iventik.h"


/* ******************************* Externs ********************************* */
extern mbdev_t *prv;
extern cydata_t DevData;

extern bool     TSetUpdateRequireBit;
extern bool     FanSpeedUpdateRequireBit;
extern bool     BoostUpdateRequireBit;
extern bool     PrvResetRequireBit;
extern bool     PrvSettingsUpdateRequireBit;

extern uint16_t ShowWhatTimer;

extern USHORT   usMRegHoldStart;
extern USHORT   usMCoilStart;

eMBErrorCode    MbPoolResult;

bool            PrvDataPrepared = false;
bool            MbStackFree = false;


void T_MBus(){  // T = 10ms    
    static uint32_t delay_a = ( 0u );
    static uint8_t  delay_b = ( 0u );
    
    MbStackFree = false;
    
    switch( eMBMasterPoll() ){
        case MB_ENOERR:
            if( xMBMasterGetIsBusy() == false ) MbStackFree = true;      
            break;
        case MB_EIO:    // exception error
        
            break;
        case MB_ETIMEDOUT:  // timeout error
        
            break;
        case MB_ENOREG:
        case MB_EINVAL:
        case MB_EPORTERR:
        case MB_ENORES:
        case MB_EILLSTATE:
        
            break;
    }
    
    if( MbStackFree == false ) return;   

    ( void )ExecMbCmdFromQueue();
    
    if( delay_a < GetTicks() ){
        delay_a = GetTicks() + MB_READ_ID_DELAY;
      
        if( QueueCmdReportSlaveId( prv ) == false ) die() 
        
        delay_b = 100u;
        
        if( prv->Status.IsRecognized == true ){
            // cia tikrinam koks tipas atpazintas, palei ka pasirinkam interfeiso funkcijas
            switch(prv->Status.Type){
                case UNKNOWN:
                    break;
                case PRVE:
                case PRVW:
               
                    prv->IFace.ptrCmdWriteSpeedReg = PrvMBusCmdWriteSpeedReg;
                    prv->IFace.ptrCmdWriteTSetReg = PrvMBusCmdWriteTSetReg;
                    prv->IFace.ptrCmdWriteBoostCoil = PrvMBusCmdWriteBoostCoil;
                    prv->IFace.ptrCmdWriteBoostCounterReg = PrvMBusCmdWriteBoostCounterReg;
                    prv->IFace.ptrMBusCmdWriteReboot = PrvMBusCmdWriteReboot;
                    prv->IFace.ptrMBusCmdWritePrvSettings = PrvMBusCmdWritePrvSettings;
                
                    ptrGetDataFromSlave = PrvGetDataFromSlave;
                    ptrSendDataToSlave = PrvSendDataToSlave;
                    break;
                case VENTIKE:
                case VENTIKW:
            
                    prv->IFace.ptrCmdWriteSpeedReg = VentikMBusCmdWriteSpeedReg;
                    prv->IFace.ptrCmdWriteTSetReg = VentikMBusCmdWriteTSetReg;
                    prv->IFace.ptrCmdWriteBoostCoil = NULL;
                    prv->IFace.ptrCmdWriteBoostCounterReg = NULL;
                    prv->IFace.ptrMBusCmdWriteReboot = VentikMBusCmdWriteReboot;
                    prv->IFace.ptrMBusCmdWritePrvSettings = VentikMBusCmdWritePrvSettings;
                
                    ptrGetDataFromSlave = VentikGetDataFromSlave;
                    ptrSendDataToSlave = VentikSendDataToSlave;
                    break;
                case REGULITE:
                
                    break;
            }
            
            PrvDataPrepared = true;
        }
    }
   
    if( PrvDataPrepared == false ) return;
    
    if( TSetUpdateRequireBit || FanSpeedUpdateRequireBit || BoostUpdateRequireBit || PrvResetRequireBit || PrvSettingsUpdateRequireBit ){
        
//        ShowWhatTimer = 1000;
        
        ( *ptrSendDataToSlave )( prv ); 
        delay_b = ( 0u );
    }else{    
        if( delay_b++ < ( 100u ) || CapSenseIsActive() == true ) return;
        
        delay_b = ( 0u );
        ( *ptrGetDataFromSlave )( prv );
    }     
}

/* [] END OF FILE */
