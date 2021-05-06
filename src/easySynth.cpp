/*
 * Implementation of a simple polyphonic synthesizer module
 * - it supports different waveforms
 * - it supports polyphony
 * - implemented ADSR for velocity and filter
 * - allows usage of multiple oscillators per voice
 *
 * Author: Marcel Licence
 */

#include <Arduino.h>
#include "typdedef.h"

#include "Global.h"
#include "simple_delay.h"
#define __SYNTH__
#include "easysynth.h"
#include "midi_interface.h"
#include "Lfo.h"
#include "AC101.h"
#include "Ihm.h"

#include "esp_attr.h"

extern AC101 ac;

void Synth_Init()
{
    randomSeed(34547379);

    /*
     * we do not check if malloc was successful
     * if there is not enough memory left the application will crash
     */
    /*
    sine = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    saw = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    square = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    pulse = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    tri = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    noise = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    silence = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    wavework = (float *)malloc(sizeof(float) * WAVEFORM_CNT);
    */

    

    Delay_Init();

    /*
     * let us calculate some waveforms
     * - using lookup tables can save a lot of processing power later
     * - but it does consume memory
     */
    for (int i = 0; i < WAVEFORM_CNT; i++)
    {
        float val = (float)sin(i * 2.0 * PI / WAVEFORM_CNT);
        sine[i] = val;
        saw[i] = (2.0f * ((float)i) / ((float)WAVEFORM_CNT)) - 1.0f;
        square[i] = (i > (WAVEFORM_CNT / 2)) ? 1 : -1;
        wavework[i] = ((i > (WAVEFORM_CNT / 2)) ? (((4.0f * (float)i) / ((float)WAVEFORM_CNT)) - 1.0f) : (3.0f - ((4.0f * (float)i) / ((float)WAVEFORM_CNT)))) - 2.0f;
        pulse[i] = (i > (WAVEFORM_CNT / 4)) ? 1 : -1;
        tri[i] = ((i > (WAVEFORM_CNT / 2)) ? (((4.0f * (float)i) / ((float)WAVEFORM_CNT)) - 1.0f) : (3.0f - ((4.0f * (float)i) / ((float)WAVEFORM_CNT)))) - 2.0f;
        noise[i] = (random(1024) / 512.0f) - 1.0f;
        silence[i] = 0;
    }

    /*
     * initialize all oscillators
     */
    for (int i = 0; i < MAX_POLY_OSC; i++)
    {
        oscillatorT *osc = &oscPlayer[i];
        osc->waveForm = silence;
        osc->dest = voiceSink;
    }

    /*
     * initialize all voices
     */
    for (int i = 0; i < MAX_POLY_VOICE; i++)
    {
        notePlayerT *voice = &voicePlayer[i];
        voice->active = false;
        voice->lastSample[0] = 0.0f;
        voice->lastSample[1] = 0.0f;
        voice->filterL.filterCoeff = &voice->filterC;
        voice->filterR.filterCoeff = &voice->filterC;
    }

    /*
     * prepare lookup for constants to drive oscillators
     */
    for (int i = 0; i < MIDI_NOTE_CNT; i++)
    {
        float f = ((pow(2.0f, (float)(i - 69) / 12.0f) * 440.0f));
        uint32_t add = (uint32_t)(f * ((float)(1ULL << 32ULL) / ((float)SAMPLE_RATE)));
        midi_note_to_add[i] = add;
    }

    
    
    /*
     * assign main filter
     */
    mainFilterL.filterCoeff = &filterGlobalC;
    mainFilterR.filterCoeff = &filterGlobalC;

    filtCutoff = 0.8/1.2;
    adsr_fil.s = filtCutoff;      

   
    Lfo1.ui16_Freq = 50;
    Lfo1.ui8_Dest = LFO_CUTOFF;
    Lfo1.ui8_Wave = WLFO_SINE;
    Lfo1.waveForm =sine;
    Lfo1.f_Amount = 0.0;
    Lfo1.f_modlfo = 0.0;
    
    Lfo2.ui16_Freq = 50;
    Lfo2.ui8_Dest = LFO_CUTOFF;
    Lfo2.ui8_Wave = WLFO_SINE;
    Lfo2.waveForm =sine;
    Lfo2.f_Amount = 0.0;


    // long release
    float value;    
    value = (10+20) * NORM127MUL;
    adsr_vol.r = (0.0020 * pow(100, 1.0f - value*2));    


}

struct filterCoeffT mainFilt;

/*
 * filter calculator:
 * https://www.earlevel.com/main/2013/10/13/biquad-calculator-v2/
 *
 * some filter implementations:
 * https://github.com/ddiakopoulos/MoogLadders/blob/master/src/Filters.h
 *
 * some more information about biquads:
 * https://www.earlevel.com/main/2003/02/28/biquads/
 */


// Karlsen 24dB Filter
// b_f = frequency 0..1
// b_q = resonance 0..5

#define FILTER_1
#define MVF     10

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
#ifdef FILTER_1
float IRAM_ATTR KarlsenLPF(float signal, float freq, float res, uint8_t m)
{
	static float b_inSH[MVF], b_in[MVF], b_f[MVF], b_q[MVF], b_fp[MVF], pole1[MVF], pole2[MVF], pole3[MVF], pole4[MVF];


	b_inSH[m] = signal;
	b_in[m] = signal;
	if(freq > 1.0f)freq = 1.0f;
	if(freq < 0.0f)freq = 0.0f;
	b_f[m] = freq;
	b_q[m] = res;
	uint8_t b_oversample = 0;
		
	while (b_oversample < 2)
	{	
		//2x oversampling
		float prevfp;
		prevfp = b_fp[m];
		if (prevfp > 1.0f) {prevfp = 1.0f;}	// Q-limiter

		b_fp[m] = (b_fp[m] * 0.418f) + ((b_q[m] * pole4[m]) * 0.582f);	// dynamic feedback
		float intfp;
		intfp = (b_fp[m] * 0.36f) + (prevfp * 0.64f);	// feedback phase
		b_in[m] =	b_inSH[m] - intfp;	// inverted feedback	

		pole1[m] = (b_in[m] * b_f[m]) + (pole1[m] * (1.0f - b_f[m]));	// pole 1
		if (pole1[m] > 1.0f) {pole1[m] = 1.0f;} else if (pole1[m] < -1.0f) {pole1[m] = -1.0f;} // pole 1 clipping
		pole2[m] = (pole1[m] * b_f[m]) + (pole2[m] * (1 - b_f[m]));	// pole 2
		pole3[m] = (pole2[m] * b_f[m]) + (pole3[m] * (1 - b_f[m]));	// pole 3
		pole4[m] = (pole3[m] * b_f[m]) + (pole4[m] * (1 - b_f[m]));	// pole 4

		b_oversample++;
	}
    return pole4[m];
}
#endif

#ifdef FILTER_2
float KarlsenLPF(float signal, float freq, float res, uint8_t m)
{
	static float b_inSH[6], b_in[6], b_f[6], b_q[6], b_fp[6], pole1[6], pole2[6], pole3[6], pole4[6];
	b_inSH[m] = signal;
	b_in[m] = signal;
	if(freq > 1.0f)freq = 1.0f;
	if(freq < 0.0f)freq = 0.0f;
	b_f[m] = freq;
	b_q[m] = res;
	uint8_t b_oversample = 0;
    float rez=0;

        rez = pole4[m]*b_q[m];if(rez>1.2){rez=1.2;}
        b_inSH[m] *=0.85;
        b_inSH[m] = b_inSH[m]-rez;
        

        pole1[m] = pole1[m] + ((-pole1[m] + b_inSH[m]) * b_f[m]);
        pole2[m] = pole2[m] + ((-pole2[m] + pole1[m]) * b_f[m]);
        pole3[m] = pole3[m] + ((-pole3[m] + pole2[m]) * b_f[m]);
        pole4[m] = pole4[m] + ((-pole4[m] + pole3[m]) * b_f[m]);

    return pole4[m];
	//return pole4[m];
}
#endif

#ifdef FILTER_3
float KarlsenLPF(float signal, float freq, float res, uint8_t m)
{
  float f, p, q;             //filter coefficients
  static float b0[6], b1[6], b2[6], b3[6], b4[6];  //filter buffers (beware denormals!)
  float t1, t2;              //temporary buffers

    // Set coefficients given frequency & resonance [0.0...1.0]


  q = 1.0f - freq;
  p = freq + 0.8f * freq * q;
  f = p + p - 1.0f;
  q = res * (1.0f + 0.5f * q * (1.0f - q + 5.6f * q * q));

// Filter (in [-1.0...+1.0])

  signal -= q * b4[m];                          //feedback
  t1 = b1[m];  b1[m] = (signal + b0[m]) * p - b1[m] * f;
  t2 = b2[m];  b2[m] = (b1[m] + t1) * p - b2[m] * f;
  t1 = b3[m];  b3[m] = (b2[m] + t2) * p - b3[m] * f;
            b4[m] = (b3[m] + t1) * p - b4[m] * f;
  b4[m] = b4[m] - b4[m] * b4[m] * b4[m] * 0.166667f;    //clipping
  b0[m] = signal;
}
#endif

#ifdef FILTER_4
float KarlsenLPF(float in, float cut, float res, uint8_t m)
{
float resoclip;
static float buf1[6],buf2[6],buf3[6],buf4[6];
    
    resoclip = buf4[m]; if (resoclip > 1) resoclip = 1;
    in = in - (resoclip * res);
    buf1[m] = ((in - buf1[m]) * cut) + buf1[m];
    if (buf1[m] > 1.0f) {buf1[m] = 1.0f;} else if (buf1[m] < -1.0f) {buf1[m] = -1.0f;} // pole 1 clipping
    buf2[m] = ((buf1[m] - buf2[m]) * cut) + buf2[m];
    buf3[m] = ((buf2[m] - buf3[m]) * cut) + buf3[m];
    buf4[m] = ((buf3[m] - buf4[m]) * cut) + buf4[m];

    return(buf4[m]);
}
#endif


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
/*
 * very bad and simple implementation of ADSR
 * - but it works for the start
 */
inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase)
{
    switch (*phase)
    {
    case attack:
        if(*ctrlSig <0.1)
            *ctrlSig += ctrl->a/8;        
        else if(*ctrlSig <0.2)
            *ctrlSig += ctrl->a/4;
        else if(*ctrlSig <0.4)
            *ctrlSig += ctrl->a/2;            
        else 
            *ctrlSig += ctrl->a;

        if (*ctrlSig > (1.0f-ctrl->a))
        {
            //*ctrlSig = 1.0f;
            *phase = decay;
        }
        break;
    case decay:
        *ctrlSig -= ctrl->d;
        if (*ctrlSig < ctrl->s+ctrl->d)
        {
            //*ctrlSig = ctrl->s;
            *phase = sustain;
        }
        break;
    case sustain:
    case standby:
        break;
    case release:
    if(*ctrlSig <0.1)
        *ctrlSig -= ctrl->r/8;        
    else if(*ctrlSig <0.2)
        *ctrlSig -= ctrl->r/4;
    else if(*ctrlSig <0.4)
        *ctrlSig -= ctrl->r/2;            
    else 
        *ctrlSig -= ctrl->r;
        if (*ctrlSig < 0.0f)
        {
            *ctrlSig = 0.0f;
            //voice->active = false;
            return false;
        }
    }
    return true;
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Voice_Off(uint32_t i)
{
    notePlayerT *voice = &voicePlayer[i];
    
    for (int f = 0; f < MAX_POLY_OSC; f++)
    {
        oscillatorT *osc = &oscPlayer[i*OSC_PER_VOICE+f];
        if (osc->dest == voice->lastSample)
        {
            osc->dest = voiceSink;
            osc_act -= 1;
        }
    }
    voc_act -= 1;
}

static float out_l, out_r;
static uint32_t count = 0;
int i;

extern portMUX_TYPE timer1Mux_xms;
extern portMUX_TYPE timer2Mux_xms;

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
//[[gnu::noinline, gnu::optimize ("fast-math")]]
void Synth_Process(float *left, float *right)
{
uint16_t j=0;    
int16_t tmp16;
bool voice_off;
float nz=0;
uint16_t Triinc,Tridec;
float finc,fdec;

int cmp;
static uint8_t cptvoice=0;
int indx=0;

    nz = ((random(1024) / 512.0f) - 1.0f)*NoiseLevel*(1+NoiseMod);
    /*
     * generator simulation, rotate all wheels
     */
    out_l = 0;
    out_r = 0;

    /* counter required to optimize processing */
    count += 1;

    /*
     * destination for unused oscillators
     */
    voiceSink[0] = 0;
    voiceSink[1] = 0;

    // Reset all the mod
    
    FiltCutoffMod = 0;
    PanMod=0;
    AmpMod=0;
    NoiseMod=0;
    WaveShapping1Mod=0;
    
    if(!Lfo1_Mutex)
    {
        Lfo1_Mutex=1;
        Lfo_Process(&Lfo1);
        Lfo1_Mutex=0;
    }
    if(!Lfo2_Mutex)
    {
        Lfo2_Mutex=1;
        Lfo_Process(&Lfo2);
        Lfo2_Mutex=0;
    }
    

    /*
     * update pitch bending / modulation
     */
    /*
    if (count % 64 == 0)
    {
        pitchMultiplier = pow(2.0f, pitchVar / 12.0f);
    }
    */

    float sup;
    float inf;
    float tmp;
    
    
    if(selectedWaveForm == wavework)
    {
        sup = OldWaveShapping1Mod+0.02;
        inf = OldWaveShapping1Mod-0.02;    
        if(WaveShapping1Mod > sup || WaveShapping1Mod < inf)
        {
            tmp = WaveShapping1+WaveShapping1Mod;
            OldWaveShapping1Mod = WaveShapping1Mod;

            switch(selWaveForm1)
            {
                // PWM
                case WAVE_SQUARE:
                if(tmp<0.01)
                    tmp = 0.01;
                if(tmp>0.99)
                    tmp = 0.99;
                cmp = ((int)(float)WAVEFORM_CNT*tmp);
                for (i = 0; i < WAVEFORM_CNT; i++)
                {
                    wavework[i] = (i > cmp) ? 1 : -1;
                }
                break;

                case WAVE_PULSE:
                break;

                case WAVE_TRI:
                // from -1 to +1
                // gate1 = -0.8 gate2 = 0.8
                // tmp from 0 to 1
                // gate1 = 1-tmp/2
                // gate2 = -gate1
                float gate1,gate2;
                gate1 = 1 -tmp/2;
                gate2 = 0-gate1;
                for (i = 0; i < WAVEFORM_CNT; i++)
                {
                    if(tri[i]>gate1)
                    {
                        wavework[i] = gate1;
                    }
                    else if(tri[i]<gate2)
                    {
                        wavework[i] = gate2;
                    }
                    else
                    {
                        wavework[i] = tri[i];
                    }
                }
                break;

                case WAVE_NOISE:
                for (i = 0; i < WAVEFORM_CNT/2; i++)
                {
                    tmp16 = EpData[j+1]<<8;
                    tmp16 +=EpData[j];
                    wavework[i] = (float)(tmp16)/32768.0f;
                    wavework[i+WAVEFORM_CNT/2] = wavework[i];
                    //Serial.printf("%04d %02x %02x %04x %3.2f\n",i,VoiceData[j],VoiceData[j+1],tmp16,wavework[i]);
                    j+=2;
                }
                break;
            
                case WAVE_SINE:
                
                //cmp = ((int)(float)WAVEFORM_CNT*tmp);
                //for (i = 0; i < WAVEFORM_CNT; i++)
                //{
                    //wavework[i] = (i > cmp) ? sine[i] : 0;
                //}
                
                cmp = 1+tmp*20;
                for (i = 0; i < WAVEFORM_CNT; i++)
                {
                    wavework[i] = sine[indx];
                    indx+=cmp;
                    if(indx>WAVEFORM_CNT)
                        indx=0;
                }

                break;

                // SAW To TRI
                case WAVE_SAW:
                if(tmp<0.05)
                    tmp = 0.0;
                if(tmp>0.95)
                    tmp = 1;
                        
                Triinc = (WAVEFORM_CNT/2)*(1+tmp);
                Tridec = (WAVEFORM_CNT/2)*(1-tmp);

                finc = 2.0/Triinc;
                fdec = 2.0/Tridec;
                
                for(i = 0;i<Triinc;i++)
                {
                    wavework[i] = -1 + i*finc;
                    //wavework[i] = sine[i];
                }
                for(i=0;i<Tridec;i++)
                {
                    //wavework[i]=0;
                    wavework[i] = 1 - i*fdec;
                }
                break;
            }
        }
    }

  
    /*
     * oscillator processing -> mix to voice
    */
    float sig;
    for (int v = 0; v < MAX_POLY_VOICE; v++) /* one loop is faster than two loops */
    {
        notePlayerT *voice = &voicePlayer[v];
        if (voice->active)            
        {
            for (int o = 0; o < 3; o++)
            {
                oscillatorT *osc = &oscPlayer[o+v*3];
                // Apply the pitch modulation and the pitch bend + yhe pitch EG
                osc->samplePos += (uint32_t)((float)osc->addVal*(1+PitchMod)*pitchMultiplier*(1+voice->p_control_sign*pitchEG));

                
                switch(o)
                {
                    case 0: sig = osc->waveForm[WAVEFORM_I(osc->samplePos)]*MixOsc;break;
                    case 1: sig = osc->waveForm[WAVEFORM_I(osc->samplePos)]*MixOsc;break;
                    case 2: sig = osc->waveForm[WAVEFORM_I(osc->samplePos)]*MixSub;break;
                }
                osc->dest[0] += osc->pan_l * sig;
                osc->dest[1] += osc->pan_r * sig;
            }
        }
    }
    PitchMod = 0;

    /*
     * voice processing
     */
    for (int i = 0; i < MAX_POLY_VOICE; i++) /* one loop is faster than two loops */
    {
        notePlayerT *voice = &voicePlayer[i];
        if (voice->active)
        {
            if (count % 4 == 0)
            {
                voice_off = ADSR_Process(&adsr_vol, &voice->control_sign, &voice->phase);
                if (!voice_off && voice->active)
                {
                    for (int j = 0; j < MAX_POLY_VOICE ; j++)
                    {
                        if(voicePlayer[j].active>voice->active)
                            voicePlayer[j].active--;
                    }
                    voice->midiNote=0;
                    voice->active=0;
                    voice->phase = standby;
                    voice->f_phase = standby;
                    voice->p_phase = standby;
                    globalrank--;
                    Voice_Off(i);
                }
                /*
                 * make is slow to avoid bad things .. or crying ears
                 */
                (void)ADSR_Process(&adsr_fil, &voice->f_control_sign, &voice->f_phase);
                (void)ADSR_Process(&adsr_pit, &voice->p_control_sign, &voice->p_phase);
            }
            if (voice->active)
            {

                /* add some noise to the voice */
                voice->lastSample[0] += nz*(1+NoiseMod);
                voice->lastSample[1] += nz*(1+NoiseMod);

                voice->lastSample[0] *= voice->control_sign * voice->avelocity;
                voice->lastSample[1] *= voice->control_sign * voice->avelocity;

                //channel[i] = channel[i] * (vcacutoff[i] * vcavellvl[i]);
                //summer = summer + KarlsenLPF(channel[i], (vcfval * vcfenvlvl) * ((vcfcutoff[i] * vcfkeyfollow[i]) * vcfvellvl[i]), resonance, i);

                // Apply the filter Modulation
                FiltCutoffMod +=filtCutoff;
                if(FiltCutoffMod>1.0)
                    FiltCutoffMod = 1.0;
                if(FiltCutoffMod<0.0)
                    FiltCutoffMod = 0.0;

                // Apply the filter EG
                float cf = FiltCutoffMod+voice->f_control_sign*filterEG;
                cf *=1+voice->fvelocity;
                // Apply the kbtrack
                cf *= 1+(voice->midiNote-64)*filterKBtrack;
                voice->lastSample[0] = KarlsenLPF(voice->lastSample[0],cf, filtReso,i);
                voice->lastSample[1] = voice->lastSample[0];

                out_l += voice->lastSample[0];
                out_r += voice->lastSample[1];
                voice->lastSample[0] = 0.0f;
                voice->lastSample[1] = 0.0f;
            }
        }
    }

    cptvoice++;
    if(cptvoice==MAX_POLY_VOICE)
        cptvoice=0;
        
    float multi = (1+AmpMod)*0.25f;
    out_l *=multi;
    out_r *=multi;

    out_l *= (1+PanMod);
    out_r *= (1-PanMod);

    /*
     * process delay line
     */
    Delay_Process(&out_l, &out_r);

    /*
     * reduce level a bit to avoid distortion
     */
    

    
   
    /*
     * finally output our samples
     */
    *left = out_l;
    *right = out_r;
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
struct oscillatorT *getFreeOsc()
{
    for (int i = 0; i < MAX_POLY_OSC ; i++)
    {
        if (oscPlayer[i].dest == voiceSink)
        {
            return &oscPlayer[i];
        }
    }
    return NULL;
}

// voicePlayer[0].active = 2
// voicePlayer[1].active = 1   -> first voice to steal if no release 
// voicePlayer[2].active = 3
// voicePlayer[3].active = 0   -> this voice is free 

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
struct notePlayerT *getFreeVoice(uint8_t note,uint8_t* retrig)
{
uint8_t keysteal=99;    

    *retrig=0;
    //--------------------------------------------
    // Retrig the same note - Simple case
    //--------------------------------------------
    /*
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if (voicePlayer[i].active && voicePlayer[i].midiNote==note)
        {
            Voice_Off(i);
            //voicePlayer[i].active = globalrank;
            *retrig=1;
            return &voicePlayer[i];
        }
    }
    */

    //--------------------------------------------
    // A voice is free - Simple case
    //--------------------------------------------
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if (voicePlayer[i].active == 0)
        {
            globalrank++;
            voicePlayer[i].active = globalrank;
            return &voicePlayer[i];
        }
    }
    // Steal a voice here
    // Search first the lower rank release note
    keysteal =99;
    uint8_t keytab;
    *retrig=1;  
    //--------------------------------------------
    // Search a note on release phase
    //--------------------------------------------
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if(voicePlayer[i].phase==release)
        {
            if(voicePlayer[i].active<keysteal)
            {
                keysteal = voicePlayer[i].active;
                keytab=i;
            }
        }
    }
    //--------------------------------------------
    // A note is on release phase
    //--------------------------------------------
    if(keysteal!=99)
    {
        //Serial.printf("R S %d Act %d\n",keytab,voicePlayer[keytab].active);
        //Midi_Dump();        
        Voice_Off(keytab);  
        //*retrig=1;   
        for (int i = 0; i < MAX_POLY_VOICE ; i++)
        {
            if(voicePlayer[i].active>1)
                voicePlayer[i].active--;
        }
        voicePlayer[keytab].active = globalrank;
        //Midi_Dump();        
        return &voicePlayer[keytab];
    }
    //--------------------------------------------
    // No Release note -Steal the first note
    //--------------------------------------------
    else
    {
        for(int i = 0; i < MAX_POLY_VOICE ; i++)
        {
            if(voicePlayer[i].active<keysteal)
            {
                keysteal = voicePlayer[i].active;
                keytab=i;
            }
        }
        if(keysteal!=99)
        {   
            //Serial.printf("S S %d Act %d\n",keytab,voicePlayer[keytab].active);
            Voice_Off(keytab);   
            //*retrig=1;
            for (int i = 0; i < MAX_POLY_VOICE ; i++)
            {
            if(voicePlayer[i].active>1)
                voicePlayer[i].active--;
            }
            voicePlayer[keytab].active = globalrank;
            return &voicePlayer[keytab];
        }
    }
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Filter_Reset(struct filterProcT *filter)
{
    filter->w[0] = 0.0f;
    filter->w[1] = 0.0f;
    filter->w[2] = 0.0f;
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Synth_NoteOn(uint8_t note,uint8_t vel)
{
uint8_t retrig;
float setvel;

    struct notePlayerT *voice = getFreeVoice(note,&retrig);
    struct oscillatorT *osc = getFreeOsc();
   
    /*
     * No free voice found, return otherwise crash xD
     */
    if ((voice == NULL) || (osc == NULL))
    {  
        //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
        return ;
    }

    voice->midiNote = note;
    setvel = (float)vel/127;        // Real velocity
    //setvel *= AmpVel;               // Apply setting curve
    //setvel *=0.75;                  // Apply global amp

    voice->avelocity = setvel*AmpVel*0.75; 
    voice->avelocity = 0.5;

    voice->fvelocity = setvel*FilterVel*1.0; 
    if(!retrig)
    {
        voice->lastSample[0] = 0.0f;
        voice->lastSample[1] = 0.0f;
        voice->control_sign = 0.0f;
    }
    voice->phase = attack;
    voice->f_phase = attack;
    voice->p_phase = attack;

    voice->f_control_sign = 0;
    voice->f_control_sign_slow = adsr_fil.a;

    voc_act += 1;
  

    /*
     * add oscillator
     */

    osc = getFreeOsc();
    float tmp;
    if (osc != NULL)
    {
        tmp = midi_note_to_add[note]*(1.0+oscdetune);
        //tmp = midi_note_to_add[note];     // No Detune
        osc->addVal = tmp;
        if(!retrig)
        {
            //osc->samplePos = 0;
        }
        osc->waveForm = selectedWaveForm;
        osc->dest = voice->lastSample;
        osc->pan_l = 1;
        osc->pan_r = 1;
    }

    osc_act += 1;

    osc = getFreeOsc();
    if (osc != NULL)
    {
        tmp= midi_note_to_add[note]*(1.0-oscdetune);
        //tmp= midi_note_to_add[note];      // No Detune
        osc->addVal = tmp;
        if(!retrig)
        {
            //osc->samplePos = 0;
        }
        osc->waveForm = selectedWaveForm;
        osc->dest = voice->lastSample;
        osc->pan_l = 1;
        osc->pan_r = 1;

        osc_act += 1;
    }  
      

    osc = getFreeOsc();
    if (osc != NULL)
    {
        if (note + SubTranspose < 128)
        {
            osc->addVal = midi_note_to_add[note+SubTranspose]*(0.5-oscdetune);
            if(!retrig)
            {
                //osc->samplePos = 0; /* we could add some offset maybe */
            }
            osc->waveForm = selectedWaveForm2;
            osc->dest = voice->lastSample;
            osc->pan_l = 1;
            osc->pan_r = 1;

            osc_act += 1;
        }
    }
    //Midi_Dump();
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Synth_NoteOff(uint8_t note)
{
    for (int i = 0; i < MAX_POLY_VOICE ; i++)
    {
        if ((voicePlayer[i].active) && (voicePlayer[i].midiNote == note))
        {
            voicePlayer[i].phase = release;
            voicePlayer[i].f_phase = release;
            voicePlayer[i].p_phase = release;
            
        }
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Synth_SetRotary(uint8_t rotary, int val)
{
uint8_t s=0,e=0;
uint8_t range;
float factor;

    // Search the CC

    for(s=0;s<MAX_SECTION;s++)
    {
        for(e=0;e<MAX_ENCODER;e++)
        {
            if(Tab_Encoder[s][e].MidiCC==rotary)
            {
                if(Tab_Encoder[s][e].Type==TYPE_LIST)
                {
                    float value = val * NORM127MUL;
                    Tab_Encoder[s][e].Index= (value) * (Tab_Encoder[s][e].MaxData);
                }
                // 0-127 to min max data -> 0 = Min data 127 = Max data
                // -24 to +12   range = 36
                // factor = 0,283
                // val = 0   -> -24
                // val = 64  -> -6
                // val = 127 -> 12
                else
                {
                    range = Tab_Encoder[s][e].MaxData-Tab_Encoder[s][e].MinData;
                    factor = (float)range/127;
                    val = Tab_Encoder[s][e].MinData + (int)((float)val*factor);
                    
                }
                Tab_EncoderVal[s][e]=val;   // Keep the value
                Tab_Encoder[s][e].ptrfunctValueChange(val);
                e=0x55;
                s=0x55;
            }
        }           
    }
    return(val);

}















