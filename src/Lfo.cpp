#include "Arduino.h"
#include "typdedef.h"

#define __LFO__
#include "Lfo.h"
#include "easysynth.h"
#include "reverb.h"
#include "simple_delay.h"


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Lfo_Process(stLfo* prlfo)
{
float lfoamt=0;    
uint16_t freq=9999;    
static uint16_t sl1,sl2;
    
    if(prlfo == &Lfo1)
    {
        prlfo->ui16_Cpt = Lfo_cnt1;
        lfoamt = prlfo->f_Amount + Lfo1AmtMod;
        freq = prlfo->ui16_Freq*(1+Lfo1SpeedMod);
        freq +=LFO_MIN_CPT;
        if(freq !=sl1)
        {
            sl1=freq;
            timerAlarmWrite(Lfo_timer1,freq,true);       
        }
        
    }
    if(prlfo == &Lfo2)
    {
        prlfo->ui16_Cpt = Lfo_cnt2;        
        lfoamt = prlfo->f_Amount + Lfo2AmtMod;
        freq = prlfo->ui16_Freq*(1+Lfo2SpeedMod);
        freq +=LFO_MIN_CPT;
        if(freq !=sl2)
        {
            sl2=freq;
            timerAlarmWrite(Lfo_timer2,freq,true);       
        }        
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
        PitchMod += prlfo->f_modlfo/8;
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

        case LFO_REVPAN:
        RevPanMod = reverbPan+prlfo->f_modlfo/2;            // +/- 1 max;
        if(RevPanMod<0)
        RevPanMod=0;
        if(RevPanMod>1)
        RevPanMod=1;
        break;
        case LFO_DELAYPAN:
        DelayPanMod = delayPan+prlfo->f_modlfo/2;            // +/- 1 max;
        if(DelayPanMod<0)
        DelayPanMod=0;
        if(DelayPanMod>1)
        DelayPanMod=1;
        break;
        case LFO_REVAMT:
        break;
        case LFO_DELAYAMT:
        break;

    }
    
    
}