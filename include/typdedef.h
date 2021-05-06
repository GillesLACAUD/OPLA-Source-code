#include "Arduino.h"

struct adsrT
{
    float a;
    float d;
    float s;
    float r;
};

typedef enum
{
    attack, decay, sustain, release,standby
} adsr_phaseT;


struct filterCoeffT
{
    float aNorm[2] = {0.0f, 0.0f};
    float bNorm[3] = {1.0f, 0.0f, 0.0f};
};

struct filterProcT
{
    struct filterCoeffT *filterCoeff;
    float w[3];
};

struct notePlayerT
{
    float lastSample[2];

    float avelocity;        // For amplitude
    float fvelocity;        // For Filter
    uint8_t active;
    adsr_phaseT phase;
    uint8_t midiNote;

    float control_sign;
    float out_level;

    struct filterCoeffT filterC;
    struct filterProcT filterL;
    struct filterProcT filterR;
    float f_control_sign;
    float p_control_sign;
    float f_control_sign_slow;
    adsr_phaseT f_phase;
    adsr_phaseT p_phase;
};

struct oscillatorT
{
    float *waveForm;
    float *dest;
    uint32_t samplePos;
    uint32_t addVal;
    float pan_l;
    float pan_r;
};


typedef enum
{
    WLFO_SINE,WLFO_TRI,WLFO_SAW,WLFO_INVSAW,WLFO_SQUARE,WLFO_SH,WLFO_NOISE,WLFO_MUTE
} lfo_wave;

typedef enum
{
    LFO_AMP,LFO_CUTOFF,LFO_PITCH,LFO_NOISE,LFO_A_PAN,LFO_WS1,LFOx_SPEED,LFOx_AMT
} lfo_dest;

struct stLfo
{
	uint16_t	ui16_Freq;			// LFO Frequency
	uint8_t 	ui8_Wave;			// Waveform
    float       *waveForm;          // 
    uint8_t		ui8_Sync;		    // Type of sync
	float   	f_Amount;	        // Amounts 
	lfo_dest    ui8_Dest;           // Destination
    float       f_modlfo;           // Affectation

    uint8_t	    ui8_Endsave;        // 

    uint16_t    ui16_Cpt;           // Internal cpt
};
