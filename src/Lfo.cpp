#include "Arduino.h"
#include "typdedef.h"

#define __LFO__
#include "Lfo.h"
#include "easysynth.h"


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Lfo_Process(stLfo* prlfo)
{
float lfoamt;    
uint16_t freq=9999;    

    if(prlfo == &Lfo1)
    {
        prlfo->ui16_Cpt = Lfo_cnt1;
        lfoamt = prlfo->f_Amount + Lfo1AmtMod;
        freq = prlfo->ui16_Freq*(1+Lfo1SpeedMod);
        freq +=LFO_MIN_CPT;
        timerAlarmWrite(Lfo_timer1,freq,true);       
    }
    if(prlfo == &Lfo2)
    {
        prlfo->ui16_Cpt = Lfo_cnt2;        
        lfoamt = prlfo->f_Amount + Lfo2AmtMod;
        freq = prlfo->ui16_Freq*(1+Lfo2SpeedMod);
        freq +=LFO_MIN_CPT;
        timerAlarmWrite(Lfo_timer2,freq,true);       
        //Serial.printf("lfoamt2 %f Lfo1AmtMod %f Lfo2AmtMod %f\n",lfoamt,Lfo1AmtMod,Lfo2AmtMod);
    }

    switch(prlfo->ui8_Wave)        
    {
        case WLFO_SINE:
        prlfo->f_modlfo = sine[prlfo->ui16_Cpt]*lfoamt;
        break;
        case WLFO_TRI:
        prlfo->f_modlfo = tri[prlfo->ui16_Cpt]*lfoamt;
        break;
        case WLFO_SAW:
        prlfo->f_modlfo = saw[prlfo->ui16_Cpt]*lfoamt;
        break;
        case WLFO_INVSAW:
        prlfo->f_modlfo = -saw[prlfo->ui16_Cpt]*lfoamt;
        break;
        case WLFO_SQUARE:
        prlfo->f_modlfo = square[prlfo->ui16_Cpt]*lfoamt;
        break;
        case WLFO_SH:
        if(!prlfo->ui16_Cpt%(WAVE_LFO_SIZE/10))
        {
            prlfo->f_modlfo = ((random(1024) / 512.0f) - 1.0f)*lfoamt;
            prlfo->f_modlfo *=2;
            if(prlfo->f_modlfo>1.0)
                prlfo->f_modlfo=1;
            if(prlfo->f_modlfo<-1.0)
                prlfo->f_modlfo=-1;
        }
        break;
        case WLFO_NOISE:
        prlfo->f_modlfo = noise[prlfo->ui16_Cpt]*lfoamt;
        break;
        case WLFO_MUTE:
        prlfo->f_modlfo = silence[prlfo->ui16_Cpt]*lfoamt;
        break;
    }

    switch(prlfo->ui8_Dest)
    {
        case LFO_AMP:
        AmpMod += prlfo->f_modlfo/2;
        break;
        case LFO_CUTOFF:
        FiltCutoffMod += prlfo->f_modlfo/2;
        break;
        case LFO_PITCH:
        PitchMod += prlfo->f_modlfo/10;
        break;
        case LFO_NOISE:
        NoiseMod += prlfo->f_modlfo;            // +/- 1 max
        break;
        case LFO_A_PAN:
        PanMod += prlfo->f_modlfo/2;              // +/- 1 max
        break;
        case LFO_WS1:
        WaveShapping1Mod += prlfo->f_modlfo;
        break;

        case LFOx_SPEED:
        if(prlfo == &Lfo1)
            Lfo2SpeedMod=prlfo->f_modlfo;
        if(prlfo == &Lfo2)
            Lfo1SpeedMod=prlfo->f_modlfo;
        break;
        case LFOx_AMT:
        if(prlfo == &Lfo1)
            Lfo2AmtMod=prlfo->f_modlfo;
        if(prlfo == &Lfo2)
            Lfo1AmtMod=prlfo->f_modlfo;
        break;
    }
}