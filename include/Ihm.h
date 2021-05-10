#include <Arduino.h>

#ifdef __IHM__
#define IHM_EXTRN
#else
#define IHM_EXTRN   extern 
#endif

#include "midi_interface.h"
#include "Lfo.h"

#define TYPE_DATA	0
#define TYPE_LIST	1

IHM_EXTRN   uint8_t gui_Section;
IHM_EXTRN   uint8_t gui_Param;
IHM_EXTRN   uint8_t gui_Value;

IHM_EXTRN   char TabListNull[0][0];
IHM_EXTRN   int           IntNull;
#ifdef __IHM__
int FctNull(int val)
{
  return val;
}
#else
extern int FctNull(int val);
#endif

typedef struct
{
  char 		    Name[5];
  char 		    LgName[20];
  uint8_t	    MidiCC;
  uint8_t		  Type;
  char	      *ptTabList;	    // pt to the list
  char		    Index;			    // Nb item in list
  int*		    Data;
  uint8_t		  SizeData;
  int 		    MinData;
  int 		    MaxData;
  int 		    Step;
  int 		    (*ptrfunctValueChange) (int);
  int 		    (*ptrfunctBPOn) (int);
  int 		    (*ptrfunctBPOff) (int);
  int 		    (*ptrfunctBPHold) (int);
  int 		    (*ptrfunctBPDoubleClick) (int);
} Encoder_Data;

#define MAX_SECTION     5
#define MAX_ENCODER     10

#define SECTION_OSC     0
#define SECTION_FILTER  1
#define SECTION_EG      2
#define SECTION_LFO     3
#define SECTION_FX      4

#ifdef __IHM__
char Tab_Section_Name[MAX_SECTION][15]=
{
    "OSCILLATOR",
    "FILTER",
    "ENV GENERATOR",
    "LFO",
    "FX",
};

int Fct_Ch_OscWave(int val);
int Fct_Ch_SubWave(int val);
int Fct_Ch_Noise(int val); 
int Fct_Ch_Detune(int val);
int Fct_Ch_WS1(int val);   
int Fct_Ch_OscMix(int val);
int Fct_Ch_SubMix(int val);
int Fct_Ch_SubOct(int val);

int Fct_Ch_Cutoff(int val);  
int Fct_Ch_Resonance(int val);
int Fct_Ch_KbTrack(int val); 
int Fct_Ch_FlAttack(int val);
int Fct_Ch_FlDecay(int val);
int Fct_Ch_FlRelease(int val);
int Fct_Ch_FlAmount(int val);

int Fct_Ch_AmAttack(int val);
int Fct_Ch_AmDecay(int val);
int Fct_Ch_AmSustain(int val);
int Fct_Ch_AmRelease(int val);

int Fct_Ch_PiAttack(int val);
int Fct_Ch_PiDecay(int val);
int Fct_Ch_PiRelease(int val);
int Fct_Ch_PiAmount(int val);

int Fct_Ch_L1Speed(int val);
int Fct_Ch_L1Shape(int val);
int Fct_Ch_L1Dest(int val);
int Fct_Ch_L1Amount(int val);
int Fct_Ch_L2Speed(int val);
int Fct_Ch_L2Shape(int val);
int Fct_Ch_L2Dest(int val);  
int Fct_Ch_L2Amount(int val);

int Fct_Ch_DlLen(int val);
int Fct_Ch_DlAmount(int val);
int Fct_Ch_DlFeed(int val);

int Tab_EncoderVal[MAX_SECTION][MAX_ENCODER];
Encoder_Data    Tab_Encoder[MAX_SECTION][MAX_ENCODER]=
{
    // SECTION OSC
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE      SIZE MIN     MAX     STEP    CHANGE      ON          OFF         HOLD        DCLK  */
    "OSC",  "OSC WAVEFORM", MIDI_CC_WAVE1,  TYPE_LIST,  &Wave_Name[0][0],   0,      &IntNull,   1,  0,      WAVEFORM_TYPE_COUNT,    1,        Fct_Ch_OscWave,   FctNull,    FctNull,    FctNull,    FctNull,
    "SUB",  "SUB WAVEFORM", MIDI_CC_SUBOSC, TYPE_LIST,  &Wave_Name[0][0],   0,      &IntNull,   1,  0,      WAVEFORM_TYPE_COUNT,    1,        Fct_Ch_SubWave,   FctNull,    FctNull,    FctNull,    FctNull,
    "NOIS", "NOISE VOLUME", MIDI_CC_NOISE,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_Noise,     FctNull,    FctNull,    FctNull,    FctNull,
    "DET",  "DETUNE",       MIDI_CC_DETUNE, TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_Detune,    FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,          FctNull,    FctNull,    FctNull,    FctNull,

    "MIX",  "OSC VOLUME",   MIDI_CC_OSCVOL, TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_OscMix,    FctNull,    FctNull,    FctNull,    FctNull,
    "MIX",  "SUB VOLUME",   MIDI_CC_SUBVOL, TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_SubMix,    FctNull,    FctNull,    FctNull,    FctNull,
    "TRA",  "TRANSPOSE",    MIDI_CC_SUBTR,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  -24,    12,     1,      Fct_Ch_SubOct,    FctNull,    FctNull,    FctNull,    FctNull,
    "WS1",  "WAVE SHAPE1",  MIDI_CC_WS1,    TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_WS1,       FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,          FctNull,    FctNull,    FctNull,    FctNull,

    // SECTION FILTER
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE      SIZE MIN     MAX     STEP    CHANGE      ON          OFF         HOLD        DCLK  */
    "FC ",  "CUTOFF",       MIDI_CC_CUTOFF, TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_Cutoff,    FctNull,    FctNull,    FctNull,    FctNull,
    "RES",  "RESONANCE",    MIDI_CC_RES,    TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_Resonance, FctNull,    FctNull,    FctNull,    FctNull,
    "KBT",  "KB TRACK",     MIDI_CC_FOLLOW, TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_KbTrack,   FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,          FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,          FctNull,    FctNull,    FctNull,    FctNull,

    "ATT",  "FL ATTACK",    MIDI_CC_FLT_A,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_FlAttack,  FctNull,    FctNull,    FctNull,    FctNull,
    "DEC",  "FL DECAY",     MIDI_CC_FLT_D,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_FlDecay,   FctNull,    FctNull,    FctNull,    FctNull,
    "REL",  "FL RELEASE",   MIDI_CC_FLT_R,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_FlRelease, FctNull,    FctNull,    FctNull,    FctNull,
    "AMT",  "FL AMOUNT",    MIDI_CC_FLT_Q,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_FlAmount,  FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,          FctNull,    FctNull,    FctNull,    FctNull,

    // SECTION EG 
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE      SIZE MIN     MAX     STEP    CHANGE      ON          OFF         HOLD        DCLK  */
    "ATT",  "AM ATTACK",    MIDI_CC_AMP_A,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_AmAttack,   FctNull,    FctNull,    FctNull,    FctNull,
    "DEC",  "AM DECAY",     MIDI_CC_AMP_D,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_AmDecay,    FctNull,    FctNull,    FctNull,    FctNull,
    "SUS",  "AM SUSTAIN",   MIDI_CC_AMP_S,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_AmSustain,  FctNull,    FctNull,    FctNull,    FctNull,
    "REL",  "AM RELEASE",   MIDI_CC_AMP_R,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_AmRelease,  FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,           FctNull,    FctNull,    FctNull,    FctNull,

    "ATT",  "PI ATTACK",    MIDI_CC_PITC_A,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_PiAttack,  FctNull,    FctNull,    FctNull,    FctNull,
    "DEC",  "PI DECAY",     MIDI_CC_PITC_D,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_PiDecay,   FctNull,    FctNull,    FctNull,    FctNull,
    "REL",  "PI RELEASE",   MIDI_CC_PITC_R,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_PiRelease, FctNull,    FctNull,    FctNull,    FctNull,
    "AMT",  "PI AMOUNT",    MIDI_CC_PITC_Q,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_PiAmount,  FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,            TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,          FctNull,    FctNull,    FctNull,    FctNull,

    // SECTION LFO
    /* Name                 MIDICC               TYPE        LIST                INDEX   VALUE      SIZE MIN     MAX     STEP    CHANGE             ON          OFF         HOLD        DCLK  */
    "SPE",  "LFO1 SPEED",   MIDI_CC_LFO1_SPEED,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_L1Speed,    FctNull,    FctNull,    FctNull,    FctNull,
    "SHA",  "LFO1 SHAPE",   MIDI_CC_LFO1_SHAPE,  TYPE_LIST,  &Wave_LfoName[0][0],0,      &IntNull,   1,  0,      WAVE_LFO_COUNT,    1,      Fct_Ch_L1Shape,    FctNull,    FctNull,    FctNull,    FctNull,
    "DES",  "LFO1 DEST",    MIDI_CC_LFO1_DEST,   TYPE_LIST,  &Dest_Name[0][0],   0,      &IntNull,   1,  0,      DEST_TYPE_COUNT,    1,      Fct_Ch_L1Dest,     FctNull,    FctNull,    FctNull,    FctNull,
    "AMT",  "LFO1 AMOUNT",  MIDI_CC_LFO1_AMT,    TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_L1Amount,   FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,           FctNull,    FctNull,    FctNull,    FctNull,

    "SPE",  "LFO2 SPEED",   MIDI_CC_LFO2_SPEED,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_L2Speed,    FctNull,    FctNull,    FctNull,    FctNull,
    "SHA",  "LFO2 SHAPE",   MIDI_CC_LFO2_SHAPE,  TYPE_LIST,  &Wave_LfoName[0][0],0,      &IntNull,   1,  0,      WAVE_LFO_COUNT,    1,      Fct_Ch_L2Shape,    FctNull,    FctNull,    FctNull,    FctNull,
    "DES",  "LFO2 DEST",    MIDI_CC_LFO2_DEST,   TYPE_LIST,  &Dest_Name[0][0],   0,      &IntNull,   1,  0,      DEST_TYPE_COUNT,    1,      Fct_Ch_L2Dest,     FctNull,    FctNull,    FctNull,    FctNull,
    "AMT",  "LFO2 AMOUNT",  MIDI_CC_LFO2_AMT,    TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_L2Amount,   FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,           FctNull,    FctNull,    FctNull,    FctNull,

    // SECTION FG 
    /* Name                 MIDICC                 TYPE        LIST                INDEX   VALUE      SIZE MIN     MAX     STEP    CHANGE           ON          OFF         HOLD        DCLK  */
    "LEN",  "DELAY LEN",    MIDI_CC_DEL_LENGHT,    TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_DlLen,    FctNull,    FctNull,    FctNull,    FctNull,
    "AMT",  "DELAY AMOUNT", MIDI_CC_DEL_LEVEL,     TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_DlAmount, FctNull,    FctNull,    FctNull,    FctNull,
    "FEE",  "DELAY FEEBACK",MIDI_CC_DEL_FEEDBACK,  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      Fct_Ch_DlFeed,   FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,

    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,   1,  0,      127,    1,      FctNull,         FctNull,    FctNull,    FctNull,    FctNull,

};
#else
IHM_EXTRN int Tab_EncoderVal[MAX_SECTION][MAX_ENCODER];
IHM_EXTRN char Tab_Section_Name[MAX_SECTION][15];
IHM_EXTRN Encoder_Data    Tab_Encoder[MAX_SECTION][MAX_ENCODER];
#endif


