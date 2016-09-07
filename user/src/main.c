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

extern uint16_t ResponceTimeout;

extern CYTOUCH_CONTROL_T CyTouchControl;
extern const SERVICE_DEVICE_INFO_T CyTouchInfo;

mbdev_t *prv;
cydata_t DevData;


/* Internal function prototype */
static CYBLE_API_RESULT_T HWInit( void );
static CYBLE_API_RESULT_T CyTouchSettings( void );
static CYBLE_API_RESULT_T SCB_ModeHandler( void );


/* ------------------------------ RTOS Tasks --------------------------------*/
void T_Control(void);
void T_Leds(void);
void T_MBus(void);
void T_Ble(void); 

static void FindSlaveDevice(void);
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
    
    //if( UartConfig( MbPort.Settings.Baudrate.val, MbPort.Settings.Parity.val, MbPort.Settings.StopBits.val, MbPort.Settings.DataBits.val ) == CYBLE_ERROR_OK ){ 
    if( UartConfig( 19200u, SCB_UART_PARITY_EVEN, SCB_UART_STOP_BITS_1, 8u ) == CYBLE_ERROR_OK ){
        ( void )eMBMasterInit( MB_RTU, (0u), MbPort.Settings.Baudrate.val, MB_PAR_NONE );
        ( void )eMBMasterEnable();
    }  

       
    prv = CreateSlave( 1u );
    if( prv == NULL ) CYASSERT(0u);
    
    /* skaitom is EEPROM paskutinio veikiancio sleivo adresa */
    EE_GetByte( EE_SLAVE_ADDR, &prv->Data.addr );
    
    
/* Jieskome sleiva. Galimi variantai:
    1. Neteisingas sleivo adresas. Jieskame veikianti sleiva ir, jai toks yra, isimenam EEPROMe jo adresa.
    2. Sleivo adresas teisingas, bet jis neatsako. Galimi variantai:
        2.1 Nera veikianciu sleivu.
        2.2 Sleivo adresas skiriasi nuo uzregistroto paskutinio veikiancio.  Surandame toki ir isimenam EEPROMe
            jo adresa.
        2.3 Sleivo adresas sutampa su uzregistruotu paskutiniu veikianciu, bet del kazkokiu tai priezasciu
            iskarto neatsake. Surandame toki ir nieko nedarom.   
    
    */    
    
    
    if( prv->Data.addr == 0 || prv->Data.addr > MB_ADDRESS_MAX ) {
        /* adresas neteisingas, reikia jieskoti veikiancio slaivo */
        
        /* jieskome pirma veikianti devaisa */
        FindSlaveDevice();
        
        /* saugojam EEPROM'e rasto slaivo adresa */
        EE_PutByte( EE_SLAVE_ADDR, &prv->Data.addr );
    }else{
        /* adresas teisingas */
    
        eMBMasterReqReportSlaveId( prv->Data.addr );
        
        do{
            eMBMasterPoll();
            CyDelayUs(100);
        }while( xMBMasterGetIsBusy() == true );
        
        
        if( prv->Status.IsRecognized == false ){
            /* jai esamu adresu sleivas neatsake, bandom dar arba jieskome kita sleiva */
            
            uint8_t tmp = prv->Data.addr;
            
            /* jieskome pirma veikianti devaisa */
            FindSlaveDevice();
            
            /* saugojam EEPROM'e rasto slaivo adresa, jai jo adresas skiriasi nuo issaugoto */
            if( tmp != prv->Data.addr ) EE_PutByte( EE_SLAVE_ADDR, &prv->Data.addr );
        }        
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

/* UART rezimo valdymas */
static CYBLE_API_RESULT_T SCB_ModeHandler(){
    
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

/* automatine veikiancio slaivo pajeska */
static void FindSlaveDevice(){

    uint8_t i = 120;
    uint16_t leds = TLEDS_OFF, bit = LEDT1;    
    uint32_t led_timer = GetTicks() + 100;
    bool show = false;
    
    while( true ){        
        
        eMBMasterReqReportSlaveId( i );
        
        do{
            eMBMasterPoll();          

            if( led_timer < GetTicks() ){
                
                led_timer = GetTicks() + 100;
         
                switch( i / 16 ){
                    case 0:
                    leds = TLEDS_OFF;
                    bit = LEDT1;
                    break;
                    case 1:
                    leds |= LEDT1;
                    bit = LEDT2;
                    break;
                    case 2:
                    leds |= LEDT2;
                    bit = LEDT3;
                    break;
                    case 3:
                    leds |= LEDT3;
                    bit = LEDT4;
                    break;
                    case 4:
                    leds |= LEDT4;
                    bit = LEDT5;
                    break;
                    case 5:
                    leds |= LEDT5;
                    bit = LEDT6;
                    break;
                    case 6:
                    leds |= LEDT6;
                    bit = LEDT7;
                    break;
                    case 7:
                    leds |= LEDT7;
                    bit = LEDT8;
                    break;
                    case 8:
                    leds |= LEDT8;
                    bit = LEDT9;
                    break;
                    case 9:
                    leds |= LEDT9;
                    bit = LEDT10;
                    break;
                    case 10:
                    leds |= LEDT10;
                    bit = LEDT11;
                    break;
                    case 11:
                    leds |= LEDT11;
                    bit = LEDT12;
                    break;
                    case 12:
                    leds |= LEDT12;
                    bit = LEDT13;
                    break;
                    case 13:
                    leds |= LEDT13;
                    bit = LEDT14;
                    break;
                    case 14:
                    leds |= LEDT14;
                    bit = LEDT15;
                    break;
                    case 15:
                    leds |= LEDT15;
                    bit = LEDT16;
                    break;
                    default:

                    break;
                }
                
                show = !show;
                
                if( show )leds |= bit;
                else leds &= ~bit;
                
                SpiWriteWord( leds );
            }
            
            CyDelayUs(100);
        }while( xMBMasterGetIsBusy() == true );
    
        if( prv->Status.IsRecognized == true ){
            prv->Data.addr = i;
            break;
        }

        if( i++ > MB_ADDRESS_MAX ) i = 1;
    }
}

/* hardvaro inicializacija */
static CYBLE_API_RESULT_T HWInit(){
    CYBLE_API_RESULT_T apiResult = CYBLE_ERROR_OK;
    
    /* nested interaptu nustatymas */
    CyIntSetVector( NESTED_ISR, NestedIsrHandler );
    CyIntSetPriority( NESTED_ISR, NES_DEF_PRIORITY );
    CyIntEnable( NESTED_ISR );
    
    SysTickInit();
    WdtInit();
    
    WDT0_DISABLE
    
    CapSenseInit();
    
    AdcInit();

    SoundInit( true, CyTouchControl.Sound.Level );
    BacklightInit( true, CyTouchControl.Blank.Timeout.val );
    BlStartTimeout();
    
    InitLEDS();
    
    SetSoundTone(3u);
    
    return apiResult;
}

/* darbiniu reiksmiu uzkrovimas */
static CYBLE_API_RESULT_T CyTouchSettings(){
    uint8_t var = 0xFF;
    
    //EE_PutByte( EE_DATA_OK_FLAG_ADDR, &var);
    //EE_PutByte( EE_SLAVE_ADDR, &var );
    
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

/* pradiniu reiksmiu atstatymas */
static void RestoreDefaults(){
    
    uint8_t var;
    
    EE_GetDWord( EE_WTIME_ADDR, &Time.WTime );
    
    EE_EraseChip();
    
    EE_PutDWord( EE_WTIME_ADDR, &Time.WTime );
    
    MbPort.Settings.Baudrate.val = (19200u);
    MbPort.Settings.Parity.val = SCB_UART_PARITY_NONE;
    MbPort.Settings.StopBits.val = SCB_UART_STOP_BITS_1;
    MbPort.Settings.DataBits.val = (8u);
    EE_PutArray( MbPort.Settings.data, EE_MB_DATA_ADDR, sizeof( MbPort.Settings.data )); 
    
    var = 1u;
    EE_PutByte( EE_SLAVE_ADDR, &var );
   
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
    
    var = 0xAA;    
    EE_PutByte( EE_DATA_OK_FLAG_ADDR, &var );   
}


/* Tikrinam ar baitas yra skaicius */
uint8_t IsDigit(uint8_t byte)
{
    if( byte >= 0x30 && byte <= 0x39 ) return true;
    return false;
}


CY_ISR(NestedIsrHandler)
{
    /* Clear pending Interrupt */
    CyIntClearPending( NESTED_ISR );
}

/* [] END OF FILE */
