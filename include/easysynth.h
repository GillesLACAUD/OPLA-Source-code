#include <Arduino.h>

#ifdef __SYNTH__
#define SYNTH_EXTRN
#else
#define SYNTH_EXTRN extern
#endif


#define MIDI_NOTE_CNT 128
SYNTH_EXTRN uint32_t midi_note_to_add[MIDI_NOTE_CNT]; /* lookup to playback waveforms with correct frequency */

/*
 * set the correct count of available waveforms
 */
#define MAX_LABEL           5
#define DEST_TYPE_COUNT	    12

//----------------------------------------------------------
// SOUND MODE AND POLY DEFINE
//----------------------------------------------------------
#define MAX_POLY_VOICE	8                               	/* max single voices, can use multiple osc */
															/* 8 or more for para mode				   */

#define OSC_PER_VOICE	3                               	/* max single voices, can use multiple osc */
#define MAX_POLY_OSC	(MAX_POLY_VOICE)*(OSC_PER_VOICE)    /* osc polyphony, always active reduces single voices max poly */

#define MAX_SND_MODE		3

#define SND_MODE_POLY		0
#define SND_MODE_PARA		1
#define SND_MODE_MONO		2

#define SND_MAX_POLY		4
#define SND_MAX_PARA		4			// 5 without delay and with reverb
#define SND_MAX_MONO		4			// 5 without delay and with reverb

//----------------------------------------------------------
// FILTER DEFINE
//----------------------------------------------------------

#define FILTER_1		// hard resonance at hight level
//#define FILTER_7		// USE THIS ONE ok for poly but hard sound at the edge



//#define FILTER_8		// 12 dB
//#define FILTER_2		// No resonance ???
//#define FILTER_3		// Crash direct
//#define FILTER_4		// Poly ok but had sound at the edge like FILTER7 but less less rapid ass filter7

#define MAX_FLT_TYPE		4*2

// 24 db/Oct
#define FILTER_2LP			0
#define FILTER_2HP			1
#define FILTER_2BP			2
#define FILTER_2NP			3
// 12 db/Oct
#define FILTER_1LP			4
#define FILTER_1HP			5
#define FILTER_1BP			6
#define FILTER_1NP			7

//----------------------------------------------------------
// TUNE DEFINE
//----------------------------------------------------------
#define TUNE_SUB            0
#define TUNE_OSC            1
#define TUNE_TRANSPOSE      2

//----------------------------------------------------------
// WAVEFORM DEFINE
//----------------------------------------------------------

/*
 * this is just a kind of magic to go through the waveforms
 * - WAVEFORM_BIT sets the bit length of the pre calculated waveforms
 */

#define WAVEFORM_BIT	10UL                            // 10 = 1024 sample 9 = 512 8 = 256 
#define WAVEFORM_CNT	(1<<WAVEFORM_BIT)
#define WAVEFORM_Q2		(1<<(WAVEFORM_BIT-1))
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


#define WAVEFORM_TYPE_COUNT	8
#define WAVEFORM_SUB_COUNT	7

#define WAVE_SINE       0
#define WAVE_SAW        1
#define WAVE_SQUARE     2
#define WAVE_PULSE      3
#define WAVE_TRI        4
#define WAVE_NOISE      5
#define WAVE_SILENCE    6
#define WAVE_AKWF	    7

//----------------------------------------------------------
// NOISE DEFINE
//----------------------------------------------------------
#define NOISE_TYPE_COUNT	2

#define NOISE_PRE       0
#define NOISE_POST      1

//----------------------------------------------------------
// END DEFINE
//----------------------------------------------------------

SYNTH_EXTRN int16_t		MidiRx;

SYNTH_EXTRN WorkSound	WS;

SYNTH_EXTRN uint8_t CurrentSound;

SYNTH_EXTRN uint8_t SoundMode;
SYNTH_EXTRN uint8_t FilterType;

SYNTH_EXTRN uint8_t NoiseType;

SYNTH_EXTRN uint8_t FlipPan;

SYNTH_EXTRN uint8_t u8_ArpOn;
SYNTH_EXTRN uint8_t u8_ArpHold;
SYNTH_EXTRN uint8_t u8_ArpDiv;
SYNTH_EXTRN uint8_t u8_ArpMode;


#define MONO_MAX_KEEP_NOTE	20
SYNTH_EXTRN uint8_t MonoIndexNote;
SYNTH_EXTRN uint8_t MonoCptNote;
SYNTH_EXTRN uint8_t MonoKeepNote[MONO_MAX_KEEP_NOTE];
SYNTH_EXTRN uint8_t MonoKeepVel[MONO_MAX_KEEP_NOTE];


#define MAXARPDIV	8
#define MAXARPMODE	8	

/*
 * add here your waveforms
 */
#ifdef __SYNTH__
uint8_t oldCurrentSound=250;

char Noise_Name[NOISE_TYPE_COUNT][5] = 
{"PRE","POS"};

char YesNo[2][5] = 
{"NO","YES"};

char ArpDiv[MAXARPDIV][5] = 
{"04","08","16","32","04T","08T","16T","32T"};
char ArpMode[MAXARPMODE][5] = 
{"UP","DWN","INC","EXC","RND","ORD","UP2","DW2"};


float sine[WAVEFORM_CNT];
float saw[WAVEFORM_CNT];
float square[WAVEFORM_CNT];
float pulse[WAVEFORM_CNT];
float tri[WAVEFORM_CNT];
float noise[WAVEFORM_CNT];
float silence[WAVEFORM_CNT];

float wavework[WAVEFORM_CNT];
float waveAKWF[WAVEFORM_CNT];

uint8_t selWaveForm1=0;
uint8_t selWaveForm2=0;

uint8_t globalrank=0;

float oscdetune=0;
float subdetune=0;

char Filter_Type[MAX_FLT_TYPE][MAX_LABEL] =
{"4LP","4HP","4BP","4NP","2LP","2HP","2BP","2NP"};

char Sound_Mode[MAX_SND_MODE][MAX_LABEL] = 
{"POL","PAR","MON"};

char Wave_Name[WAVEFORM_TYPE_COUNT][MAX_LABEL] = 
{"SIN","SAW","SQU","PUL","TRI","NOI","NOT","AKW"};

char Wave_SubName[WAVEFORM_SUB_COUNT][MAX_LABEL] = 
{"SIN","SAW","SQU","PUL","TRI","NOI","NOT"};

char Dest_Name[DEST_TYPE_COUNT][MAX_LABEL] = 
{"AMP","CUT","PIT","NOI","PAN","WS1","LxS","LxA","RPA","DPA","RAM","DAM"};

/*
 * do not forget to enter the waveform pointer addresses here
 */
float *waveFormLookUp[WAVEFORM_TYPE_COUNT] = {&sine[0], &saw[0], &square[0], &pulse[0], &tri[0], &noise[0], &silence[0],&waveAKWF[0]};
float *selectedWaveForm =  &waveAKWF[0];
float *selectedWaveForm2 = &sine[0];
uint32_t osc_act = 0;
uint32_t voc_act = 0;
struct adsrT adsr_vol = {0.04f, 0.03f, 1.0f, 0.04f};
struct adsrT adsr_fil = {0.25f, 0.10f, 1.0f, 0.01f};
struct adsrT adsr_pit = {0.0f, 0.0f, 0.0f, 0.0f};

float filtCutoff = 1.0f;
float filtReso = 0.5f;
float soundFiltReso = 0.5f;
float filterEG=0;
float filterKBtrack=0;
float NoiseLevel = 0.0f;
float WaveShapping1 = 0.0f;
float WaveShapping2 = 0.0f;

float pitchEG=0;
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
float WaveShapping2Mod=0;
float OldWaveShapping2Mod=0;


float MixOsc = 1;
float MixSub = 1;
float SubTranspose = -12.0;

float pitchBendValue = 0.0f;
float pitchMultiplier = 1.0f;

float AmpVel=0.5;
float FilterVel=1.0;
float GeneralVolume=1.0;

#else

extern char Noise_Name[NOISE_TYPE_COUNT][5];
extern char YesNo[2][5];
extern char Filter_Type[MAX_FLT_TYPE][MAX_LABEL];
extern char Wave_Name[WAVEFORM_TYPE_COUNT][MAX_LABEL];
extern char Wave_SubName[WAVEFORM_SUB_COUNT][MAX_LABEL];
extern char Dest_Name[DEST_TYPE_COUNT][MAX_LABEL];
extern char Sound_Mode[3][MAX_LABEL];
extern char ArpDiv[MAXARPDIV][5]; 
extern char ArpMode[MAXARPMODE][5];

extern float sine[WAVEFORM_CNT];
extern float saw[WAVEFORM_CNT];
extern float square[WAVEFORM_CNT];
extern float pulse[WAVEFORM_CNT];
extern float tri[WAVEFORM_CNT];
extern float noise[WAVEFORM_CNT];
extern float silence[WAVEFORM_CNT];
extern float waveAKWF[WAVEFORM_CNT];

extern float wavework[WAVEFORM_CNT];




extern uint8_t selWaveForm1;
extern uint8_t selWaveForm2;

extern uint8_t globalrank;

extern float oscdetune;
extern float subdetune;

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
extern float soundFiltReso;
extern float filtReso;
extern float filterEG;
extern float filterKBtrack;

extern float NoiseLevel;
extern float WaveShapping1;
extern float OldWaveShapping1Mod;
extern float WaveShapping2;
extern float OldWaveShapping2Mod;


extern float pitchEG;
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
extern float WaveShapping2Mod;


extern float SubTranspose;

extern float MixOsc;
extern float MixSub;

extern float pitchBendValue;
extern float pitchMultiplier;

extern float AmpVel;
extern float FilterVel;

extern float GeneralVolume;

extern uint8_t oldCurrentSound;


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
SYNTH_EXTRN void Synth_NoteOn(uint8_t note,uint8_t vel);
SYNTH_EXTRN void Synth_NoteOff(uint8_t note);
SYNTH_EXTRN void Synth_MonoNoteOn(uint8_t note,uint8_t vel);
SYNTH_EXTRN void Synth_MonoNoteOff(uint8_t note);

SYNTH_EXTRN int Synth_SetRotary(uint8_t rotary, int val);

SYNTH_EXTRN void Update_Tune(uint8_t type);

#ifdef FILTER_5
SYNTH_EXTRN void Filter_Calculate(float c, float reso, struct filterCoeffT *const  filterC);
#endif

