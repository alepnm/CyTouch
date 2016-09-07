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
#include "leds.h"
#include "common.h"
#include "stypes.h"
#include "systick.h"
#include "mspi.h"
#include "mbunit.h"
#include "rtos.h"
#include "mbutils.h"
#include "buffers.h"


 
#define BLE_LED_TO_ADVESTING    ( 3000u )
#define BLE_LED_TO_CONNECT      ( 100u )
#define MIN_TEMPERATURE         ( 15u )


extern CYTOUCH_CONTROL_T CyTouchControl;
extern cydata_t DevData;
extern mbdev_t *prv;

extern bool PrvDataPrepared;
extern bool MbStackFree;
extern bool ErrorBit;

uint16_t ShowWhatTimer = (1000u);

struct _leds LEDS;



/* ******************************* Externs ********************************* */
static void BleLed_Process(void);
static void OnOffLed_Process(void);
static void TLeds_Process(void);
static void SpeedLeds_Process(void);


/* Grazina true, kai visi funkcijos zingsniai atlikti */
static bool ShowNoLink(void);
static bool BlinkInError(void);


// periodas 20 ms
void T_Leds(){   
    static uint8_t stage = (0u);
    static uint8_t cnt = (0u);
   
  
    
    switch (cyBle_state)
    {
        case CYBLE_STATE_ADVERTISING:
            if( LEDS.BleLed.State == true ) LEDS.BleLed.counter_val = BLE_LED_TO_ADVESTING;
            else LEDS.BleLed.counter_val = (10u); 
            break;            
        case CYBLE_STATE_CONNECTED:             
            //LEDS.BleLed.counter_val = BLE_LED_TO_CONNECT;
            
            BLE_LED_Write( LED_ON );
            break;            
        default:
            break;
    }       
   
    OnOffLed_Process();
    BleLed_Process();    
    
    
    if( ErrorBit ){
        if( BlinkInError() == true ) ErrorBit = false;
        return;
    }    

    
    // jai slaivas gyvas
    if( ( prv->Status.IsAlive == false && prv->Status.IsRecognized == false ) || PrvDataPrepared == false){
        ShowNoLink();
        return;
    } 
    
    
    if(DevData.Alarms.val > 0u){
        switch( stage ){
            case 0:
                BlStartTimeout();
                
//                LEDS.Sp1Led.State = LED_OFF;
//                LEDS.Sp2Led.State = LED_OFF;
//                LEDS.Sp3Led.State = LED_OFF;
//                LEDS.BoostLed.State = LED_OFF;
                
                
                Sp1_LED_Write( LED_OFF );
                Sp2_LED_Write( LED_OFF );
                Sp3_LED_Write( LED_OFF );
                Boost_LED_Write( LED_OFF );
                
                SpiWriteWord( DevData.Alarms.val );
                stage++;
            break;
            case 1:
            if( cnt > 2 ){
                stage++;
                SpiWriteWord( TLEDS_OFF );
                cnt = 0;
            }
            break;
            case 2:
            if( cnt > 2 ){
                stage++;
                SpiWriteWord( DevData.Alarms.val );
                cnt = 0;
            }
            break;
            case 3:
            if( cnt > 2 ){
                stage++;
                SpiWriteWord( TLEDS_OFF );
                cnt = 0;
            }
            break;
            case 4:
            if( cnt > 2 ){
                stage++;
                SpiWriteWord( DevData.Alarms.val );
                cnt = 0;
            }
            break;
            case 5:
            if( cnt > 2 ){
                stage++;
                SpiWriteWord( TLEDS_OFF );
                cnt = 0;
            }
            break;
            case 6:
                if( cnt < 50 ) break;
                cnt = 0u;
                stage = 0;
                break;            
        }
        cnt++;
        return;
    }    
    
    Filter_LED_Write(DevData.Alarms.Filter);

    
//    // jai valdymas uzblokuitas
//    if( CyTouchControl.Status.Locked == true ){
//        (void)RunLeds_1();
//        return;
//    }
    
    TLeds_Process();
    SpeedLeds_Process();
}


void InitLEDS(){
    
//    LEDS.OnOffLed.State = LED_OFF;
//    LEDS.TLED.Value = TLEDS_OFF;
//    LEDS.Sp1Led.State = LED_OFF;
//    LEDS.Sp2Led.State = LED_OFF;
//    LEDS.Sp3Led.State = LED_OFF;
//    LEDS.BoostLed.State = LED_OFF;
//    LEDS.BleLed.State = LED_OFF;
//    LEDS.FilterLed.State = LED_OFF;
//    LEDS.ServiceLed.State = LED_OFF;
    
    LEDS.BleLed.counter_val = BLE_LED_TO_ADVESTING;
    
    
    OnOff_LED_Write( LED_OFF );    
    Sp1_LED_Write( LED_OFF );
    Sp2_LED_Write( LED_OFF );
    Sp3_LED_Write( LED_OFF );
    Filter_LED_Write( LED_OFF );
    Service_LED_Write( LED_OFF );
    Boost_LED_Write( LED_OFF );
}
static void BleLed_Process(){    
    /* BLE LED process */
    if( cyBle_state == CYBLE_STATE_CONNECTED ) return;
    
    if(LEDS.BleLed.Counter < GetTicks()){
        LEDS.BleLed.Counter = GetTicks() + LEDS.BleLed.counter_val;
        LEDS.BleLed.State = !LEDS.BleLed.State;
        
        BLE_LED_Write( LEDS.BleLed.State ); 
    }    
}
static void OnOffLed_Process(){
    // Link LED handle
    if( ShowWhatTimer == 0 ){
        OnOff_LED_Write( !MbStackFree );
    }else{
        OnOff_LED_Write( LED_ON );
    }    
}
static void TLeds_Process(){
    
    static int8_t tmp = (0u);    
    uint16_t led_bar = 0x0000;
    uint16_t led_point = 0x0000;
    static uint32_t blink_timer = (0u);
    uint8_t outval = (0u);
    
    
//    ShowWhatTimer = 1;//blokuoju taimeri
    
    if( ShowWhatTimer > (0u) ){
        if( tmp != DevData.TSet.val ){
            if( tmp < DevData.TSet.val ) tmp++;            
            if( tmp > DevData.TSet.val ) tmp--;     
        }
        
        outval = tmp - CyTouchControl.TSet.MinValue.val;        
    }
    else{
        
        if( blink_timer < GetTicks() ){
            blink_timer = GetTicks() + (500u);
        
            if( DevData.Sensors.sn0 < CyTouchControl.TSet.MinValue.val ){
                // mirkciojam min sviesos diodu
                    
                if( led_point == LEDT1 ) led_point = TLEDS_OFF;
                else led_point = LEDT1;
                
                SpiWriteWord(led_point);
                
                return;
            }
            
            if( DevData.Sensors.sn0 > (int8_t)( CyTouchControl.TSet.MinValue.val + (15u) ) ){
                // mirkciojam max sviesos diodu
                    
                if( led_point == LEDT16 ) led_point = TLEDS_OFF;
                else led_point = LEDT16;
                
                SpiWriteWord(led_point);
                
                return;
            }
        }             
        
        if( tmp != DevData.Sensors.sn0 ){
            if( tmp < DevData.Sensors.sn0 ) tmp++;            
            if( tmp > DevData.Sensors.sn0 ) tmp--;     
        }        
        
        outval = DevData.Sensors.sn0 - CyTouchControl.TSet.MinValue.val;        
    }    
    
    switch( outval ){
        case 0:
        led_point = LEDT1;
        led_bar = LEDT1;
        break;
        case 1:
        led_point = LEDT2;
        led_bar = LEDT1 | LEDT2;
        break;
        case 2:
        led_point = LEDT3;
        led_bar = LEDT1 | LEDT2 | LEDT3;
        break;
        case 3:
        led_point = LEDT4;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4;
        break;
        case 4:
        led_point = LEDT5;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5;
        break;
        case 5:
        led_point = LEDT6;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6;
        break;
        case 6:
        led_point = LEDT7;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7;
        break;
        case 7:
        led_point = LEDT8;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        break;
        case 8:
        led_point = LEDT9;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9;
        break;
        case 9:
        led_point = LEDT10;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10;
        break;
        case 10:
        led_point = LEDT11;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10 | LEDT11;
        break;
        case 11:
        led_point = LEDT12;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10 | LEDT11 | LEDT12;
        break;
        case 12:
        led_point = LEDT13;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10 | LEDT11 | LEDT12 | LEDT13;
        break;
        case 13:
        led_point = LEDT14;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10 | LEDT11 | LEDT12 | LEDT13 | LEDT14;
        break;
        case 14:
        led_point = LEDT15;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10 | LEDT11 | LEDT12 | LEDT13 | LEDT14 | LEDT15;
        break;
        case 15:
        led_point = LEDT16;
        led_bar = LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8;
        led_bar |= LEDT9 | LEDT10 | LEDT11 | LEDT12 | LEDT13 | LEDT14 | LEDT15 | LEDT16;
        break;
    }   
    
    if( ShowWhatTimer > (0u) ){
        ShowWhatTimer --;
        SpiWriteWord( led_bar );
    }
    else{
        SpiWriteWord( led_point );
    }
}  



static void SpeedLeds_Process(){
    
    static bool led = LED_OFF; 
    static uint16_t hist = 0;
    static uint32_t led_delay = 0;
    
    if( prv->Status.Res.SpeedLowActive == true ){   
        hist = 1000;
    }
    else{
        if( hist > 0 ) hist--;
        else led = LED_ON;
    }
    
    if( hist > 0 && led_delay < GetTicks() ){
        led_delay = GetTicks() + 1000;
        
        led = !led;
    }
    
    
    /* Speed LEDs */
    switch( DevData.Speed.val ){
    case 0:
        Sp1_LED_Write( LED_OFF );
        Sp2_LED_Write( LED_OFF );
        Sp3_LED_Write( LED_OFF );
        break;            
    case 1:
        Sp1_LED_Write( led );
        Sp2_LED_Write( LED_OFF );
        Sp3_LED_Write( LED_OFF );
        break;
    case 2:
        Sp1_LED_Write( LED_OFF );
        Sp2_LED_Write( led );
        Sp3_LED_Write( LED_OFF );
        break;
    case 3: 
        Sp1_LED_Write( LED_OFF );
        Sp2_LED_Write( LED_OFF );
        Sp3_LED_Write( led );
        break;
    }
    
    
    
    #warning PATAISYTI logika su BOOST!!!
    if( DevData.Boost.val == true && prv->Data.BoostCounter == 0 ){
        Boost_LED_Write( Time.t1s );
    }
    else{
        Boost_LED_Write( prv->Status.Res.BoostActive );
    } 
    
    
    
    
    
    
}
    

void CheckLeds(){
    OnOff_LED_Write( LED_ON );
    Sp1_LED_Write( LED_ON );
    Sp2_LED_Write( LED_ON );
    Sp3_LED_Write( LED_ON );
    Boost_LED_Write( LED_ON );
    BLE_LED_Write( LED_ON );
    Filter_LED_Write( LED_ON );
    Service_LED_Write( LED_ON );
    
//    RunLeds_1();
//    RunLeds_2();
    while( RunLeds_3() == false );   
    
    OnOff_LED_Write( LED_OFF ); 
    Sp1_LED_Write( LED_OFF );
    Sp2_LED_Write( LED_OFF );
    Sp3_LED_Write( LED_OFF );
    Boost_LED_Write( LED_OFF );
    BLE_LED_Write( LED_OFF );
    Filter_LED_Write( LED_OFF );
    Service_LED_Write( LED_OFF );
    
}
bool RunLeds_1(){
    uint16_t spi_out = TLEDS_OFF; 
    static uint8_t stage = 0u;
    static uint32_t delay = 0u;
    bool result = false;
    
    if( delay > GetTicks() ) return result;
    
    delay = GetTicks() + 50;
    
    switch( stage ){
        case 0:
            spi_out = ( LEDT8 | LEDT9 );
            break;
        case 1:
            spi_out = ( LEDT7 | LEDT10 );
            break;
        case 2:
            spi_out = ( LEDT6 | LEDT11 );
            break;
        case 3:
            spi_out = ( LEDT5 | LEDT12 );
            break;
        case 4:
            spi_out = ( LEDT4 | LEDT13 );
            break;
        case 5:
            spi_out = ( LEDT3 | LEDT14 );
            break;
        case 6:
            spi_out = ( LEDT2 | LEDT15 );
            break;
        case 7:
            spi_out = ( LEDT1 | LEDT16 );
            break;
        case 8:
            spi_out = ( LEDT2 | LEDT15 );
            break;
        case 9:
            spi_out = ( LEDT3 | LEDT14 );
            break;
        case 10:
            spi_out = ( LEDT4 | LEDT13 );
            break;
        case 11:
            spi_out = ( LEDT5 | LEDT12 );
            break;
        case 12:
            spi_out = ( LEDT6 | LEDT11 );
            break;
        case 13:
            spi_out = ( LEDT7 | LEDT10 );
            result = true;
            break;
    }
        
    if( ++stage > 13 ) stage = 0;
    
    SpiWriteWord(spi_out);
    
    return result;
}
bool RunLeds_2(){
    uint16_t spi_out = TLEDS_OFF; 
    static uint8_t stage = 0u;
    static uint32_t delay = 0u;
    bool result = false;
    
    if( delay > GetTicks() ) return result;
    
    delay = GetTicks() + 30;
    
    switch( stage ){
        case 0:
            spi_out = ( LEDT1 | LEDT16 );
            break;
        case 1:
            spi_out = ( LEDT1 | LEDT2 | LEDT15 | LEDT16 );
            break;
        case 2:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 3:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT13 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 4:
            spi_out = ( LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT12 | LEDT13 | LEDT14 | LEDT15 );
            break;
        case 5:
            spi_out = ( LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT11 | LEDT12 | LEDT13 | LEDT14 );
            break;
        case 6:
            spi_out = ( LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT10 | LEDT11 | LEDT12 | LEDT13 );
            break;
        case 7:
            spi_out = ( LEDT5 | LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT12 );
            break;
        case 8:
            spi_out = ( LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 );
            break;
        case 9:
            spi_out = ( LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT7 | LEDT8 | LEDT9 | LEDT10 );
            break;
        case 10:
            spi_out = ( LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT6 | LEDT7 | LEDT8 | LEDT9 );
            break;
        case 11:
            spi_out = ( LEDT9 | LEDT10 | LEDT11 | LEDT12 | LEDT5 | LEDT6 | LEDT7 | LEDT8 );
            break;
        case 12:
            spi_out = ( LEDT10 | LEDT11 | LEDT12 | LEDT13 | LEDT4 | LEDT5 | LEDT6 | LEDT7 );
            break;
        case 13:
            spi_out = ( LEDT11 | LEDT12 | LEDT13 | LEDT14 | LEDT3 | LEDT4 | LEDT5 | LEDT6 );
            break;
        case 14:
            spi_out = ( LEDT12 | LEDT13 | LEDT14 | LEDT15 | LEDT2 | LEDT3 | LEDT4 | LEDT5 );
            break;
        case 15:
            spi_out = ( LEDT13 | LEDT14 | LEDT15 | LEDT16 | LEDT1 | LEDT2 | LEDT3 | LEDT4 );
            break;
        case 16:
            spi_out = ( LEDT14 | LEDT15 | LEDT16 | LEDT1 | LEDT2 | LEDT3 );
            break;
        case 17:
            spi_out = ( LEDT15 | LEDT16 | LEDT1 | LEDT2 );
            break;
        case 18:
            spi_out = ( LEDT16 | LEDT1 );
            break;
        case 19:
            spi_out = TLEDS_OFF;
            result = true;
            break;
    }
        
    if( ++stage > 19 ) stage = 0;
    
    SpiWriteWord(spi_out);
    
    return result;
}
bool RunLeds_3(){
    uint16_t spi_out = TLEDS_OFF; 
    static uint8_t stage = 0u;
    static uint32_t delay = 0u;
    bool result = false;
    
    if( delay > GetTicks() ) return result;
    
    delay = GetTicks() + 50;    
    
    switch( stage ){
        case 0:
            spi_out = ( LEDT8 | LEDT9 );
            break;
        case 1:
            spi_out = ( LEDT7 | LEDT8 | LEDT9 | LEDT10 );
            break;
        case 2:
            spi_out = ( LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 );
            break;
        case 3:
            spi_out = ( LEDT5 | LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT12 );
            break;
        case 4:
            spi_out = ( LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT12 | LEDT13 );
            break;
        case 5:
            spi_out = ( LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT12 |LEDT13 | LEDT14 );
            break;
        case 6:
            spi_out = ( LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT12 |LEDT13 | LEDT14 | LEDT15 );
            break;
        case 7:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT8 | LEDT9 | LEDT10 | LEDT11 | LEDT12 |LEDT13 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 8:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT7 | LEDT10 | LEDT11 | LEDT12 |LEDT13 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 9:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT6 | LEDT11 | LEDT12 |LEDT13 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 10:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT5 | LEDT12 |LEDT13 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 11:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT4 | LEDT13 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 12:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 | LEDT14 | LEDT15 | LEDT16 );
            break;
        case 13:
            spi_out = ( LEDT1 | LEDT2 | LEDT15 | LEDT16 );
            break;
        case 14:
            spi_out = ( LEDT1 | LEDT16 );
            break;
        case 15:
            spi_out = TLEDS_OFF;
            result = true;
            break;      
    }
        
    if( ++stage > 15 ) stage = 0;
    
    SpiWriteWord(spi_out);
    
    return result;
}

static bool ShowNoLink(){    
    static uint8_t stage = (0u);
    uint16_t spi_out = TLEDS_OFF;
    static uint32_t delay = (0u);
    bool result = false;

    if( delay > GetTicks() ) return result;
    
    if( GetTicks() > (delay + 2000) ) stage = 0u;
    
    delay = GetTicks() + (50u);
    
    switch( stage ){
        case 0:
            Sp1_LED_Write( LED_OFF );
            Sp2_LED_Write( LED_OFF );
            Sp3_LED_Write( LED_OFF );
            Boost_LED_Write( LED_OFF );
            Filter_LED_Write( LED_OFF );
            Service_LED_Write( LED_OFF );
            SpiWriteWord( TLEDS_OFF );
            break;
        case 1:        
            spi_out = LEDT1;
            break;
        case 2:
            spi_out = ( LEDT1 | LEDT2 );
            break;
        case 3:
            spi_out = ( LEDT1 | LEDT2 | LEDT3 );
            break;
        case 4:
            spi_out = ( LEDT2 | LEDT3 | LEDT4 );
            break;
        case 5:
            spi_out = ( LEDT3 | LEDT4 | LEDT5 );
            break;
        case 6:
            spi_out = ( LEDT4 | LEDT5 | LEDT6 );
            break;
        case 7:
            spi_out = ( LEDT5 | LEDT6 | LEDT7 );
            break;
        case 8:
            spi_out = ( LEDT6 | LEDT7 | LEDT8 );
            break;
        case 9:
            spi_out = ( LEDT7 | LEDT8 | LEDT9 );
            break;
        case 10:
            spi_out = ( LEDT8 | LEDT9 | LEDT10 );
            break;
        case 11:
            spi_out = ( LEDT9 | LEDT10 | LEDT11 );
            break;
        case 12:
            spi_out = ( LEDT10 | LEDT11 | LEDT12 );
            break;
        case 13:
            spi_out = ( LEDT11 | LEDT12 | LEDT13 );
            break;
        case 14:
            spi_out = ( LEDT12 | LEDT13 | LEDT14 );
            break;
        case 15:
            spi_out = ( LEDT13 | LEDT14 | LEDT15 );
            break;
        case 16:
            spi_out = ( LEDT14 | LEDT15 | LEDT16 );
            break;
        case 17:
            spi_out = ( LEDT13 | LEDT14 | LEDT15 );
            break;
        case 18:
            spi_out = ( LEDT12 | LEDT13 | LEDT14 );
            break;
        case 19:
            spi_out = ( LEDT11 | LEDT12 | LEDT13 );
            break;
        case 20:
            spi_out = ( LEDT10 | LEDT11 | LEDT12 );
            break;
        case 21:
            spi_out = ( LEDT9 | LEDT10 | LEDT11 );
            break;
        case 22:
            spi_out = ( LEDT8 | LEDT9 | LEDT10 );
            break;
        case 23:
            spi_out = ( LEDT7 | LEDT8 | LEDT9 );
            break;
        case 24:
            spi_out = ( LEDT6 | LEDT7 | LEDT8 );
            break;
        case 25:
            spi_out = ( LEDT5 | LEDT6 | LEDT7 );
            break;
        case 26:
            spi_out = ( LEDT4 | LEDT5 | LEDT6 );
            break;
        case 27:
            spi_out = ( LEDT3 | LEDT4 | LEDT5 );
            break;
        case 28:
            spi_out = ( LEDT2 | LEDT3 | LEDT4 );
            result = true;
            break;            
    }
        
    if( ++stage > (28u) ) stage = (3u);
        
    SpiWriteWord(spi_out);
    
    return result;
}
static bool BlinkInError(){    
    uint16_t spi_out = TLEDS_OFF; 
    static uint8_t stage = (0u);
    static uint32_t delay = (0u);
    bool result = false;
    
    if( delay > GetTicks() ) return result;
    
    delay = GetTicks() + (50u);    
    
    switch( stage ){
        case 0:
            SetSoundTone(2u);
            break;
        case 1:
        case 3:
        case 5:
            spi_out = TLEDS_ON;
            break;
        case 6:
            result = true;
            break;
    }
    
    if( ++stage > (6u) ) stage = (0u);
    
    SpiWriteWord(spi_out);
    
    return result;    
}


/* [] END OF FILE */
