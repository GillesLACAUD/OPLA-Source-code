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
#include "Reverb.h"
#include "Nextion.h"
#include "Modulator.h"

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
        voice->spread=1.0;
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

    SoundMode=SND_MODE_POLY;
    WS.PolyMax=SND_MAX_POLY;

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

#define MVF     10

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
// Good filter but only for mono
// https://www.musicdsp.org/en/latest/Filters/26-moog-vcf-variation-2.html
#ifdef FILTER_6
float in1,in2,in3,in4;
float out1,out2,out3,out4;

float IRAM_ATTR KarlsenLPF(float input, float fc, float res, uint8_t m)
{
 float up2;
 float invf;
  
  res /=2;      
  float f = fc * 1.16;
  up2= f*f;
  invf = 1-f;
  float fb = res * (1.0 - 0.15*up2);
  input -= out4 * fb;
  input *= 0.35013 * (up2*up2);
  out1 = input + 0.3 * in1 + invf * out1; // Pole 1
  in1  = input;
  out2 = out1 + 0.3 * in2 + invf * out2;  // Pole 2
  in2  = out1;
  out3 = out2 + 0.3 * in3 + invf * out3;  // Pole 3
  in3  = out2;
  out4 = out3 + 0.3 * in4 + invf * out4;  // Pole 4
  in4  = out3;
  return out4;
}
#endif

#ifdef FILTER_7
float IRAM_ATTR KarlsenLPF(float in, float cut, float res, uint8_t m)
{
float resoclip;
static float buf1[6],buf2[6],buf3[6],buf4[6];
int16_t icut;
int16_t ires;
    
    // Compare to an int seem faster
    icut = cut*100;
    
    if(icut>85)
        cut=0.85;
    if(icut<0)
        cut=0.1;
    
    if(FilterType<FILTER_1LP)
        resoclip = buf4[m];
    else
        resoclip = buf2[m];
    ires = resoclip*100;
    
    if (ires > 73) resoclip = 0.73;
    in = (-resoclip * res)+in;

    buf1[m] = ((- buf1[m]+in)*cut) + buf1[m];
    buf2[m] = ((- buf2[m]+buf1[m])*cut) + buf2[m];
    if(FilterType<FILTER_1LP)
    {
        buf3[m] = ((- buf3[m]+buf2[m])*cut) + buf3[m];
        buf4[m] = ((- buf4[m]+buf3[m])*cut) + buf4[m];
    }
  

    switch(FilterType)
    {
        case FILTER_2LP: return buf4[m];
        case FILTER_2HP: return (buf4[m]-buf1[m]);
        case FILTER_2BP: return (in-buf1[m]);
        case FILTER_2NP: return (in-buf4[m]-buf1[m]);

        case FILTER_1LP: return buf2[m];
        case FILTER_1HP: return (buf2[m]-buf1[m]);
        case FILTER_1BP: return (in-buf1[m]);
        case FILTER_1NP: return (in-buf2[m]-buf1[m]);
    }
}
#endif

#ifdef FILTER_8

float IRAM_ATTR KarlsenLPF(float in, float cut, float res, uint8_t m)
{
static float buf0[6],buf1[6];
float resoclip;  
int16_t icut;
    //set feedback amount given f and q between 0 and 1
    //fb = 0;

    icut = cut*100;
    if(icut>80)
        cut=0.8;
    if(icut<10)
       cut=0.1;

    /*
    if(cut > 0.8) cut=0.8;
    if(cut < 0.1) cut=0.1;
    */
   resoclip = buf1[m];
   in = (-resoclip * res)+in;

    //for each sample...
    buf0[m] = buf0[m] + cut*(in-buf0[m]);           // 6db
    buf1[m] = buf1[m] + cut * (buf0[m] - buf1[m]);      // 12db
    return buf1[m];
}
#endif

#ifdef FILTER_1
inline float KarlsenLPF(float signal, float freq, float res, uint8_t m)
{
	static float b_inSH[MVF], b_in[MVF], b_f[MVF], b_q[MVF], b_fp[MVF], pole1[MVF], pole2[MVF], pole3[MVF], pole4[MVF];


	b_inSH[m] = signal;
	b_in[m] = signal;
	if(freq > 0.95f)freq = 0.95f;
	if(freq < 0.0f)freq = 0.0f;
	b_f[m] = freq;
	b_q[m] = res;
	uint8_t b_oversample = 0;
		
	while (b_oversample < 1)
	{	
		//2x oversampling
		float prevfp;
		prevfp = b_fp[m];
		if (prevfp > 1.0f) {prevfp = 1.0f;}	// Q-limiter

		b_fp[m] = (b_fp[m] * 0.418f) + ((b_q[m] * pole4[m]) * 0.582f);	// dynamic feedback
		float intfp;
		intfp = (b_fp[m] * 0.36f) + (prevfp * 0.64f);	// feedback phase
		b_in[m] =	b_inSH[m] - intfp;	                // inverted feedback	

		pole1[m] = (b_in[m] * b_f[m]) + (pole1[m] * (1.0f - b_f[m]));	// pole 1
		//if (pole1[m] > 1.0f) {pole1[m] = 1.0f;} else if (pole1[m] < -1.0f) {pole1[m] = -1.0f;} // pole 1 clipping
		pole2[m] = (pole1[m] * b_f[m]) + (pole2[m] * (1 - b_f[m]));	// pole 2
		pole3[m] = (pole2[m] * b_f[m]) + (pole3[m] * (1 - b_f[m]));	// pole 3
		pole4[m] = (pole3[m] * b_f[m]) + (pole4[m] * (1 - b_f[m]));	// pole 4

		b_oversample++;
	}
    /*
    LPF:=fPole[4];
    BPF:=fPole[4]-fPole[1];
    NPF:=I-fPole[1];
    HPF:=I-fPole[4]-fPole[1];
    */

    switch(FilterType)
    {
        case FILTER_2LP: return pole4[m];
        case FILTER_2HP: return (pole4[m]-pole1[m]);
        case FILTER_2BP: return (signal-pole1[m]);
        case FILTER_2NP: return (signal-pole4[m]-pole1[m]);

        case FILTER_1LP: return pole2[m];
        case FILTER_1HP: return (pole2[m]-pole1[m]);
        case FILTER_1BP: return (signal-pole1[m]);
        case FILTER_1NP: return (signal-pole2[m]-pole1[m]);
    }
    
}
#endif

#ifdef FILTER_2
inline float KarlsenLPF(float signal, float freq, float res, uint8_t m)
{
	static float b_inSH[6], b_f[6], b_q[6],pole1[6], pole2[6], pole3[6], pole4[6];

    int16_t reso = 100;

    if(1)
    {
	b_inSH[m] = signal;
	if(freq > 0.9f)freq = 0.9f;
	if(freq < 0.0f)freq = 0.0f;
	b_f[m] = freq;
	b_q[m] = res;
	float rez=0;

        rez = pole2[m]*b_q[m];
        if(rez>0.9){rez=0.9;}
        b_inSH[m] *=0.85;
        b_inSH[m] = b_inSH[m]-rez;
        

        pole1[m] = pole1[m] + ((-pole1[m] + b_inSH[m]) * b_f[m]);
        pole2[m] = pole2[m] + ((-pole2[m] + pole1[m]) * b_f[m]);
        pole3[m] = pole3[m] + ((-pole3[m] + pole2[m]) * b_f[m]);
        pole4[m] = pole4[m] + ((-pole4[m] + pole3[m]) * b_f[m]);
    }

    return pole4[m];

}
#endif

#ifdef FILTER_3
float IRAM_ATTR KarlsenLPF(float signal, float freq, float res, uint8_t m)
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
float IRAM_ATTR KarlsenLPF(float in, float cut, float res, uint8_t m)
{
float resoclip;
static float buf1[6],buf2[6],buf3[6],buf4[6];
    
    //return(in);

    if(cut > 0.8) cut=0.8;
    if(cut < 0) cut=0;
    //if(res > 0.9) res=0.9;
    resoclip = buf4[m]; if (resoclip > 1) resoclip = 1;
    in = in - (resoclip * res);
    buf1[m] = ((in - buf1[m]) * cut) + buf1[m];
    if (buf1[m] > 1.0f) {buf1[m] = 1.0f;} else if (buf1[m] < -1.0f) {buf1[m] = -1.0f;} // pole 1 clipping
    buf2[m] = ((buf1[m] - buf2[m]) * cut) + buf2[m];
    buf3[m] = ((buf2[m] - buf3[m]) * cut) + buf3[m];
    buf4[m] = ((buf3[m] - buf4[m]) * cut) + buf4[m];
    switch(FilterType)
    {
        case FILTER_2LP: return buf4[m];
        case FILTER_2HP: return (buf4[m]-buf1[m]);
        case FILTER_2BP: return (in-buf1[m]);
        case FILTER_2NP: return (in-buf4[m]-buf1[m]);
    }
 
}
#endif

#ifdef FILTER_5
/*
 * calculate coefficients of the 2nd order IIR filter
 */
void IRAM_ATTR Filter_Calculate(float c, float reso, struct filterCoeffT *const  filterC)
{
    float *aNorm = filterC->aNorm;
    float *bNorm = filterC->bNorm;

    float Q = reso;
    float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

    /*
     * change curve of cutoff a bit
     * maybe also log or exp function could be used
     */
    c = c * c * c;

    if (c >= 1.0f)
    {
        omega = 1.0f;
    }
    else if (c < 0.0025f)
    {
        omega = 0.0025f;
    }
    else
    {
        omega = c;
    }

    /*
     * use lookup here to get quicker results
     */
    cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
    sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];

    alpha = sinOmega / (2.0 * Q);
    b[0] = (1 - cosOmega) / 2;
    b[1] = 1 - cosOmega;
    b[2] = b[0];
    a[0] = 1 + alpha;
    a[1] = -2 * cosOmega;
    a[2] = 1 - alpha;

    // Normalize filter coefficients
    float factor = 1.0f / a[0];

    aNorm[0] = a[1] * factor;
    aNorm[1] = a[2] * factor;

    bNorm[0] = b[0] * factor;
    bNorm[1] = b[1] * factor;
    bNorm[2] = b[2] * factor;
}

inline void Filter_Process(float *const signal, struct filterProcT *const filterP)
{
    const float out = filterP->filterCoeff->bNorm[0] * (*signal) + filterP->w[0];
    filterP->w[0] = filterP->filterCoeff->bNorm[1] * (*signal) - filterP->filterCoeff->aNorm[0] * out + filterP->w[1];
    filterP->w[1] = filterP->filterCoeff->bNorm[2] * (*signal) - filterP->filterCoeff->aNorm[1] * out;
    *signal = out;
}
#endif

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Update_Tune(uint8_t type)
{
struct oscillatorT *osc;
float tmp;
uint8_t note;
uint8_t oldnote;

    switch(type)
    {
        case TUNE_SUB:
        for (int i = 0; i < WS.PolyMax; i++)
        {
            if (voicePlayer[i].active)
            {
                osc = &oscPlayer[2+i*3];                // 2 -> The thirth OSC is the sub
                note = voicePlayer[i].midiNote+WS.Transpose;
                osc->addVal = midi_note_to_add[note+(int8_t)SubTranspose]*(1.0+subdetune*0.9);
            }
        }        
        break;
        case TUNE_OSC:
        for(uint8_t v=0;v<=voc_act;v++)
        {
            if (voicePlayer[v].active)
            {
                note = voicePlayer[v].midiNote;
                voicePlayer[v].midiNote=note+WS.Transpose;
                // Detune OSC1
                osc = &oscPlayer[v*3+0];
                tmp= midi_note_to_add[note]*(1.0+oscdetune);
                osc->addVal = tmp;
                // Detune OSC2
                osc = &oscPlayer[v*3+1];
                tmp= midi_note_to_add[note]*(1.0-oscdetune*0.75);
                osc->addVal = tmp;
                // Detune OSC3 SUB
                osc = &oscPlayer[v*3+2];
                osc->addVal = midi_note_to_add[note+(int8_t)SubTranspose]*(1.0+subdetune*0.9);
            }
        }
        break;
        case TUNE_TRANSPOSE:
        for(uint8_t v=0;v<=voc_act;v++)
        {
            if (voicePlayer[v].active)
            {
                note = voicePlayer[v].midiNote+WS.Transpose;
                // Detune OSC1
                osc = &oscPlayer[v*3+0];
                tmp= midi_note_to_add[note]*(1.0+oscdetune);
                osc->addVal = tmp;
                // Detune OSC2
                osc = &oscPlayer[v*3+1];
                tmp= midi_note_to_add[note]*(1.0-oscdetune*0.75);
                osc->addVal = tmp;
                // Detune OSC3 SUB
                osc = &oscPlayer[v*3+2];
                osc->addVal = midi_note_to_add[note+(int8_t)SubTranspose]*(1.0+subdetune*0.9);
            }
        }
        break;
    }
}


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
        *ctrlSig += ctrl->a;
        if (*ctrlSig > (1.0f-ctrl->a))
        {
            *phase = decay;
        }
        break;
    case decay:
        *ctrlSig -= ctrl->d/4;
        if (*ctrlSig < ctrl->s+ctrl->d)
        {
            if(ctrl->loop)
                *phase = attack;
            else
                *phase = sustain;
        }
        break;
    case sustain:
        break;
    case standby:
        break;
    /*
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
    break;
    */
    case release:
    if (*ctrlSig > ctrl->r)
        *ctrlSig -= ctrl->r;
    else
        return false;
    /*
    if (*ctrlSig < 0.0f)
    {
        *ctrlSig = 0.0f;
        return false;
    }
    */
    break;   
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

    if(SoundMode !=SND_MODE_MONO)    
    {
        sprintf(messnex,"page0.b2.txt=%c%d%c",0x22,voc_act,0x22);
        Nextion_Send(messnex);
    }
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
void IRAM_ATTR Synth_Process(float *left, float *right)
{
bool voice_off;
float nz=0;
uint16_t Triinc,Tridec;
float finc,fdec;
uint32_t spread;
float ftmp;

int cmp;
int indx=0;

    nz = ((random(1024) / 512.0f) - 1.0f)*NoiseLevel*(1+NoiseMod);
    if(NoiseType == NOISE_POST)
        nz /=16;

    out_l = 0;
    out_r = 0;

    /* counter required to optimize processing */
    count += 1;

    /*
     * destination for unused oscillators
     */
    voiceSink[0] = 0;
    voiceSink[1] = 0;


    float sup;
    float inf;
    float tmp;
    float slope=0;
    int trig;
    
    //-------------------------------------------------
    // Rebuilt the wavework tab with WS1 vs waveform1
	// OUT wavework tab
    //-------------------------------------------------
    selectedWaveForm = &wavework[0];
	sup = OldWaveShapping1Mod+0.02;
	inf = OldWaveShapping1Mod-0.02;    
	if(WaveShapping1Mod > sup || WaveShapping1Mod < inf)
	{
		tmp = WaveShapping1+WaveShapping1Mod;
		if(tmp>0.99)
			tmp = 0.99;
		if(tmp<0)
			tmp = 0;
		OldWaveShapping1Mod = WaveShapping1Mod;

		switch(selWaveForm1)
		{

			case WAVE_SILENCE:
			for (i = 0; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = 0;
			}
			break;
			
			// PWM
			case WAVE_SQUARE:
			cmp = WAVEFORM_Q2-((int)(float)WAVEFORM_Q2*tmp);
			for (i = 0; i < cmp; i++)
			{
				wavework[i] = 1;
			}
			for (i = cmp; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = -1;
			}
			break;

			case WAVE_PULSE:
			trig=WAVEFORM_Q4 - (int)((float)(WAVEFORM_Q4)*tmp);
			if(trig!=WAVEFORM_Q4)
				slope=2/(float)(WAVEFORM_Q4-trig);
			for (i = 0; i < trig; i++)
			{
				wavework[i] = 1;
			}
			for (i = trig; i < WAVEFORM_Q4; i++)
			{
				wavework[i] = 1-slope*(i-trig);
			}
			for (i = WAVEFORM_Q4; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = -1;
			}
			break;

			case WAVE_TRI:
			// Tri to Saw
			for (i = 0; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = tri[i]*(1-tmp)+saw[i]*tmp;
				if(wavework[i]>1.0)
					wavework[i]=1.0;
				if(wavework[i]<-1.0)
					wavework[i]=-1.0;
			}
			break;

			case WAVE_NOISE:
			// Noise to Saw
			for (i = 0; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = noise[i]*(1-tmp)+saw[i]*tmp;      // Morphing
				if(wavework[i]>1.0)
					wavework[i]=1.0;
				if(wavework[i]<-1.0)
					wavework[i]=-1.0;
			}
			break;
		
			case WAVE_SINE:
			// Multiply sine
			cmp = 1+tmp*12;
			for (i = 0; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = sine[indx];
				indx+=cmp;
				if(indx>WAVEFORM_CNT)
					indx=0;
			}
			break;

			case WAVE_SAW:
			// Saw to Sine
			for (i = 0; i < WAVEFORM_CNT; i++)
			{
				wavework[i] = saw[i]*(1-tmp)+sine[i]*tmp;      // Morphing
				if(wavework[i]>1.0)
					wavework[i]=1.0;
				if(wavework[i]<-1.0)
					wavework[i]=-1.0;
			}
			break;

			case WAVE_AKWF:
			trig=WAVEFORM_CNT - (float)WAVEFORM_CNT*tmp;
			for (i = 0; i < trig; i++)
			{
				wavework[i] = waveAKWF[i];
			}
			int j=0;
			for (i = trig; i < WAVEFORM_CNT; i++)
			{
				//wavework[i] = saw[j];
				//j++;
				wavework[i] = 0;
			}
			break;

		}
	}
	//-------------------------------------------------
    // Add Waveshapping2
	// OUT wavework tab
    //-------------------------------------------------

    // wave shaping2 on going ---- ????
    /*
    tmp = 0.5;
    trig=WAVEFORM_CNT - (float)WAVEFORM_CNT*tmp;
    for (i = 0; i < trig; i++)
    {
        wavework[i] = wavework[i];
    }
    */
    // To much time ?
    /*
    sup = OldWaveShapping2Mod+0.10;
    inf = OldWaveShapping2Mod-0.10;  
    trig = WAVEFORM_CNT*(1-WaveShapping2Mod);
    if(WaveShapping2Mod > sup || WaveShapping2Mod < inf)
    {
        OldWaveShapping2Mod = WaveShapping2Mod;
        for (i = 0; i < trig; i++)
        {
            wavework[i] = 0;
        }
        for (i = trig; i < WAVEFORM_CNT; i++)
        {
        }
        
    }
    */


    /*
    for (i = 0; i < WAVEFORM_CNT; i++)
    {
        if(wavework[i]>1.0)
            wavework[i]=1.0;
        if(wavework[i]<-1.0)
           wavework[i]=-1.0;
    }
    */ 
	
	//-------------------------------------------------
    // Oscillator processing -> mix to voice
	// OUT dest[0],dest[1] for left right
    //-------------------------------------------------
    float sig;
    for (int v = 0; v < WS.PolyMax; v++) /* one loop is faster than two loops */
    {
        notePlayerT *voice = &voicePlayer[v];
        if (voice->active)            
        {
            for (int o = 0; o < OSC_PER_VOICE; o++)
            {
                oscillatorT *osc = &oscPlayer[o+v*OSC_PER_VOICE];
                // Apply the pitch spread
                spread = (uint32_t)((float)osc->addVal*(voice->spread));
                // Apply the pitch modulation and the pitch bend + the pitch EG
                osc->samplePos += (uint32_t)((float)spread*(1+PitchMod)*pitchMultiplier*(1+voice->p_control_sign*pitchEG));
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

	
    
    // Apply the filter Modulation
    FiltCutoffMod +=filtCutoff;
    if(FiltCutoffMod>1.0)
        FiltCutoffMod = 1.0;
    if(FiltCutoffMod<0.0)
        FiltCutoffMod = 0.0;

    float cf; // Temp for the filter cut frequency
    
	//-------------------------------------------------
    // Voice processing
	// OUT dest[0],dest[1] for left right
    //-------------------------------------------------    
    for (int i = 0; i < WS.PolyMax; i++) /* one loop is faster than two loops */
    {
        notePlayerT *voice = &voicePlayer[i];
        cf = FiltCutoffMod;
        if (voice->active)
        {
			// Update the EG only 1 time on 4
			// OUT
			// voice->control_sign 		for the volume
			// voice->f_control_sign	for the filter
			// voice->p_control_sign	for the pitch	
            if (count % 4 == 0)
            {
                voice_off = ADSR_Process(&adsr_vol, &voice->control_sign, &voice->phase);
                if (!voice_off && voice->active)
                {
                    for (int j = 0; j < WS.PolyMax ; j++)
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
                if(SoundMode==SND_MODE_POLY)
                {
                    (void)ADSR_Process(&adsr_fil, &voice->f_control_sign, &voice->f_phase);
                }
                else
                {
                    if(i==0)
                        (void)ADSR_Process(&adsr_fil, &voice->f_control_sign, &voice->f_phase);
                }
                (void)ADSR_Process(&adsr_pit, &voice->p_control_sign, &voice->p_phase);
            }
			
            // Add some noise to the voice pre filter
            if(NoiseType == NOISE_PRE)
                voice->lastSample[0] += nz*(1+NoiseMod);

            
            cf += voice->f_control_sign*filterEG;			// Apply EG Filter
            cf *=1+voice->fvelocity;						// Apply Velocity Filter
            cf *= 1+(voice->midiNote-64)*filterKBtrack;		// Apply Kbtrack

            // Apply EG Amp
            //voice->lastSample[0] *= voice->control_sign*voice->avelocity;			

			voice->lastSample[0] /=8.0; 
			// Filter for each voice
            if(SoundMode==SND_MODE_POLY)
			{
                voice->lastSample[0] = KarlsenLPF(voice->lastSample[0],cf, filtReso,i);
            }
			
            // Add some noise to the voice post filter
            
            if(NoiseType == NOISE_POST)
                voice->lastSample[0] += nz*(1+NoiseMod);
            

			// Apply EG Amp
            voice->lastSample[0] *= voice->control_sign*voice->avelocity;			
			
            out_l += voice->lastSample[0]*(1-voice->panspread);
            out_r += voice->lastSample[0]*(voice->panspread);

            voice->lastSample[0] = 0.0f;
        }
    }
    // Para or mono mode
    if(SoundMode!=SND_MODE_POLY)
    {
        out_l = KarlsenLPF(out_l,cf+voicePlayer[0].f_control_sign*filterEG, filtReso,0);
        out_r = KarlsenLPF(out_r,cf+voicePlayer[0].f_control_sign*filterEG, filtReso,1);
    }
    
    float multi = (1+AmpMod)*GeneralVolume;
    out_l *=multi;
    out_r *=multi;
    
    out_l *= (1+PanMod);
    out_r *= (1-PanMod);
   
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
uint8_t aff=0;    
    for (int i = 0; i < MAX_POLY_OSC ; i++)
    {
        if (oscPlayer[i].dest == voiceSink)
        {
            if(aff)
                Serial.printf("Get OSC %d\n",i);
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
uint8_t aff=0;

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
            if (aff) Serial.printf("Retrig\n");
            //voicePlayer[i].active = globalrank;
            *retrig=1;
            return &voicePlayer[i];
        }
    }
    */

    //--------------------------------------------
    // A voice is free - Simple case
    //--------------------------------------------
    for (int i = 0; i < WS.PolyMax ; i++)
    {
        if (voicePlayer[i].active == 0)
        {
            globalrank++;
            voicePlayer[i].active = globalrank;
            if (aff) Serial.printf("Free slot %d Rank %d MaxPoly %d Retrig %d\n",i,globalrank,WS.PolyMax,*retrig);
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
    for (int i = 0; i < WS.PolyMax ; i++)
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
        for (int i = 0; i < WS.PolyMax ; i++)
        {
            if(voicePlayer[i].active>1)
                voicePlayer[i].active--;
        }
        voicePlayer[keytab].active = globalrank;
        //Midi_Dump();        
        //if (aff) Serial.printf("Release\n");
        return &voicePlayer[keytab];
    }
    //--------------------------------------------
    // No Release note -Steal the first note
    //--------------------------------------------
    else
    {
        for(int i = 0; i < WS.PolyMax ; i++)
        {
            if(voicePlayer[i].active<keysteal)
            {
                keysteal = voicePlayer[i].active;
                keytab=i;
            }
        }
        if(keysteal!=99)
        {   
            if (aff) Serial.printf("Steal %d Act %d\n",keytab,voicePlayer[keytab].active);
            Voice_Off(keytab);   
            //*retrig=1;
            for (int i = 0; i < WS.PolyMax ; i++)
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

    
    // PanSpread  000 -> 1
    // PanSpread  127 -> 1 to 0 
    if(FlipPan)
        voice->panspread = (float)WS.PanSpread/127.0;
    else
        voice->panspread = 1-(float)WS.PanSpread/127.0;

    setvel = (float)vel/127;        // Real velocity
    voice->avelocity =1-(1-setvel)*AmpVel;
    voice->fvelocity = (setvel-0.5)*FilterVel; 
    if(!retrig)
    {
        voice->lastSample[0] = 0.0f;
        voice->lastSample[1] = 0.0f;
        voice->control_sign = 0.0f;
        voice->f_control_sign = 0;
        voice->f_control_sign_slow = adsr_fil.a;
    }
    voice->phase = attack;
    voice->p_phase = attack;

    if(SoundMode==SND_MODE_POLY)
        voice->f_phase = attack;
    else
    {
        if(voc_act==0)
            voice->f_phase = attack;  
    }

    for(uint8_t n=0;n<WS.PolyMax;n++)
    {
        if(adsr_fil.trig==1)
        {
            voicePlayer[n].f_phase = attack;  
            voicePlayer[n].f_control_sign = 0;
        }
        if(adsr_vol.trig==1)
        {
            voicePlayer[n].phase = attack;  
            voicePlayer[n].control_sign = 0;
        }
        if(adsr_pit.trig==1)
        {
            voicePlayer[n].p_phase = attack;  
            voicePlayer[n].p_control_sign = 0;
        }
    }

    // No voices
    if(voc_act==0)
    {
        if(Lfo1.ui8_Sync != LFO_FREE)
        {
            Lfo_cnt1=0;
        }
        if(Lfo2.ui8_Sync != LFO_FREE)
        {
            Lfo_cnt2=0;
        }
    }

    voc_act += 1;

    sprintf(messnex,"page0.b2.txt=%c%d%c",0x22,voc_act,0x22);
    Nextion_Send(messnex);

   

    /*
     * add oscillator
     */

    //osc = getFreeOsc();
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
        tmp= midi_note_to_add[note]*(1.0-oscdetune*0.75);
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
            osc->addVal = midi_note_to_add[note+(int8_t)SubTranspose]*(1.0+subdetune*0.9);
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
    
    /*
    for (int i = 0; i < WS.PolyMax; i++)
    {
        notePlayerT *voice = &voicePlayer[i];
        if (voice->active)
        {
            Serial.printf("V %d pA %d pF %d A %d\n",i,voice->phase,voice->f_phase,voice->active);
        }
    }
    */
    


    //Midi_Dump();
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Synth_NoteOff(uint8_t note)
{
    for (int i = 0; i < WS.PolyMax ; i++)
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
/* Play all the notes for monophonic sound         */
/*                                                 */
/*                                                 */
/***************************************************/
void Synth_MonoNoteOn(uint8_t note,uint8_t vel)
{
uint8_t retrig;
float setvel;

    struct notePlayerT *voice;
    struct oscillatorT *osc;

    for(uint8_t n=0;n<WS.PolyMax;n++)
    {
        voice = getFreeVoice(note,&retrig);
        osc = getFreeOsc();

        /*
        * No free voice found, return otherwise crash xD
        */
        if ((voice == NULL) || (osc == NULL))
        {  
            //Serial.printf("voc: %d, osc: %d\n", voc_act, osc_act);
            return ;
        }

        voicePlayer[n].midiNote = note;
        
        // PanSpread  000 -> 1
        // PanSpread  127 -> 1 to 0 
        if(FlipPan)
            voicePlayer[n].panspread = (float)WS.PanSpread/127.0;
        else
            voicePlayer[n].panspread = 1-(float)WS.PanSpread/127.0;

        setvel = (float)vel/127;        // Real velocity
        voicePlayer[n].avelocity =1-(1-setvel)*AmpVel;
        voicePlayer[n].fvelocity = (setvel-0.5)*FilterVel; 
        if(!retrig)
        {
            voicePlayer[n].lastSample[0] = 0.0f;
            voicePlayer[n].lastSample[1] = 0.0f;
            voicePlayer[n].control_sign = 0.0f;
            voicePlayer[n].f_control_sign = 0;
            voicePlayer[n].f_control_sign_slow = adsr_fil.a;
        }
        if(MonoCptNote==0)
        {
            voicePlayer[n].phase = attack;
            voicePlayer[n].p_phase = attack;
            voicePlayer[n].f_phase = attack;  
        }
        if(adsr_fil.trig==1)
        {
            voicePlayer[n].f_phase = attack;  
            voicePlayer[n].f_control_sign = 0;
        }
        if(adsr_vol.trig==1)
        {
            voicePlayer[n].phase = attack;  
            voicePlayer[n].control_sign = 0;
        }
        if(adsr_pit.trig==1)
        {
            voicePlayer[n].p_phase = attack;  
            voicePlayer[n].p_control_sign = 0;
        }

        // No voices
        if(voc_act==0)
        {
            if(Lfo1.ui8_Sync != LFO_FREE)
            {
                Lfo_cnt1=0;
            }
            if(Lfo2.ui8_Sync != LFO_FREE)
            {
                Lfo_cnt2=0;
            }
        }

        voc_act += 1;
    
        float tmp;
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

        osc_act += 1;

        osc = getFreeOsc();
        tmp= midi_note_to_add[note]*(1.0-oscdetune*0.75);
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

        osc = getFreeOsc();
        if (note + SubTranspose < 128)
        {
            osc->addVal = midi_note_to_add[note+(int8_t)SubTranspose]*(1.0+subdetune*0.9);
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

}

/***************************************************/
/* Stop all the notes for monophonic sound         */
/*                                                 */
/*                                                 */
/***************************************************/
void Synth_MonoNoteOff(uint8_t note)
{
    for (int i = 0; i < WS.PolyMax ; i++)
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
int Synth_Change(uint8_t s,uint8_t e, int val)
{
uint8_t range;
float factor;    

    *Tab_Encoder[s][e].Data=val;   // New value between -24 and +12   
    if(Tab_Encoder[s][e].Type==TYPE_LIST)
    {
        if(val==MAXPOT)
            val=MAXPOT-Tab_Encoder[s][e].Step/2;
        float value = val * NORM127MUL;
        Tab_Encoder[s][e].Index= (value) * (Tab_Encoder[s][e].Step);
    }
    else
    {
    
        range = Tab_Encoder[s][e].MaxData-Tab_Encoder[s][e].MinData;
        factor = (float)range/127;
        val = Tab_Encoder[s][e].MinData + (int)((float)val*factor);
        //Serial.printf("New2 %d\n",val);
    }
    Tab_Encoder[s][e].ptrfunctValueChange(val);
    return(val);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Synth_SetRotary(uint8_t rotary, int val)
{
uint8_t s=0,e=0;

    // Search the CC

    for(s=0;s<MAX_SECTION;s++)
    {
        for(e=0;e<MAX_ENCODER;e++)
        {
            if(Tab_Encoder[s][e].MidiCC==rotary)
            {
                //Serial.printf("New1 %d\n",val);
                val=Synth_Change(s,e,val);
                e=0x55;
                s=0x55;
            }
        }           
    }
    return(val);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int Synth_GetandSet(uint8_t rotary,int16_t val,int16_t signe)
{
uint8_t s=0,e=0;
int16_t newval=0;
uint8_t range;
float factor;
int16_t tmp;

    // Search the CC

    for(s=0;s<MAX_SECTION;s++)
    {
        for(e=0;e<MAX_ENCODER;e++)
        {
            if(Tab_Encoder[s][e].MidiCC==rotary)
            {
                newval=(int16_t)(*(Tab_Encoder[s][e].Data));
                newval+=signe*val;
                if(newval>MAXPOT)
                    newval=MAXPOT-Tab_Encoder[s][e].Step/2;
                if(newval<0)
                    newval=0;
                *(Tab_Encoder[s][e].Data)=(int16_t)newval;
                //Serial.printf("New1 %d\n",newval);
                newval=Synth_Change(s,e,newval);
                e=0x55;
                s=0x55;
            }
        }           
    }
    return(newval);
}



/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
int OldSynth_GetandSet(uint8_t rotary,int16_t val,int16_t signe)
{
uint8_t s=0,e=0;
int16_t newval=0;
uint8_t range;
float factor;
int16_t tmp;

    // Search the CC

    for(s=0;s<MAX_SECTION;s++)
    {
        for(e=0;e<MAX_ENCODER;e++)
        {
            if(Tab_Encoder[s][e].MidiCC==rotary)
            {
                newval=(int16_t)(*(Tab_Encoder[s][e].Data));
                newval+=signe*val;
                if(newval>MAXPOT)
                    newval=MAXPOT;
                if(newval<0)
                    newval=0;
                *(Tab_Encoder[s][e].Data)=(int16_t)newval;

                if(Tab_Encoder[s][e].Type==TYPE_LIST)
                {
                    if(newval==MAXPOT)
                        newval=MAXPOT-1;
                    float value = newval * NORM127MUL;
                    //Tab_Encoder[s][e].Index= (value) * (Tab_Encoder[s][e].MaxData);
                }
                else
                {
                    range = Tab_Encoder[s][e].MaxData-Tab_Encoder[s][e].MinData;
                    factor = (float)range/127;
                    newval = Tab_Encoder[s][e].MinData + (int)((float)newval*factor);
                }
                Tab_Encoder[s][e].ptrfunctValueChange(newval);
                e=0x55;
                s=0x55;
            }
        }           
    }
    return(newval);
}

