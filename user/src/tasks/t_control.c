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
#include "common.h"
#include "capsen.h"
#include "stypes.h"
#include "mpwm.h"
#include "systick.h"
#include "mbunit.h"
#include "leds.h"

#define CS_ISR_NUMBER       ( 7u )
#define CS_DEF_PRIORITY     ( 3u )
#define CS_HI_PRIORITY      ( 2u )

#define USE_PRESS_REPEAT    ( ENABLED )
#define USE_PRESS_TIMEOUT   ( ENABLED )
#define USE_PRESS_RELEASE   ( ENABLED )
#define USE_DOUBLE_CLICK    ( ENABLED )


typedef enum { FREE = 0, PRESSED = 1 } btn_state_t;


//#define SLIDER_ENABLED
#define BUTTONS_ENABLED
#define BTN_DEBOUNCE_VAL    ( 5u )

#define TIMEOUT_DLY         ( 2000u )
#define REPEAT_DLY          ( 1200u )
#define REPEAT_PERIOD       ( 80u )
#define DCLICK_PERIOD       ( 280u )

typedef struct{
    struct{
        btn_state_t State   :1;
        btn_state_t lState  :1;
        bool RepeatEna      :1;
        bool IsTimeout      :1;       
    };

    uint32_t TimeoutDelay;
    uint32_t RepeatPeriod;
    uint32_t DClickPeriod;
    void(*pPressHandler)(void);
    void(*pReleaseHandler)(void);
    void(*pRepeatHandler)(void);
    void(*pTimeoutHandler)(void);
    void(*pDClickHandler)(void);
}btn_t;

static struct _csdata{
    struct{
        btn_t     OnOff;
        btn_t     Speed1;
        btn_t     Speed2;
        btn_t     Speed3;
        btn_t     Boost;
        btn_t     TUp;
        btn_t     TDown;
        btn_t     Func; 
    }Button;    
    struct{
        bool Enable     :1;
        bool Start      :1;
        bool IsTimeout  :1;
    }Repeat;  
    struct{
        bool IsActive   :1;
        bool IsTimeout  :1;
    };
}CSData;

/* ******************************* Globals ********************************* */
bool TSetUpdateRequireBit = false;
bool FanSpeedUpdateRequireBit = false;
bool BoostUpdateRequireBit = false;
bool PrvResetRequireBit = false;
//bool CyTouchControl.Status.Locked = false;
bool ErrorBit = false;

/* ******************************* Externs ********************************* */
extern CYTOUCH_CONTROL_T CyTouchControl;
extern cydata_t DevData;
extern mbdev_t *prv;
extern uint16_t ShowWhatTimer;

static bool IsrNeed = false; 
static uint32_t LockTimer = LOCK_TIMER_DEF;


/* ************************ Funkciju prototipai **************************** */
static void ButtonInit( btn_t *btn );
static void ButtonEvents( btn_t *btn );

//static void LockModeHandler( void );

static void OnOff_PressHandler( void );
static void OnOff_ReleaseHandler( void );
static void OnOff_RepeatHandler( void );
static void OnOff_TimeoutHandler( void );
static void OnOff_DClickHandler( void );

static void TUp_PressHandler( void );
static void TUp_ReleaseHandler( void );
static void TUp_RepeatHandler( void );
static void TUp_TimeoutHandler( void );
static void TUp_DClickHandler( void );

static void TDown_PressHandler( void );
static void TDown_ReleaseHandler( void );
static void TDown_RepeatHandler( void );
static void TDown_TimeoutHandler( void );
static void TDown_DClickHandler( void );

static void Speed1_PressHandler( void );
static void Speed1_ReleaseHandler( void );
static void Speed1_RepeatHandler( void );
static void Speed1_TimeoutHandler( void );
static void Speed1_DClickHandler( void );

static void Speed2_PressHandler( void );
static void Speed2_ReleaseHandler( void );
static void Speed2_RepeatHandler( void );
static void Speed2_TimeoutHandler( void );
static void Speed2_DClickHandler( void );

static void Speed3_PressHandler( void );
static void Speed3_ReleaseHandler( void );
static void Speed3_RepeatHandler( void );
static void Speed3_TimeoutHandler( void );
static void Speed3_DClickHandler( void );

static void Boost_PressHandler( void );
static void Boost_ReleaseHandler( void );
static void Boost_RepeatHandler( void );
static void Boost_TimeoutHandler( void );
static void Boost_DClickHandler( void );

static void Func_PressHandler( void );
static void Func_ReleaseHandler( void );
static void Func_RepeatHandler( void );
static void Func_TimeoutHandler( void );
static void Func_DClickHandler( void );


CY_ISR_PROTO( CapSenseIsrHandler );


/* ************************ Funkciju implementacija ************************ */
void T_Control(){ // T = 50ms
    
    /* Automatinis valdymo blokavimas */
    if( LockTimer < GetTicks() && 
        CyTouchControl.Status.Locked == false && 
        CyTouchControl.Status.Autolock == true &&
        DevData.Alarms.val == 0u )
    {
        ResetLockTimer();
        
        SetSoundTone(3u);
        CyTouchControl.Status.Locked = true;
    }   
}


void CapSenseInit(){
    btn_t *btn;    
   
    CapSense_Start();
    CapSense_InitializeAllBaselines();    

    CapSense_SetDebounce(CapSense_SENSOR_TDOWN__BTN, BTN_DEBOUNCE_VAL );
    CapSense_SetDebounce(CapSense_SENSOR_TUP__BTN, BTN_DEBOUNCE_VAL );
    CapSense_SetDebounce(CapSense_SENSOR_SP1__BTN, BTN_DEBOUNCE_VAL );
    CapSense_SetDebounce(CapSense_SENSOR_SP2__BTN, BTN_DEBOUNCE_VAL );
    CapSense_SetDebounce(CapSense_SENSOR_SP3__BTN, BTN_DEBOUNCE_VAL );
    CapSense_SetDebounce(CapSense_SENSOR_BOOST__BTN, BTN_DEBOUNCE_VAL );    
    
    CSData.Repeat.Enable = true;
    CSData.Repeat.IsTimeout = false;
    CSData.IsTimeout = false;
    
    btn = &CSData.Button.OnOff;
    ButtonInit(btn);
    btn->pPressHandler = OnOff_PressHandler;
    btn->pReleaseHandler = OnOff_ReleaseHandler;
    btn->pRepeatHandler = OnOff_RepeatHandler;
    btn->pTimeoutHandler = OnOff_TimeoutHandler; 
    btn->pDClickHandler = OnOff_DClickHandler;
    
    btn = &CSData.Button.TUp;
    ButtonInit(btn);
    btn->pPressHandler = TUp_PressHandler;
    btn->pReleaseHandler = TUp_ReleaseHandler;
    btn->pRepeatHandler = TUp_RepeatHandler;
    btn->pTimeoutHandler = TUp_TimeoutHandler;
    btn->pDClickHandler = TUp_DClickHandler;
    
    btn = &CSData.Button.TDown;
    ButtonInit(btn);
    btn->pPressHandler = TDown_PressHandler;
    btn->pReleaseHandler = TDown_ReleaseHandler;
    btn->pRepeatHandler = TDown_RepeatHandler;
    btn->pTimeoutHandler = TDown_TimeoutHandler;
    btn->pDClickHandler = TDown_DClickHandler;
    
    btn = &CSData.Button.Speed1;
    ButtonInit(btn);
    btn->pPressHandler = Speed1_PressHandler;
    btn->pReleaseHandler = Speed1_ReleaseHandler;
    btn->pRepeatHandler = Speed1_RepeatHandler;
    btn->pTimeoutHandler = Speed1_TimeoutHandler;
    btn->pDClickHandler = Speed1_DClickHandler;
    
    btn = &CSData.Button.Speed2;
    ButtonInit(btn);
    btn->pPressHandler = Speed2_PressHandler;
    btn->pReleaseHandler = Speed2_ReleaseHandler;
    btn->pRepeatHandler = Speed2_RepeatHandler;
    btn->pTimeoutHandler = Speed2_TimeoutHandler;
    btn->pDClickHandler = Speed2_DClickHandler;
    
    btn = &CSData.Button.Speed3;
    ButtonInit(btn);
    btn->pPressHandler = Speed3_PressHandler;
    btn->pReleaseHandler = Speed3_ReleaseHandler;
    btn->pRepeatHandler = Speed3_RepeatHandler;
    btn->pTimeoutHandler = Speed3_TimeoutHandler;
    btn->pDClickHandler = Speed3_DClickHandler;

    btn = &CSData.Button.Boost;
    ButtonInit(btn);
    btn->pPressHandler = Boost_PressHandler;
    btn->pReleaseHandler = Boost_ReleaseHandler;
    btn->pRepeatHandler = Boost_RepeatHandler;
    btn->pTimeoutHandler = Boost_TimeoutHandler;
    btn->pDClickHandler = Boost_DClickHandler;

    btn = &CSData.Button.Func;
    ButtonInit(btn);
    btn->pPressHandler = Func_PressHandler;
    btn->pReleaseHandler = Func_ReleaseHandler;
    btn->pRepeatHandler = Func_RepeatHandler;
    btn->pTimeoutHandler = Func_TimeoutHandler;   
    btn->pDClickHandler = Func_DClickHandler;
    
    /* nested interaptu nustatymas ( bandymas ) */
    CyIntSetVector(CS_ISR_NUMBER, CapSenseIsrHandler);
    CyIntSetPriority(CS_ISR_NUMBER, CS_DEF_PRIORITY);
    CyIntEnable(CS_ISR_NUMBER);
}
void CapSenseProcess(){ 
    btn_t *btn;
   
    if( CapSense_IsBusy() ) return;
    
#ifdef SLIDER_ENABLED
    
    static uint16 curPos, oldPos;
    
    //curPos = CapSense_GetCentroidPos(CapSense_LINEARSLIDER__LS);    
    
    if(curPos != 0xFFFFu && curPos != 0u)
    {
        if (curPos != oldPos) oldPos = curPos;  
        CapSenseData.SliderPos = curPos;
    }
	    
    //CapSenseData.IsActive = CapSense_CheckIsWidgetActive(CapSense_LINEARSLIDER__LS); 
    
#endif

#ifdef BUTTONS_ENABLED
    
    btn = &CSData.Button.OnOff;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_PWRON__BTN));    
    ButtonEvents( btn );
    
    btn = &CSData.Button.Speed1;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_SP1__BTN));    
    ButtonEvents( btn );
    
    btn = &CSData.Button.Speed2;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_SP2__BTN));    
    ButtonEvents( btn );
    
    btn = &CSData.Button.Speed3;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_SP3__BTN));    
    ButtonEvents( btn );
    
    btn = &CSData.Button.Boost;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_BOOST__BTN));    
    ButtonEvents( btn );    
    
    btn = &CSData.Button.TUp;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_TUP__BTN));    
    ButtonEvents( btn );
    
    btn = &CSData.Button.TDown;
    btn->State = (CapSense_CheckIsWidgetActive(CapSense_SENSOR_TDOWN__BTN));    
    ButtonEvents( btn );
   
    /* isr bandymas */        
    if( IsrNeed == true ){
        IsrNeed = false;
        CyIntSetPending( CS_ISR_NUMBER );
    }
    /* isr bandymas */
    
#endif

    /* isvalom sensoriu busena */
    CapSense_ClearSensors();

    CapSense_UpdateEnabledBaselines();
	CapSense_ScanEnabledWidgets();
}

   
/* Static functions */
static void ButtonInit( btn_t *btn ){
    btn->State = FREE;
    btn->lState = FREE;
    btn->TimeoutDelay = (0u);
    btn->IsTimeout = false;
    btn->RepeatEna = false;
    btn->pPressHandler = NULL;
    btn->pRepeatHandler = NULL;
    btn->pTimeoutHandler = NULL;
    btn->pReleaseHandler = NULL;
    btn->pDClickHandler = NULL;
}
static void ButtonEvents( btn_t *btn ){
    
    uint32_t time = GetTicks();
  
    if( btn->State != btn->lState ){
        btn->lState = btn->State; 
        
        BlStartTimeout();
        ResetLockTimer();
        
        if( btn->State == PRESSED ){
            
            CSData.IsActive = true;
            btn->TimeoutDelay = time + TIMEOUT_DLY;
            /*if( btn->pPressHandler != NULL )*/ btn->pPressHandler(); 
            
#if(USE_DOUBLE_CLICK == ENABLED)
            if( btn->DClickPeriod == 0u ){
                btn->DClickPeriod = time + DCLICK_PERIOD;               
            }            
            else{                                
                if( btn->DClickPeriod > time ){
                    btn->DClickPeriod = (0u);
                    btn->pDClickHandler();
                }
            }
#endif

        }
        else{
            CSData.IsActive = false;
            btn->TimeoutDelay = (0u);
            
#if(USE_PRESS_RELEASE == ENABLED)
            btn->pReleaseHandler(); 
#endif

        }
        
        IsrNeed = true;
    }
    else{
        
#if(USE_DOUBLE_CLICK == ENABLED)
        if( btn->DClickPeriod < time ) btn->DClickPeriod = (0u);
#endif     
#if(USE_PRESS_REPEAT == ENABLED)
        if( btn->TimeoutDelay > 0u ){
            if( btn->TimeoutDelay - REPEAT_DLY < time ){
                if( btn->RepeatPeriod < time ){
                    btn->RepeatPeriod = time + REPEAT_PERIOD;
                    btn->pRepeatHandler();
                }                
            }
#endif
#if(USE_PRESS_TIMEOUT == ENABLED)
            if( btn->TimeoutDelay < time ){
                btn->TimeoutDelay = (0u);
                btn->pTimeoutHandler();
            }
#endif
#if(USE_PRESS_REPEAT == ENABLED)
        }   
#endif

    }
}


/* On/Off mygtuko handleriai */
static void OnOff_PressHandler(){
    SetSoundTone( 1u );    
    if( CyTouchControl.Status.Locked == true ) return;
}
static void OnOff_ReleaseHandler(){
    if( CyTouchControl.Status.Locked == true ) return;
}
static void OnOff_RepeatHandler(){
    if( CyTouchControl.Status.Locked == true ) return;
}
static void OnOff_TimeoutHandler(){
    static uint8_t lSpeed = 0u;    
    
    /* resetinam prv kai alarmai */
    if( DevData.Alarms.val > 0u ){
        SetSoundTone(3u);
        ShowWhatTimer = (1000u);
        PrvResetRequireBit = true;
        DevData.Alarms.val = (0u);
        ClearSlaveData( prv );
        return;
    }
    
    if( CyTouchControl.Status.Locked == true ) return;
    
    SetSoundTone(3u);
    
    /* resetinam prv darbiniame rezime */
    if( CSData.Button.Boost.State == true ){
        PrvResetRequireBit = true;
        ClearSlaveData( prv );
        return;
    }
    
    /* darbo rezimas: stndby/run */
    if( DevData.Speed.val > 0u ){
        lSpeed = DevData.Speed.val;
        DevData.Speed.val = (0u);
        DevData.Boost.val = false;
        FanSpeedUpdateRequireBit = true;
        BoostUpdateRequireBit = true;
    }else{
        DevData.Speed.val = lSpeed;
        FanSpeedUpdateRequireBit = true;
    }
}
static void OnOff_DClickHandler(){
    
    /* atblokuojam valdyma */    
    if( CyTouchControl.Status.Locked == true ) {
        CyTouchControl.Status.Locked = false;
        ShowWhatTimer = (1000u);
    }else{
        CyTouchControl.Status.Locked = true;
        ShowWhatTimer = (10u);
    }    
}

/* TUp mygtuko handleriai */
static void TUp_PressHandler(){
    if( DevData.Alarms.val > 0u ) return;    
    if( CyTouchControl.Status.Locked == true ){
        ErrorBit = true;
        return;
    }
    
    ShowWhatTimer = ( 1000u );
    
    if( DevData.TSet.val >= ( int8_t )(CyTouchControl.TSet.MinValue.val + ( 15u ) ) )
    {
        DevData.TSet.val = CyTouchControl.TSet.MinValue.val + ( 15u );
        SetSoundTone( 3u );
        return;
    }
    
    SetSoundTone(1u);
    DevData.TSet.val++;
}
static void TUp_ReleaseHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    TSetUpdateRequireBit = true;
}
static void TUp_RepeatHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    if( DevData.TSet.val < ( int8_t )(CyTouchControl.TSet.MinValue.val + ( 15u ) ) ){
        TUp_PressHandler();
    }
}
static void TUp_TimeoutHandler(){
}
static void TUp_DClickHandler(){
}

/* TDown mygtuko handleriai */
static void TDown_PressHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ){
        ErrorBit = true;
        return;
    }
    
    ShowWhatTimer = ( 1000u );
    
    if( DevData.TSet.val <= CyTouchControl.TSet.MinValue.val ) 
    {
        DevData.TSet.val = CyTouchControl.TSet.MinValue.val;
        CSData.Repeat.Enable = false;
        SetSoundTone( 3u );
        return;
    }
    
    SetSoundTone(1u);
    DevData.TSet.val--;
}
static void TDown_ReleaseHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    TSetUpdateRequireBit = true;
}
static void TDown_RepeatHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    if( DevData.TSet.val > CyTouchControl.TSet.MinValue.val ){
        TDown_PressHandler();
    }
}
static void TDown_TimeoutHandler(){
}
static void TDown_DClickHandler(){
}

/* Speed1 mygtuko handleriai */
static void Speed1_PressHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true || DevData.Boost.val == true ){
        ErrorBit = true;
        return;
    }
    
    SetSoundTone(1u);
    DevData.Speed.val = ( 1u );
    DevData.Boost.val = false;
}
static void Speed1_ReleaseHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    FanSpeedUpdateRequireBit = true;
    BoostUpdateRequireBit = true;
}
static void Speed1_RepeatHandler(){
}
static void Speed1_TimeoutHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    SetSoundTone(3u);
    DevData.Speed.val = ( 0u );
    DevData.Boost.val = false;
}
static void Speed1_DClickHandler(){
}

/* Speed2 mygtuko handleriai */
static void Speed2_PressHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true || DevData.Boost.val == true ){
        ErrorBit = true;
        return;
    }
    
    SetSoundTone(1u);
    DevData.Speed.val = ( 2u );
    DevData.Boost.val = false;
}
static void Speed2_ReleaseHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    FanSpeedUpdateRequireBit = true;
    BoostUpdateRequireBit = true; 
}
static void Speed2_RepeatHandler(){
}
static void Speed2_TimeoutHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    SetSoundTone(3u);
    DevData.Speed.val = ( 0u );
    DevData.Boost.val = false;
}
static void Speed2_DClickHandler(){
}

/* Speed3 mygtuko handleriai */
static void Speed3_PressHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true || DevData.Boost.val == true ){
        ErrorBit = true;
        return;
    }

    SetSoundTone(1u);
    DevData.Speed.val = ( 3u );
    DevData.Boost.val = false;
}
static void Speed3_ReleaseHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    FanSpeedUpdateRequireBit = true;
    BoostUpdateRequireBit = true;
}
static void Speed3_RepeatHandler(){
}
static void Speed3_TimeoutHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    SetSoundTone(3u);
    DevData.Speed.val = ( 0u );
    DevData.Boost.val = false;
}
static void Speed3_DClickHandler(){
}

/* Boost mygtuko handleriai */
static void Boost_PressHandler(){
    if( DevData.Alarms.val > 0u ) return;
    
    if( CyTouchControl.Status.Locked == true ){
        ErrorBit = true;
        return;
    }    
    
    if( CSData.Button.OnOff.State == PRESSED ){
        SetSoundTone( 1u );
        return;
    }
    
    if( prv->Status.Type == VENTIKE || prv->Status.Type == VENTIKW ){
        ErrorBit = true;
        SetSoundTone( 3u );
        return;
    }
    
    SetSoundTone( 1u );
    
    DevData.Boost.val = !DevData.Boost.val;
}
static void Boost_ReleaseHandler(){
    if( DevData.Alarms.val > 0u ) return;
    if( CyTouchControl.Status.Locked == true ) return;
    
    BoostUpdateRequireBit = true;
}
static void Boost_RepeatHandler(){
}
static void Boost_TimeoutHandler(){
}
static void Boost_DClickHandler(){
}

/* Func mygtuko handleriai */
static void Func_PressHandler(){
}
static void Func_ReleaseHandler(){
}
static void Func_RepeatHandler(){
}
static void Func_TimeoutHandler(){
}
static void Func_DClickHandler(){
}


bool CapSenseIsActive(){
    return CSData.IsActive;
}
void ResetLockTimer(){
    LockTimer = GetTicks() + CyTouchControl.LockTimer.val;   
}

CY_ISR( CapSenseIsrHandler )
{
    /* Clear pending Interrupt */
    CyIntClearPending( CS_ISR_NUMBER );
    
//    BLE_LED_Write( !BLE_LED_Read() );
}


/* [] END OF FILE */
