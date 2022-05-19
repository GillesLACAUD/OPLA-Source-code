#include "Arduino.h"

struct WorkSound
{

    int16_t SoundMode;
    int8_t  PolyMax;

    int16_t OscBank;
    int16_t OscWave;
    int16_t SubWave;
    int16_t NoiseLevel;
    int16_t OscDetune;
    int16_t WaveShapping1;
    int16_t OscVolume;
    int16_t SubVolume;
    int16_t SubTranspose;

    int16_t Cutoff;
    int16_t Resonance;
    int16_t KbTrack;
    int16_t FVelo;
    int16_t FType;
    int16_t FEgAttack;
    int16_t FEgDecay;
    int16_t FEgRelease;
    int16_t FEgAmount;

    int16_t AEgAttack;
    int16_t AEgDecay;
    int16_t AEgSustain;
    int16_t AEgRelease;
    int16_t AmpVelo;

    int16_t PEgAttack;
    int16_t PEgDecay;
    int16_t PEgRelease;
    int16_t PEgAmount;
    int16_t Portamento;
    int16_t PBRange;

    int16_t LFO1Speed;
    int16_t LFO1Shape;
    int16_t LFO1Dest;
    int16_t LFO1Amount;

    int16_t LFO2Speed;
    int16_t LFO2Shape;
    int16_t LFO2Dest;
    int16_t LFO2Amount;

    int16_t DelayLen;
    int16_t DelayAmount;
    int16_t DelayFeedback;
    int16_t DelayPP;

    int16_t ReverbLevel;
    int16_t ReverbPan;

    int16_t MWDest;
    int16_t MWAmt;
    int16_t ATDest;
    int16_t ATAmt;

    int16_t Spread;
    int16_t Transpose;
    int16_t SVolume;


    int16_t LFO1Sync;           // Add 12.06.21
    int16_t LFO2Sync;

    int16_t NoiseType;          // Add 22.06.21
    int16_t PanSpread;
    int16_t AKWFWave;           // Add 16.08.21
    int16_t SubDetune;          // Add 15.09.21

    int16_t FilterLoop;          // Add 17.09.21
    int16_t AmpLoop;             // Add 17.09.21
    int16_t PitchLoop;           // Add 17.09.21

    int16_t FilterTrig;          // Add 25.09.21
    int16_t AmpTrig;             // Add 25.09.21
    int16_t PitchTrig;           // Add 25.09.21

    // Add 29.11.21
    int16_t ArpOnOff;
    int16_t ArpHold;
    int16_t ArpSpeed;
    int16_t ArpDiv;
    int16_t ArpMode;
    int16_t ArpOct;
    int16_t ArpGate;
    int16_t ArpSwing;

    // Add 19.02.22
    int16_t Decimator;
    int16_t WDDecimator;
    int16_t PanDecimator;

    int16_t SoundVersion;       // To detect the different sound file version (i.e more lfo dest for version >0)

    // Add 21.02.22
    int16_t Distortion;

    // Add 05.05.22
    char    GraWave[16];            // Name of the wav file to load
    int16_t GraBegin;               // Begin of the first grain in the file in  %
    int16_t GraBeginFine;           // Begin of the first fine tune
    int16_t GraSpace;               // Space between the grains
    int16_t GraSize;                // Size of a grain max GRA_MAX_SIZE
    int16_t GraDensity;             // Number of grain Max GRAIN_MAX 1 Mini
    int16_t GraSizeAttack;          // Attack  
    int16_t GraSizeSustain;         // Sustain
    int16_t GraOverlap;             // Space between 2 EG Grains in % of Gra_Size

    int16_t Reserved[13];
};


struct adsrT
{
    float a;
    float d;
    float s;
    float r;
    uint8_t loop;
    uint8_t trig;
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
    float spread;
    float panspread;

    float control_sign;
    float out_level;

    double 		d_speed;
	double 		d_cumulspeed;
	uint32_t	u32_speed;
	uint32_t	u32_cumulspeed;
    uint32_t	u32_cumulWhole;

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
    LFO_AMP,LFO_CUTOFF,LFO_PITCH,LFO_NOISE,LFO_A_PAN,LFO_WS1,LFOx_SPEED,LFOx_AMT,LFO_REVPAN,LFO_DELAYPAN,LFO_REVAMT,LFO_DELAYAMT
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


