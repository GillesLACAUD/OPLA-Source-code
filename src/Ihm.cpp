#include "Arduino.h"
#include "typdedef.h"

#include "easysynth.h"

#define __IHM__
#include "Ihm.h"

#include "Lfo.h"
#include "simple_delay.h"
#include "reverb.h"
#include "Modulator.h"
#include "Nextion.h"
#include "SDCard.h"
#include "ArpSeq.h"

uint8_t serialdebug=0;
extern uint8_t Midi_KeyOn;
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
static uint8_t keepwav=0;

    value = val * NORM127MUL;
    selWaveForm1 = (value) * (WAVEFORM_TYPE_COUNT);

    for(uint8_t o=0;o<=osc_act;o+=3)
    {
        osc = &oscPlayer[o+0];
        osc->waveForm = selectedWaveForm;

        osc = &oscPlayer[o+1];
        osc->waveForm = selectedWaveForm;
    }
    if(selWaveForm1==WAVE_AKWF && keepwav !=WAVE_AKWF)
    {
        trigloadwave=1;
    }
    keepwav=selWaveForm1;

    // Force update
    OldWaveShapping1Mod += 0.03;
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
    selWaveForm2 = (value) * (WAVEFORM_SUB_COUNT);
    selectedWaveForm2 = waveFormLookUp[selWaveForm2];
    for(uint8_t o=0;o<=osc_act;o+=3)
    {
        osc = &oscPlayer[o+2];
        osc->waveForm = selectedWaveForm2;
    } 
    if(serialdebug)
        Serial.printf("selWaveForm2: val %d value %5.2f wave %d\n",val,value,selWaveForm2);

    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_NoiseType(int val) 
{
float value;    

    value = val * NORM127MUL;
    NoiseType = (value) * (NOISE_TYPE_COUNT);
    if(serialdebug)       
       Serial.printf("Filter type: %d\n",NoiseType);

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
    NoiseLevel = value*0.5;
    if(serialdebug)
        Serial.printf("Noise Level: %f\n", NoiseLevel);

    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Bank(int val) 
{
    if(selWaveForm1!=WAVE_AKWF)
        return(0);
  
    if(!trigloadwave)
    {
        trigloadwave=1;
        if(!IsLoadSound)
        {
            sprintf(messnex,"page 4");
            Nextion_Send(messnex);
        }
    }
    Cptloadwave=0;
    // No plot screen when load sound
    if(!IsLoadSound)
    {
        sprintf(messnex,"page3.BK.txt=%c%03d%c",0x22,WS.OscBank,0x22);
        Nextion_Send(messnex);
    }
   
    sprintf(messnex,"page3.BKNAME.txt=%c%s%c",0x22,SampleDIR[WS.OscBank].name,0x22);
    Nextion_Send(messnex);      
    Tab_Encoder[SECTION_BANK_MAX][POT_BANK_MAX].MaxData=SampleDIR[WS.OscBank].nbr-1;  // Chg the max for the MIDI CC  



    sprintf(messnex,"page3.WAPOT.maxval=%d",SampleDIR[WS.OscBank].nbr-1);
    Nextion_Send(messnex);

    if(!IsLoadSound)
        WS.AKWFWave=0;

    sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
    Nextion_Send(messnex);
    sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
    Nextion_Send(messnex);

    sprintf(messnex,"page3.BKPOT.val=%d",WS.OscBank);
    Nextion_Send(messnex);

    if(serialdebug)
        Serial.printf("BANK: %d Max %d MIDI CC %d\n", WS.OscBank,Tab_Encoder[2][6].MaxData,Tab_Encoder[2][6].MidiCC);

    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Wave(int val) 
{
    if(selWaveForm1!=WAVE_AKWF)
        return(0);
    if(!trigloadwave)
    {
        trigloadwave=1;
        if(!IsLoadSound)
        {
            sprintf(messnex,"page 4");
            Nextion_Send(messnex);
        }
    }

    Cptloadwave=0;
    // No plot screen when load sound
    //if(!IsLoadSound)
    //{
        
    //}
    if(serialdebug)
        Serial.printf("WAVE: %d\n", WS.AKWFWave);

    sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
    Nextion_Send(messnex);
    sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
    Nextion_Send(messnex);

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

    value = val * NORM127MUL;
    if(val==0)
    {
        oscdetune = 0;
    }
    else
    {
        oscdetune =(0.0001*pow(500,value)); 
    }
    Update_Tune(TUNE_OSC);

    if(serialdebug)
        Serial.printf("Osc Detune: %f\n", oscdetune);

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
    WaveShapping1=value;
    WaveShapping1Mod = 0; 
    OldWaveShapping1Mod=0;  // Force update waveform
    OldWaveShapping1Mod = WaveShapping1Mod+0.5;
    if(serialdebug)
        Serial.printf("WS1: %f\n",WaveShapping1);

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
int Fct_Ch_WS2(int val)   
{
float value;    

    value = val * NORM127MUL;
    WaveShapping2=value;
    WaveShapping2Mod = 0; 
    OldWaveShapping2Mod=0;  // Force update waveform
    OldWaveShapping2Mod = WaveShapping2Mod+0.5;
    if(serialdebug)
        Serial.printf("WS2: %f\n",WaveShapping2);
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
    MixOsc = value*0.7;
    if(serialdebug)
        Serial.printf("Osc vol: %f\n",MixOsc);
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
    MixSub = value*0.7;
    if(serialdebug)
        Serial.printf("Sub vol: %f\n",MixSub);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SubDetune(int val)
{
float value;    

    value = val * NORM127MUL;
    if(val==0)
    {
        subdetune = 0;
    }
    else
    {
        subdetune =(0.0001*pow(500,value)); 
    }
    Update_Tune(TUNE_SUB);

    if(serialdebug)
        Serial.printf("Sub Detune: %f\n", subdetune);

    return(0);

}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SubOct(int val)
{
    SubTranspose = (float)val;
    Update_Tune(TUNE_SUB);

    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PanSpread(int val)
{
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
    #ifdef FILTER_5
    filtCutoff = value;
    adsr_fil.s = value;
    Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
    #else
    filtCutoff = 0.005+value/1.2;
    //filtCutoff = 0.001+(0.0005*pow(1000,value));         // ok for KarlsenLPF
    adsr_fil.s = filtCutoff;   
    #endif
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
    #ifdef FILTER_5
    filtReso =  0.5f + 5 * value * value * value; /* min q is 0.5 here */
    Filter_Calculate(filtCutoff, filtReso, &filterGlobalC);
    soundFiltReso = 0.5f + 5 * value * value * value; /* min q is 0.5 here */
    #else
    //filtReso =  0.5f + 5 * value * value * value; /* min q is 0.5 here */
    filtReso =  0.5f + 3 * value * value * value; /* min q is 0.5 here */
    #endif
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
float value=0;    

    value = val * NORM127MUL;       // 0 to 1
    // 0.5 to 2
    filterKBtrack = (value-0.5)/10;
    float cf;
    cf=filtCutoff;
    cf *= 1+(76-64)*filterKBtrack;

    if(serialdebug)
        Serial.printf("Filter KBtrack: %0.3f Cut %0.3f ModCut %0.3f\n",filterKBtrack,filtCutoff,cf);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FVelo(int val) 
{
    FilterVel = val * NORM127MUL*5;
    if(serialdebug)
        Serial.printf("Filter Velo: %0.3f\n",FilterVel);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FType(int val) 
{
float value=0; 

    value = val * NORM127MUL;
    FilterType = (value) * (MAX_FLT_TYPE);
    if(serialdebug)       
        Serial.printf("Filter type: %d\n",FilterType);
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
    #ifdef FILTER_5
    //adsr_fil.a = value;
    adsr_fil.a = (0.00010 * pow(2000, 1.0f - value));
    #else
    adsr_fil.a = (0.00005 * pow(5000, 1.0f - value));         
    #endif
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
    #ifdef FILTER_5
    adsr_fil.d = (0.00005 * pow(5000, 1.0f - value));
    #else
    adsr_fil.d = (0.0010 * pow(100, 1.0f - value*2));    
    #endif
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
    adsr_vol.s = value;

    if(serialdebug)       
        Serial.printf("Amp Sustain: %f Loop Mode %d\n",adsr_vol.s,adsr_vol.loop);
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
int Fct_Ch_AmVelo(int val)
{
    AmpVel = val* NORM127MUL;
    if(serialdebug)
        Serial.printf("Amp Velo: %0.3f\n",AmpVel);
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
float value;    

    value = val * NORM127MUL;
    pitchEG = (value-0.5)*2;      // +/- 1
    if(serialdebug)
        Serial.printf("Pitch EG: %f\n",pitchEG);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Portamento(int val)
{
float value;    

    value = (val+20) * NORM127MUL;
     return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Spread(int val)
{
float value;    
float nz=0;

    value = val*NORM127MUL;
    for (int v = 0; v < WS.PolyMax; v++)
    {
        notePlayerT *voice = &voicePlayer[v];
        if(SoundMode ==SND_MODE_MONO)
            nz = ((((float)random(1024))/1024.0)-0.5)/10;
        else
            nz = ((((float)random(1024))/1024.0)-0.5)/30;
            
        voice->spread = 1.0+(nz*value);
        
        if(serialdebug)
            Serial.printf("Spread %d %6.3f\n",v,voice->spread);
    }
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
    if(value<0.25)
        Lfo1.ui16_Freq = LFO_MAX_TIME*(1-value)*8+LFO_MIN_CPT;
    else
        if(value<0.5)
            Lfo1.ui16_Freq = LFO_MAX_TIME*(1-value)*2+LFO_MIN_CPT;
        else
            Lfo1.ui16_Freq = LFO_MAX_TIME*(1-value)/2+LFO_MIN_CPT;
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
    if(serialdebug)    
        Serial.printf("LFO1 Shape: %d\n",WS.LFO1Shape);
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
    if(serialdebug)    
        Serial.printf("LFO1 Dest: %d\n",WS.LFO1Dest);
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
    if(serialdebug)    
        Serial.printf("LFO1 Dest: %d\n",WS.LFO1Amount);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L1Sync(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo1.ui8_Sync = (value) * (WAVE_LFO_SYNC);  
    if(serialdebug)    
        Serial.printf("LFO1 Sync: %d\n",WS.LFO1Sync);
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
    if(value<0.25)
        Lfo2.ui16_Freq = LFO_MAX_TIME*(1-value)*8+LFO_MIN_CPT;
    else
        if(value<0.5)
            Lfo2.ui16_Freq = LFO_MAX_TIME*(1-value)*2+LFO_MIN_CPT;
        else
            Lfo2.ui16_Freq = LFO_MAX_TIME*(1-value)/2+LFO_MIN_CPT;
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

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_L2Sync(int val)
{
float value;    

    value = val * NORM127MUL;
    Lfo2.ui8_Sync = (value) * (WAVE_LFO_SYNC);  
    if(serialdebug)    
        Serial.printf("LFO2 Sync: %d\n",Lfo2.ui8_Sync);
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
    if(val==0)
    {
        switch(SoundMode)
        {
            case SND_MODE_POLY:
            break;

            case SND_MODE_PARA:
            WS.PolyMax=SND_MAX_PARA+1;
            break;

            case SND_MODE_MONO:
            WS.PolyMax=SND_MAX_MONO+1;
            break;
        }
    }
    else
    {
        switch(SoundMode)
        {
            case SND_MODE_POLY:
            break;

            case SND_MODE_PARA:
            WS.PolyMax=SND_MAX_PARA;
            break;

            case SND_MODE_MONO:
            WS.PolyMax=SND_MAX_MONO;
            break;
        }
    }
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


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_DlPP(int val)
{
float value;    

    value = val * NORM127MUL;
    delayPan = value;
    DelayPanMod=delayPan;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Reverb(int val)
{
float value;    

    value = val * NORM127MUL;
    Reverb_SetLevel(0,value/2);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_RevPan(int val)
{
float value;    

    value = val * NORM127MUL;
    reverbPan = value;
    RevPanMod=reverbPan;
    return(0);
}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SoundMode(int val)
{
float value;    
  
    value = val * NORM127MUL;
    SoundMode = (value) * (MAX_SND_MODE);
   
    switch(SoundMode)
    {
        case SND_MODE_POLY:
        WS.PolyMax=SND_MAX_POLY;
        break;

        case SND_MODE_PARA:
        WS.PolyMax=SND_MAX_PARA;
        if(WS.DelayAmount==0)
            WS.PolyMax++;
        break;

        case SND_MODE_MONO:
        WS.PolyMax=SND_MAX_MONO;
        if(WS.DelayAmount==0)
            WS.PolyMax++;
        break;
    }

    if(serialdebug)       
        Serial.printf("Sound Mode: %d Max Poly %d\n",SoundMode,WS.PolyMax);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PBRange(int val)
{
float value;    

    value = val * NORM127MUL;
    if(serialdebug)       
        Serial.printf("PB Range: %d\n",WS.PBRange);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MDDest(int val)
{
float value;    

    value = val * NORM127MUL;
    ui8_ModWheelDest = (value) * (MOD_MAX);  
    if(serialdebug)       
        Serial.printf("MW Destination: %d\n",WS.MWDest);

    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MDAmt(int val)
{
float value;    

    value = val * NORM127MUL;
    ModWheelAmount=value;
    if(serialdebug)       
        Serial.printf("MW Amount: %d\n",WS.MWAmt);
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ATDest(int val)
{
float value;    

    value = val * NORM127MUL;
    ui8_AfterTouchDest = (value) * (MOD_MAX);  
    if(serialdebug)       
        Serial.printf("AT Destination: %d\n",WS.ATDest);

    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ATAmt(int val)
{
float value;    

    value = val * NORM127MUL;
    AfterTouchAmount=value;
    if(serialdebug)       
        Serial.printf("AT Amount: %d\n",WS.ATAmt);
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MidiRx(int val)
{
    if(!IsLoadSound)
        SDCard_SaveMidiRx();
    if(serialdebug)       
        Serial.printf("MIDI RX: %d\n",MidiRx);    
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MidiMode(int val) 
{
float temp;    
    if(!IsLoadSound)
        SDCard_SaveMidiRx();
    if(serialdebug)       
        Serial.printf("MIDI MODE: %d\n",MidiMode); 

    temp=((float)MidiMode/127)*MIDI_MODE_MAX;

    RealMidiMode=temp;    
    
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MidiRelCC(int val)
{
        if(!IsLoadSound)
            SDCard_SaveMidiRx();
        if(serialdebug)       
            Serial.printf("MIDI REL CC: %d\n",MidiRelCC);    
        return(0);    
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MidiRelMin(int val)
{
    if(!IsLoadSound)
        SDCard_SaveMidiRx();
    if(serialdebug)       
        Serial.printf("MIDI RelMin: %d\n",MidiRelMin);    
    return(0);   
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_MidiRelMax(int val)
{
    if(!IsLoadSound)
        SDCard_SaveMidiRx();
    if(serialdebug)       
        Serial.printf("MIDI RelMax: %d\n",MidiRelMax);    
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Transpose(int val)
{
float value;    

    oscillatorT *osc;
    value = val * NORM127MUL;
    Update_Tune(TUNE_TRANSPOSE);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_SVolume(int val)
{
    if(val<100)
        GeneralVolume = val * 0.01;
    else
        GeneralVolume = val * 0.02;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FilterLoop(int val)
{
    adsr_fil.loop=0;
    if(val>64)
        adsr_fil.loop=1;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_AmpLoop(int val)
{
    adsr_vol.loop=0;
    if(val>64)
        adsr_vol.loop=1;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PitchLoop(int val)
{
    adsr_pit.loop=0;
    if(val>64)
        adsr_pit.loop=1;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_FilterTrig(int val)
{
    adsr_fil.trig=0;
    if(val>64)
        adsr_fil.trig=1;
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_AmpTrig(int val)
{
    adsr_vol.trig=0;
    if(val>64)
        adsr_vol.trig=1;

    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_Calib(int val)
{
    if(val>64)
    {
        Nextion_Send("touch_j");
    }
    return(0);
}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_PitchTrig(int val)
{
    adsr_pit.trig=0;
    if(val>64)
        adsr_pit.trig=1;


    return(0);

}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpOnOff(int val)
{
    Timer1ms_cnt=0;
    if(val>64)
        u8_ArpOn = 1;
    else
    {
        u8_ArpOn= 0;
        Arp_Stop_Note();
    }
    if(serialdebug)       
        Serial.printf("ARP ON OFF: %d\n",u8_ArpOn);
    return(0);        

}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpHold(int val) 
{
    if(val>64)
        u8_ArpHold = 1;
    else
    {
        u8_ArpHold = 0;
        if(u8_ArpOn && u8_ArpTrig)
        {
            Serial.printf("Midi Key On %d\n",Midi_KeyOn);
        }
        if(!Midi_KeyOn)
        {
            u8_ArpNbKeyOn=0;
            u8_ArpNextNbKeyOn=0;
            u8_ArpTrig=0;
            Arp_Stop_Note();
        }
    }
    if(serialdebug)       
        Serial.printf("ARP HOLD: %d\n",u8_ArpHold);
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpSpeed(int val)
{
    u8_ArpSpeed = val;
    u32_ArpTime = (TabDiv[u8_ArpDiv][TABARPDIVDELTA]*(127-u8_ArpSpeed))/127 +  TabDiv[u8_ArpDiv][TABARPDIVMAX];
    u32_ArpTimeOff = (u32_ArpTime*WS.ArpGate)/100;
    //Timer1ms_cnt=0;
    if(serialdebug)       
        Serial.printf("ARP SPEED: %d ARPTIME %d\n",u8_ArpSpeed,u32_ArpTime);
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpDiv(int val)  
{
float value=0; 

    value = val * NORM127MUL;
    u8_ArpDiv = (value) * (MAXARPDIV);    
    u32_ArpTime = TabDiv[u8_ArpDiv][TABARPDIVDELTA]*(127-u8_ArpSpeed);
    u32_ArpTime /=127;
    u32_ArpTime +=TabDiv[u8_ArpDiv][TABARPDIVMAX];
    u32_ArpTimeOff = (u32_ArpTime*WS.ArpGate)/100;
    //Timer1ms_cnt=0;
    if(serialdebug)       
        Serial.printf("ARP DIV: %d ARPTIME %d\n",u8_ArpDiv,u32_ArpTime);
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpMode(int val) 
{
float value=0; 

    value = val * NORM127MUL;
    u8_ArpMode = (value) * (MAXARPMODE);
    if(u8_ArpMode !=u8_ArpNewMode)
    {
        u8_ArpTrigMode=1;
    }

    if(serialdebug)       
        Serial.printf("ARP MODE: %d\n",u8_ArpMode);
    return(0);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpOct(int val)  
{

}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpGate(int val) 
{
    u32_ArpTimeOff = (u32_ArpTime*WS.ArpGate)/100;
    if(serialdebug)       
        Serial.printf("ARP GATE: %d\n",WS.ArpGate);
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Fct_Ch_ArpSwing(int val)
{
    ArpSwing = (float)WS.ArpSwing/127.0;
    if(serialdebug)       
        Serial.printf("ARP SWING: %d\n",WS.ArpSwing);
}






