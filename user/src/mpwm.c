

#include <project.h>
#include "mpwm.h"

/* -------------------------- Local functions -------------------------------*/
void SoundTimerStart(void);
void SoundTimerStop(void);


#define F1KHZ       (4700u)
#define BL_PERIOD   (20000u)

/* ------------------------------ Globals -----------------------------------*/
static struct _snd
{
    uint8_t		tone:3;    	//cypso tonas (0-7). Jai 0 - nera cypso
    uint16_t    freq;       //cypso daznis
    uint8_t     volume;
    bool        enable;
}SndCtrl = {1u, F1KHZ, MIDLE, true};

static struct _bl
{
    uint16_t    timeout;
    uint16_t    value;
    uint16_t    counter;
    bool        enable;    
}BlCtrl = {BL_TIMEOUT, BL_MAX, BL_TIMEOUT, false};

/* ------------------------------- Locals -----------------------------------*/



/*============================================================================
Implementation
============================================================================*/
void BacklightInit(uint8_t enable, uint16_t timeout)
{
    PwmCLK_SetFractionalDividerRegister((4u), (0u)); 
    BacklightTmr_Start();
    BacklightTmr_WritePeriod(BL_PERIOD);
    BacklightTmr_WriteCompare(BlCtrl.value);
    BlCtrl.enable = enable;
    BlCtrl.timeout = timeout;
}

void BlSetValue(uint16_t bl){
    BlCtrl.value = bl;
}
uint16_t BlGetValue(){
    return BlCtrl.value;
}
void BlResetCounter(){
    BlCtrl.counter = (0u);
}
uint16_t BlGetCounter(){
    return BlCtrl.counter;
}

// grazinom Blank busena. true - jai LEDai aktyvus, false - jai uzgesinti
bool BlGetState(){
    if(BlCtrl.value <= BL_MIN) return false;    
    return true;
}
void BlStartTimeout(){
    BlCtrl.counter = BlCtrl.timeout;
    BlCtrl.value = BL_MAX;
    BacklightTmr_WriteCompare(BlCtrl.value);
}
void BlBlancEnable(uint8_t enable){
    BlCtrl.enable = (enable) ? true : false;
}
void Backlight(void){
    if(!BlCtrl.enable) return;
    
    /* LEDu gesinimas po timeaut'o*/
    if(BlCtrl.value >= BL_MIN)
    {
        if(BlCtrl.counter)
        {
            BlCtrl.counter--;
            return;
        }

        if(BlCtrl.value > BL_MIN)
        {
            BlCtrl.value -= (10u);
            BacklightTmr_WriteCompare(BlCtrl.value);  
            
            //SetSoundVolume(NONE);   //sumazinam garsa kai prigesinam displeju
        }

        return;
    }
}

void SoundInit(uint8_t enable, uint8_t volume){
    PwmCLK_SetFractionalDividerRegister((4u), (0u)); 
    BuzzerTmr_Start();
    SndCtrl.enable = enable;
    SndCtrl.volume = volume;
    SoundTimerStop();
}
void Sound(){
    static uint8_t stage, cnt = (1u), begin = true;
    
    if(!SndCtrl.enable || !SndCtrl.tone) return;    
    
    BuzzerTmr_WriteCompare((uint16_t)SndCtrl.volume * (23u));
    
    if(begin)
    {
        begin = false;
        stage = (0u);
    }else{		
        if(--cnt) return;  
    }
      
    if(SndCtrl.tone == (1u))
    {
      SoundTimerStart();
      SndCtrl.freq = F1KHZ;
      switch(stage)
        {
          case 0:{cnt = (30u); break;}
          case 1:{goto lp110;}
        }
        stage++;
        return;            
    }            
    if(SndCtrl.tone == (2u))
    {
      SoundTimerStart();
      switch(stage)
        {
          case 0:{cnt = (50u); SoundTimerStart(); break;}
          case 1:{cnt = (50u); SoundTimerStop(); break;}
          case 2:{cnt = (50u); SoundTimerStart(); break;}
          case 3:{cnt = (50u); SoundTimerStop(); break;}
          case 4:{goto lp110;}
        }
        stage++;
        return;
    }            
    if(SndCtrl.tone == (3u))
    {
      SndCtrl.freq = F1KHZ;        
      switch(stage)
        {
          case 0:{cnt = (50u); SoundTimerStart(); break;}
          case 1:{cnt = (50u); SoundTimerStop(); break;}
          case 2:{cnt = (50u); SoundTimerStart(); break;}
          case 3:{cnt = (50u); SoundTimerStop(); break;}
          case 4:{cnt = (50u); SoundTimerStart(); break;}
          case 5:{SndCtrl.freq = F1KHZ; goto lp110;}
        }
        if(stage < 5) stage++;
        else stage = 5;
        return;
    }
    if(SndCtrl.tone == (4u))
    {		
      SoundTimerStart();
      switch(stage)
        {
          case 0:{cnt = (40u); SndCtrl.freq = F1KHZ; break;}
          case 1:{cnt = (40u); SndCtrl.freq = (4600u); break;}
          case 2:{cnt = (40u); SndCtrl.freq = (4500u); break;}
          case 3:{cnt = (40u); SndCtrl.freq = (4400u); break;}
          case 4:{cnt = (40u); SndCtrl.freq = (4300u); break;}
          case 5:{cnt = (40u); SndCtrl.freq = (4200u); break;}
          case 6:{cnt = (40u); SndCtrl.freq = (4100u); break;}
          case 7:{cnt = (40u); SndCtrl.freq = (4000u); break;}
          case 8:{SndCtrl.freq = F1KHZ; goto lp110;}
        }
        stage++;
        return;
    }
  lp110:
    SoundTimerStop();
    stage = (0u);
    SndCtrl.tone = (0u);
    begin = true;
  }
void SoundTimerStart(){
    BuzzerTmr_Wakeup();
    BuzzerTmr_WriteCounter(0u);    
    BuzzerTmr_WritePeriod(SndCtrl.freq);
}
void SoundTimerStop(){
    BuzzerTmr_WriteCounter(0u);
    BuzzerTmr_Sleep();
}
void SetSoundTone(uint8_t tone){
    if( SndCtrl.tone || SndCtrl.volume == NONE ) return; 
    SndCtrl.tone = tone;    
}
bool GetSoundTone(){
    if( SndCtrl.tone > 0 ) return true;
    else return false;
}


void SetSoundVolume(uint8_t volume){    
    
    if( volume >= NONE && volume < LOW) volume = NONE;
    else if( volume >= LOW && volume < MIDLE) volume = 2;
         else if( volume >= MIDLE && volume < HIGH) volume = 6;
              else if( volume >= HIGH ) volume = HIGH;
    
    SndCtrl.volume = volume;
}
void SoundEnable(uint8_t enable){
    SndCtrl.enable = (enable) ? true : false;
}

/*============================================================================
END OF FILE 
============================================================================*/