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
#include "main.h"
#include "common.h"
#include "bleapi.h"
#include "mb.h"
#include "mb_m.h"
#include "mpwm.h"
#include "rtos.h"
#include "capsen.h"
#include "adc.h"
#include "wdt.h"
#include "mi2c.h"
#include "mspi.h"
#include "leds.h"
#include "mbunit.h"


#define NESTED_ISR          (2u)
#define NES_DEF_PRIORITY    (3u)
#define NES_HIGH_PRIORITY   (2u)


extern CYBLE_API_RESULT_T CyTouchError;
extern _portdat MbPort;

extern bool DeviceConnected;            //This flag is set when a Central device is connected
extern bool RestartAdvertisement;       //This flag is used to start advertisement
extern bool BleDataPrepareRequired;
extern bool ChangeUartSettingsRequired;
extern bool BoostUpdateRequireBit;
//extern bool AuthKeyRequired;

extern CYTOUCH_CONTROL_T CyTouchControl;
extern const SERVICE_DEVICE_INFO_T CyTouchInfo;

mbdev_t *prv;
cydata_t DevData;


/* Internal function prototype */
CYBLE_API_RESULT_T HWInit( void );
CYBLE_API_RESULT_T CyTouchSettings( void );
CYBLE_API_RESULT_T SCB_ModeHandler( void );


/* ------------------------------ RTOS Tasks --------------------------------*/
void T_Control(void);
void T_Leds(void);
void T_MBus(void);
void T_Ble(void); 

static void RestoreDefaults(void);
CY_ISR_PROTO(NestedIsrHandler);


int main()
{
    CYBLE_API_RESULT_T      apiResult;
    static uint32_t         delay = (0u); 
    static uint32_t         BoostCounterDelay = (0u);

    CyGlobalIntEnable;
    
    ( void )CyTouchSettings(); 
    
    ( void )HWInit();  
    
    CheckLeds();
    
    Shed_Init();
    
    Shed_SetTask (T_Control,    (1000u), (50u));
    Shed_SetTask (T_Leds,       (2000u), (25u));
	Shed_SetTask (T_MBus,       (0u), (10u));
    Shed_SetTask (T_Ble,        (0u), (5u)); 
    
    
    apiResult = CyBle_Start( StackEvents );
    if( apiResult != CYBLE_ERROR_OK ) CYASSERT(0u);     
    
    prv = CreateSlave( 1u );
    if( prv == NULL ) CYASSERT(0u);
    
    if( UartConfig( MbPort.Settings.Baudrate.val, MbPort.Settings.Parity.val, MbPort.Settings.StopBits.val, MbPort.Settings.DataBits.val ) == CYBLE_ERROR_OK ){    
        ( void )eMBMasterInit( MB_RTU, (0u), MbPort.Settings.Baudrate.val, MB_PAR_NONE );
        ( void )eMBMasterEnable();       
    }  
    
    CyDelay(250);   //uzdelsimas reikalingas portTimer sustojimui po MbMaster inicializacijos
    
    
    
    
    // autopajeska veikiancio slaivo
    uint8_t i = 200;
    
    while( true ){        
        
        eMBMasterReqReportSlaveId( i );
        
        do{
            eMBMasterPoll();  
            CyDelayUs(100);
        }while( xMBMasterGetIsBusy() == true );
    
        if( prv->Status.IsRecognized == true ){
            prv->Data.addr = i;
            break;
        }
        
        if( i++ > 247 ) i = 1;
    }
    
    

    
    
    /***************************************************************************
    * Main polling loop
    ***************************************************************************/
    for(;;)
    {       
        
        if( delay < GetTicks() ){       
            delay = GetTicks() + (100u);    
            
//            Filter_LED_Write( !Filter_LED_Read() );
            
            
            
            AdcConvStart();
            
            // gavom restart komanda
            if( CyTouchControl.Status.Restart == true ){                
                CyTouchControl.Status.Restart = false;
                CyBle_GapDisconnect( cyBle_connHandle.bdHandle );
                CyDelay(1000u);
                CySoftwareReset();
            }
            
            if( CyTouchControl.Status.Defaults == true ){                
                CyTouchControl.Status.Defaults = false;
                EE_PutByte( EE_DATA_OK_FLAG_ADDR, (uint8_t*)0xFF);
                CyDelay(1000u);
                CySoftwareReset();
            }           
            
           
            ( void )SCB_ModeHandler();             
            
            if( BoostCounterDelay < GetTicks() ){
                BoostCounterDelay = GetTicks() + (1000u);
                
                /* nested interraptu aktyvavimas */
                CyIntSetPending(NESTED_ISR);
                
                // jai laikas nenustatytas, boostas isjungiamas rankiniu budu
                if( DevData.Boost.Time > (0u) ){        
                    if( ( DevData.Boost.Counter ) > (0u) ){
                        
                        // tikrinam vienetui, kad viena syki ideti i eile komanda MBusCmdWriteBoostCoil
                        if( ( DevData.Boost.Counter-- ) == (1u) ){
                            DevData.Boost.val = false;
                            BoostUpdateRequireBit = true;
                        }            
                    }
                }else{
                    DevData.Boost.Counter = (0u);
                }
                
                Time.t1s = !Time.t1s;
                
                if( Time.secsCounter < 60u )  Time.secsCounter++;
                else{
                    Time.secsCounter = (0u);
                    
                    Time.WTime++;                    
                    EE_PutDWord( EE_WTIME_ADDR, &Time.WTime );
                    
                    if( Time.minsCounter < 60u ) Time.minsCounter++;
                    else{
                        Time.minsCounter = (0u);
                        
                        if( Time.hoursCounter < 24u ) Time.hoursCounter++;
                        else{
                            Time.hoursCounter = (0u);   
                        }
                    }
                }                
            }   
        }
   
               
        Shed_DispatchTask(); 
        AdcProcess();
        
        HandleBleProcess(); 
        CyBle_ProcessEvents();        
        
//        if( AuthKeyRequired == true ){
//            AuthKeyRequired = false;
//            
//            (void)CyBle_GapAuthPassKeyReply(cyBle_connHandle.bdHandle, 111111, 1);
//            
//            CyBle_ProcessEvents();
//        }
        
        WDT0_RESET
    }
}

CYBLE_API_RESULT_T SCB_ModeHandler(){
    
    // jai porto konfiguracija pasikeite, keiciam ja fiziskai
    if( ChangeUartSettingsRequired == true ){                
        ChangeUartSettingsRequired = false;   

        return SCB_ConfigurationChange( MbPort.Mode ); 
    }
    
    if( DeviceConnected == true){
        
        if( CyTouchControl.Status.Ble2Uart == true && MbPort.Mode != SCB_BLE){
            return SCB_ConfigurationChange( SCB_BLE );
        }
        if( CyTouchControl.Status.Ble2Uart == false && MbPort.Mode != SCB_MBUS){
            return SCB_ConfigurationChange( SCB_MBUS );
        }            
    }else{
        if( CyTouchControl.Status.Ble2Uart == true && MbPort.Mode == SCB_BLE){
            CyTouchControl.Status.Ble2Uart = false;
            return SCB_ConfigurationChange( SCB_MBUS );
        }            
    }
    
    return CYBLE_ERROR_INVALID_STATE;
}

CYBLE_API_RESULT_T HWInit(){
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    /* nested interaptu nustatymas */
    CyIntSetVector(NESTED_ISR, NestedIsrHandler);
    CyIntSetPriority(NESTED_ISR, NES_DEF_PRIORITY);
    CyIntEnable(NESTED_ISR);
    
    SysTickInit();   
    WdtInit();
    
    WDT0_DISABLE    
    
    CapSenseInit();
    
    AdcInit();

    SoundInit(true, CyTouchControl.Sound.Level);   
    BacklightInit(true, CyTouchControl.Blank.Timeout.val);    
    BlStartTimeout();
    
    InitLEDS(); 
    
    SetSoundTone(3u);
    
    return apiResult;
}

CYBLE_API_RESULT_T CyTouchSettings(){    
    uint8_t var;
    
//    EE_PutByte( EE_DATA_OK_FLAG_ADDR, 0xFF);
    
    EE_GetByte( EE_DATA_OK_FLAG_ADDR, &var );
    
    if( var != EE_DATA_OK_FLAG_VAL ) RestoreDefaults(); 
    
    EE_GetDWord( EE_WTIME_ADDR, &Time.WTime );
    
    EE_GetArray( MbPort.Settings.data, EE_MB_DATA_ADDR, sizeof( MbPort.Settings.data ));
    MbPort.Mode = SCB_MBUS;

    EE_GetArray( CyTouchControl.TSet.data, EE_TSET_RANGE_ADDR, sizeof( CyTouchControl.TSet.data ));
    EE_GetArray( CyTouchControl.Passwd, EE_PASSWD_ADDR, sizeof( CyTouchControl.Passwd ));
    EE_GetArray( CyTouchControl.LockTimer.data, EE_LOCKTIMER_ADDR, sizeof( CyTouchControl.LockTimer.data ));

    CyTouchControl.Sound.Level = MIDLE;   
    CyTouchControl.Blank.Timeout.val = BL_TIMEOUT;
    CyTouchControl.Blank.MinValue.val = BL_MIN;
    CyTouchControl.Blank.MaxValue.val = BL_MAX;
    CyTouchControl.Status.SoundEna = true;
    CyTouchControl.Status.BlankEna = true;
    CyTouchControl.Status.Restart = false;
    CyTouchControl.Status.OnOff = true;   
    CyTouchControl.Status.Autolock = true;
    CyTouchControl.Status.Locked = false;
    
    DevData.Boost.Counter = (0u);
    DevData.Boost.val = false;
    DevData.Boost.Time = (5u);    

    return CYBLE_ERROR_OK;
}


void RestoreDefaults(){
    
    uint8_t var = 0xAA;
    
    EE_GetDWord( EE_WTIME_ADDR, &Time.WTime );    
    
    EE_EraseChip();
    
    EE_PutDWord( EE_WTIME_ADDR, &Time.WTime );
    
    MbPort.Settings.Baudrate.val = (19200u);
    MbPort.Settings.Parity.val = SCB_UART_PARITY_NONE;
    MbPort.Settings.StopBits.val = SCB_UART_STOP_BITS_1;
    MbPort.Settings.DataBits.val = (8u);    
    EE_PutArray( MbPort.Settings.data, EE_MB_DATA_ADDR, sizeof( MbPort.Settings.data )); 
   
    CyTouchControl.TSet.MinValue.val = 15;
    CyTouchControl.TSet.MinRange.valLow = -5;
    CyTouchControl.TSet.MinRange.valHigh = 30;
    EE_PutArray( CyTouchControl.TSet.data, EE_TSET_RANGE_ADDR, sizeof( CyTouchControl.TSet.data ));
    
    CyTouchControl.Passwd[0] = '4';
    CyTouchControl.Passwd[1] = '4';
    CyTouchControl.Passwd[2] = '4';
    CyTouchControl.Passwd[3] = '4';
    EE_PutArray( CyTouchControl.Passwd, EE_PASSWD_ADDR, sizeof( CyTouchControl.Passwd ));
    
    CyTouchControl.LockTimer.val = LOCK_TIMER_DEF;
    EE_PutArray( CyTouchControl.LockTimer.data, EE_LOCKTIMER_ADDR, sizeof( CyTouchControl.LockTimer.data ));
    
    EE_PutByte( EE_DATA_OK_FLAG_ADDR, &var );   
}


/* Tikrinam ar baitas yra skaicius */
uint8_t IsDigit(uint8_t byte)
{
    if(byte >= 0x30 && byte <= 0x39) return true;
    return false;
}


CY_ISR(NestedIsrHandler)
{
    /* Clear pending Interrupt */
    CyIntClearPending(NESTED_ISR);
}

/* [] END OF FILE */
