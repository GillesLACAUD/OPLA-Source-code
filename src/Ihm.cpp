#include "Arduino.h"
#include "typdedef.h"

#include "easysynth.h"

#define __IHM__
#include "Ihm.h"

#include "Lfo.h"
#include "simple_delay.h"

uint8_t serialdebug=0;

//--------------------------------------------------
// OSC
//--------------------------------------------------

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_OscWave(int val)
{
float value;
struct oscillatorT *osc;

    value = val * NORM127MUL;
    selWaveForm1 = (value) * (WAVEFORM_TYPE_COUNT);
    selectedWaveForm = waveFormLookUp[selWaveForm1];

    for(uint8_t o=0;o<=osc_act;o+=3)
    {
        osc = &oscPlayer[o+0];
        osc->waveForm = selectedWaveForm;

        osc = &oscPlayer[o+1];
        osc->waveForm = selectedWaveForm;
    }
    if(serialdebug)
        Serial.printf("selWaveForm1: %d\n", selWaveForm1);      
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SubWave(int val)
{
float value;    
struct oscillatorT *osc;

    value = val * NORM127MUL;
    selWaveForm2 = (value) * (WAVEFORM_TYPE_COUNT);
    selectedWaveForm2 = waveFormLookUp[selWaveForm2];
    // bug here ????    
    for(uint8_t o=0;o<=osc_act;o+=3)
    {
        osc = &oscPlayer[o+2];
        osc->waveForm = selectedWaveForm2;
    } 
    if(serialdebug)
        Serial.printf("selWaveForm2: %d\n", selWaveForm2);

    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Noise(int val) 
{
float value;    

    value = val * NORM127MUL;
    NoiseLevel = value/1;
    if(serialdebug)
        Serial.printf("Noise Level: %f\n", NoiseLevel);

    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Detune(int val)
{
float value;    
struct notePlayerT *voice;
struct oscillatorT *osc;
float tmp;
uint8_t note;

    value = val * NORM127MUL;
    oscdetune =  value/30;    
    for(uint8_t v=0;v<=voc_act;v++)
    {
        voice = &voicePlayer[v];
        note = voice->midiNote;

        osc = &oscPlayer[v*3+0];
        tmp= midi_note_to_add[note]*(1.0+oscdetune);
        osc->addVal = tmp;
        
        osc = &oscPlayer[v*3+1];
        tmp= midi_note_to_add[note]*(1.0-oscdetune);
        osc->addVal = tmp;
        
    }

    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_WS1(int val)   
{
float value;    
struct oscillatorT *osc;

    value = val * NORM127MUL;
    OldWaveShapping1Mod=value+0.5;  // Force update waveform
    WaveShapping1=value;
    if(serialdebug)
        Serial.printf("WS1: %f\n",WaveShapping1);
    /*
    if(WaveShapping1<0.01)
        WaveShapping1 = 0.01;
    if(WaveShapping1>0.99)
        WaveShapping1 = 0.99;
    cmp = ((int)(float)WAVEFORM_CNT*WaveShapping1);
    for (int i = 0; i < WAVEFORM_CNT; i++)
    {
        wavework[i] = (i > cmp) ? 1 : -1;
    }
    */

    selectedWaveForm = &wavework[0];
    for(uint8_t o=0;o<=osc_act;o+=3)
    {
        osc = &oscPlayer[o+0];
        osc->waveForm = selectedWaveForm;

        osc = &oscPlayer[o+1];
        osc->waveForm = selectedWaveForm;
    }
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_OscMix(int val)
{
float value;    

    value = val * NORM127MUL;
    MixOsc = value;
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SubMix(int val)
{
float value;    

    value = val * NORM127MUL;
    MixSub = value;
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SubOct(int val)
{
uint8_t note;

    SubTranspose = val;
    oscillatorT *osc;

    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if (voicePlayer[i].active)
        {
            osc = &oscPlayer[2+i*3];                // 2 -> The thirth OSC is the sub
            note = voicePlayer[i].midiNote;
            osc->addVal = midi_note_to_add[note+SubTranspose];
        }
    }
    return(0);
}

//--------------------------------------------------
// FILTER
//--------------------------------------------------

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Cutoff(int val)
{
float value;    

    value = val * NORM127MUL;
    filtCutoff = value/1.2;
    adsr_fil.s = filtCutoff;   
    if(serialdebug)   
        Serial.printf("main filter cutoff: %f\n", filtCutoff);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Resonance(int val)
{
float value;    

    value = val * NORM127MUL;
    filtReso =  0.5f + 5 * value * value * value; /* min q is 0.5 here */
    if(serialdebug)
        Serial.printf("main filter reso: %0.3f\n", filtReso);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_KbTrack(int val) 
{
/*
float value=0;    

    value = val * NORM127MUL;
    */
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FlAttack(int val)
{
float value;    

    value = val * NORM127MUL;
    adsr_fil.a = (0.00005 * pow(5000, 1.0f - value));         
    //adsr_fil.a = value;       
    if(serialdebug) 
        Serial.printf("Filter Attack: %f\n",adsr_fil.a);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FlDecay(int val)
{
float value;    

    value = val * NORM127MUL;
    adsr_fil.d = (0.0010 * pow(100, 1.0f - value*2));    
    if(serialdebug)  
        Serial.printf("Filter Decay: %f\n",adsr_fil.d);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FlRelease(int val)
{
float value;    

    value = val * NORM127MUL;
    adsr_fil.r = (0.0001 * pow(100, 1.0f - value));       
    if(serialdebug) 
        Serial.printf("Filter Release: %f\n",adsr_fil.r);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FlAmount(int val)
{
float value;    

    value = val * NORM127MUL;
    filterEG = (value-0.5)*2;      // +/- 1
    if(serialdebug)
        Serial.printf("Filter EG: %f\n",filterEG);
    return(0);
}

//--------------------------------------------------
// EG
//--------------------------------------------------

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_AmAttack(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
    adsr_vol.a = (0.00005 * pow(5000, 1.0f - value))/2;      
    if(serialdebug) 
        Serial.printf("Amp Attack: %f\n",adsr_vol.a);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_AmDecay(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
    adsr_vol.d = (0.00005 * pow(5000, 1.0f - value))/2;     
    if(serialdebug)   
        Serial.printf("Amp Decay: %f\n",adsr_vol.d);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_AmSustain(int val)
{
float value;    

    value = val * NORM127MUL;
    adsr_vol.s = (0.01 * pow(100, value)); 
    if(serialdebug)       
        Serial.printf("Amp Sustain: %f\n",adsr_vol.s);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_AmRelease(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
    adsr_vol.r = (0.0020 * pow(100, 1.0f - value*2));    
    if(serialdebug)    
        Serial.printf("Amp Release: %f\n",adsr_vol.r);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PiAttack(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
    adsr_pit.a = (0.00005 * pow(5000, 1.0f - value))/2;      
    if(serialdebug) 
        Serial.printf("Pitch Attack: %f\n",adsr_pit.a);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PiDecay(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
    adsr_pit.d = (0.00005 * pow(5000, 1.0f - value))/2;     
    if(serialdebug)   
        Serial.printf("Pitch Decay: %f\n",adsr_pit.d);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PiRelease(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
    adsr_pit.r = (0.0020 * pow(100, 1.0f - value*2));    
    if(serialdebug)    
        Serial.printf("Pitch Release: %f\n",adsr_pit.r);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PiAmount(int val)
{
/*
float value=0;    

    value = val * NORM127MUL;
    */
    return(0);
}

//--------------------------------------------------
// LFO
//--------------------------------------------------

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L1Speed(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo1.ui16_Freq = LFO_MAX_TIME*(1-value)+LFO_MIN_CPT;
    timerAlarmWrite(Lfo_timer1,Lfo1.ui16_Freq,true);     // 1024 -> T=1s 2048 T=2s 16=2048/127 T=16ms = 32 Hertz
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L1Shape(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo1.ui8_Wave = (value) * (WAVE_LFO_COUNT);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L1Dest(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo1SpeedMod=0;
    Lfo1AmtMod=0;
    Lfo1.ui8_Dest = (lfo_dest)((value) * (DEST_TYPE_COUNT));  
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L1Amount(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo1.f_Amount=value;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L2Speed(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo2.ui16_Freq = LFO_MAX_TIME*(1-value)*2+LFO_MIN_CPT;
    timerAlarmWrite(Lfo_timer2,Lfo2.ui16_Freq,true);     // 1024 -> T=1s 2048 T=2s 16=2048/127 T=16ms = 32 Hertz
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L2Shape(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo2.ui8_Wave = (value) * (WAVE_LFO_COUNT);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L2Dest(int val)  
{
float value;    

    value = val * NORM127MUL;
    Lfo1SpeedMod=0;
    Lfo1AmtMod=0;
    Lfo2.ui8_Dest = (lfo_dest)((value) * (DEST_TYPE_COUNT));  
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L2Amount(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo2.f_Amount=value;           
    return(0);
}

//--------------------------------------------------
// FX
//--------------------------------------------------

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_DlLen(int val)
{
float value;    

    value = val * NORM127MUL;
    Delay_SetLength(value); 
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_DlAmount(int val)
{
float value;    

    value = val * NORM127MUL;
    Delay_SetLevel(value);     
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_DlFeed(int val)
{
float value;    

    value = val * NORM127MUL;
    Delay_SetFeedback(value); 
    return(0);
}