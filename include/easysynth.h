#include <Arduino.h>

#ifdef __SYNTH__
#define SYNTH_EXTRN
#else
#define SYNTH_EXTRN extern
#endif


/*
 * Following defines can be changed for different puprposes
 */
#define MAX_POLY_VOICE	4                               /* max single voices, can use multiple osc */
#define OSC_PER_VOICE	3                               /* max single voices, can use multiple osc */
#define MAX_POLY_OSC	MAX_POLY_VOICE*OSC_PER_VOICE    /* osc polyphony, always active reduces single voices max poly */




/*
 * this is just a kind of magic to go through the waveforms
 * - WAVEFORM_BIT sets the bit length of the pre calculated waveforms
 */

#define WAVEFORM_BIT	10UL                            // 10 = 1024 sample 9 = 512 8 = 256 
#define WAVEFORM_CNT	(1<<WAVEFORM_BIT)
#define WAVEFORM_Q4		(1<<(WAVEFORM_BIT-2))
#define WAVEFORM_MSK	((1<<WAVEFORM_BIT)-1)
#define WAVEFORM_I(i)	((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK

/*
#define WAVEFORM_BIT	8UL                            // 10 = 1024 sample 9 = 512 8 = 256 
#define WAVEFORM_CNT	600
#define WAVEFORM_Q4		100
#define WAVEFORM_MSK	300
#define WAVEFORM_I(i)	((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK
*/


#define MIDI_NOTE_CNT 128
SYNTH_EXTRN uint32_t midi_note_to_add[MIDI_NOTE_CNT]; /* lookup to playback waveforms with correct frequency */

/*
 * set the correct count of available waveforms
 */
#define MAX_LABEL           5
#define WAVEFORM_TYPE_COUNT	7
#define DEST_TYPE_COUNT	    8

#define WAVE_SINE       0
#define WAVE_SAW        1
#define WAVE_SQUARE     2
#define WAVE_PULSE      3
#define WAVE_TRI        4
#define WAVE_NOISE      5
#define WAVE_SILENCE    6

/*
 * add here your waveforms
 */
#ifdef __SYNTH__
float sine[WAVEFORM_CNT];
float saw[WAVEFORM_CNT];
float square[WAVEFORM_CNT];
float pulse[WAVEFORM_CNT];
float tri[WAVEFORM_CNT];
float noise[WAVEFORM_CNT];
float silence[WAVEFORM_CNT];
float wavework[WAVEFORM_CNT];
float wavetrash[WAVEFORM_CNT];


uint8_t selWaveForm1=0;
uint8_t selWaveForm2=0;

uint8_t globalrank=0;

float oscdetune=0;

char Wave_Name[WAVEFORM_TYPE_COUNT][MAX_LABEL] = 
{"SIN","SAW","SQU","PUL","TRI","NOI","NOT"};

char Dest_Name[DEST_TYPE_COUNT][MAX_LABEL] = 
{"AMP","CUT","PIT","NOI","PAN","WS1","LxS","LxA"};

/*
 * do not forget to enter the waveform pointer addresses here
 */
float *waveFormLookUp[WAVEFORM_TYPE_COUNT] = {&sine[0], &saw[0], &square[0], &pulse[0], &tri[0], &noise[0], &silence[0]};
float *selectedWaveForm =  &tri[0];
float *selectedWaveForm2 = &sine[0];
uint32_t osc_act = 0;
uint32_t voc_act = 0;
struct adsrT adsr_vol = {0.04f, 0.03f, 1.0f, 0.04f};
struct adsrT adsr_fil = {1.0f, 0.25f, 1.0f, 0.01f};
struct adsrT adsr_pit = {0.0f, 0.0f, 0.0f, 0.0f};

float filtCutoff = 1.0f;
float filtReso = 0.5f;
float filterEG=0;
float NoiseLevel = 0.0f;
float WaveShapping1 = 0.0f;


float PitchMod = 0;
float NoiseMod = 0;
float FiltCutoffMod = 0;
float PanMod=0;
float AmpMod=0;
float Lfo1SpeedMod=0;
float Lfo1AmtMod=0;
float Lfo2SpeedMod=0;
float Lfo2AmtMod=0;
float WaveShapping1Mod=0;
float OldWaveShapping1Mod=0;

float MixOsc = 1;
float MixSub = 1;
int8_t SubTranspose = -12;

#else

extern char Wave_Name[WAVEFORM_TYPE_COUNT][MAX_LABEL];
extern char Dest_Name[DEST_TYPE_COUNT][MAX_LABEL];

extern float sine[WAVEFORM_CNT];
extern float saw[WAVEFORM_CNT];
extern float square[WAVEFORM_CNT];
extern float pulse[WAVEFORM_CNT];
extern float tri[WAVEFORM_CNT];
extern float noise[WAVEFORM_CNT];
extern float silence[WAVEFORM_CNT];
extern float wavework[WAVEFORM_CNT];
extern float wavetrash[WAVEFORM_CNT];

extern uint8_t selWaveForm1;
extern uint8_t selWaveForm2;

extern uint8_t globalrank;

extern float oscdetune;

/*
 * do not forget to enter the waveform pointer addresses here
 */
extern float *waveFormLookUp[WAVEFORM_TYPE_COUNT];
extern float *selectedWaveForm;
extern float *selectedWaveForm2;
extern uint32_t osc_act;
extern uint32_t voc_act;
extern struct adsrT adsr_vol;
extern struct adsrT adsr_fil;
extern struct adsrT adsr_pit;
extern uint32_t osc_act;
extern struct stLfo Lfo1;
extern struct stLfo Lfo2;

extern float filtCutoff;
extern float filtReso;
extern float filterEG;

extern float NoiseLevel;
extern float WaveShapping1;
extern float OldWaveShapping1Mod;

extern float PitchMod;
extern float NoiseMod;
extern float FiltCutoffMod;
extern float PanMod;
extern float AmpMod;
extern float Lfo1SpeedMod;
extern float Lfo1AmtMod;
extern float Lfo2SpeedMod;
extern float Lfo2AmtMod;
extern float WaveShapping1Mod;

extern int8_t SubTranspose;

extern float MixOsc;
extern float MixSub;

#endif




SYNTH_EXTRN struct filterCoeffT filterGlobalC;
SYNTH_EXTRN struct filterProcT mainFilterL, mainFilterR;

SYNTH_EXTRN float voiceSink[2];
SYNTH_EXTRN struct oscillatorT oscPlayer[MAX_POLY_OSC];

SYNTH_EXTRN struct notePlayerT voicePlayer[MAX_POLY_VOICE];

SYNTH_EXTRN void Synth_Init();
SYNTH_EXTRN void Filter_Calculate(float c, float reso, struct filterCoeffT *const  filterC);
SYNTH_EXTRN inline void Filter_Process(float *const signal, struct filterProcT *const filterP);
SYNTH_EXTRN inline bool ADSR_Process(const struct adsrT *ctrl, float *ctrlSig, adsr_phaseT *phase);
SYNTH_EXTRN void Voice_Off(uint32_t i);
SYNTH_EXTRN void Synth_Process(float *left, float *right);
SYNTH_EXTRN struct oscillatorT *getFreeOsc();
SYNTH_EXTRN struct notePlayerT *getFreeVoice();
SYNTH_EXTRN inline void Filter_Reset(struct filterProcT *filter);
SYNTH_EXTRN void Synth_NoteOn(uint8_t note);
SYNTH_EXTRN void Synth_NoteOff(uint8_t note);
SYNTH_EXTRN int Synth_SetRotary(uint8_t rotary, int val);

