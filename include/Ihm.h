#include <Arduino.h>

#ifdef __IHM__
#define IHM_EXTRN
#else
#define IHM_EXTRN   extern 
#endif

#include "midi_interface.h"
#include "Lfo.h"
#include "Modulator.h"

#define TYPE_DATA	0
#define TYPE_LIST	1

IHM_EXTRN unsigned char Midi_KeyOn;

IHM_EXTRN   uint8_t gui_Section;
IHM_EXTRN   uint8_t gui_Param;
IHM_EXTRN   uint8_t gui_Value;

IHM_EXTRN   int8_t  SoundNameInc10;

IHM_EXTRN   char TabListNull[0][0];
IHM_EXTRN   int16_t IntNull;
IHM_EXTRN   int16_t IntCalib;
IHM_EXTRN   int16_t IntAudioIn;
IHM_EXTRN   int16_t StopAudioOut;

IHM_EXTRN   uint8_t gui_WaveBank;
IHM_EXTRN   uint8_t gui_WaveNumber;

IHM_EXTRN   int16_t WSMidiRx;

#ifdef __IHM__
int FctNull(int val)
{
  IntNull=0;
  val=0;
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
  int16_t*	  Data;
  uint8_t		  SizeData;
  int 		    MinData;
  int 		    MaxData;
  int 		    Step;
  int 		    (*ptrfunctValueChange) (int);
} Encoder_Data;

#define MAX_SECTION     7+1+1     // Add +1 for the arpegiator 29.11.21 +1 Add Midi
#define MAX_ENCODER     10

#define SECTION_OSC     0
#define SECTION_NOISE   1
#define SECTION_FILTER  2
#define SECTION_EG      3
#define SECTION_LFO     4
#define SECTION_FX      5
#define SECTION_ARP     6
#define SECTION_SYSTEM  7
#define SECTION_MIDI    8

#ifdef __IHM__
char Tab_Section_Name[MAX_SECTION][20]=
{
    "OSCILLATOR",
    "NOISE-LOOP-TRIG",
    "FILTER",
    "ENV GENERATOR",
    "LFO",
    "FX",
    "ARP",
    "SYSTEM",
    "MIDI-MISC",
    
};



int Fct_Ch_OscWave(int val);
int Fct_Ch_OscMix(int val);
int Fct_Ch_Detune(int val);
int Fct_Ch_WS1(int val);   
int Fct_Ch_WS2(int val);   

int Fct_Ch_SubWave(int val);
int Fct_Ch_SubMix(int val);
int Fct_Ch_SubOct(int val);
int Fct_Ch_SubDetune(int val);

int Fct_Ch_NoiseType(int val);
int Fct_Ch_Noise(int val); 
int Fct_Ch_Bank(int val); 
int Fct_Ch_Wave(int val); 

int Fct_Ch_Cutoff(int val);  
int Fct_Ch_Resonance(int val);
int Fct_Ch_KbTrack(int val); 
int Fct_Ch_FVelo(int val); 
int Fct_Ch_FType(int val); 
int Fct_Ch_FlAttack(int val);
int Fct_Ch_FlDecay(int val);
int Fct_Ch_FlRelease(int val);
int Fct_Ch_FlAmount(int val);

int Fct_Ch_AmAttack(int val);
int Fct_Ch_AmDecay(int val);
int Fct_Ch_AmSustain(int val);
int Fct_Ch_AmRelease(int val);
int Fct_Ch_AmVelo(int val);

int Fct_Ch_PiAttack(int val);
int Fct_Ch_PiDecay(int val);
int Fct_Ch_PiRelease(int val);
int Fct_Ch_PiAmount(int val);
int Fct_Ch_Portamento(int val);

int Fct_Ch_L1Speed(int val);
int Fct_Ch_L1Shape(int val);
int Fct_Ch_L1Dest(int val);
int Fct_Ch_L1Amount(int val);
int Fct_Ch_L1Sync(int val);
int Fct_Ch_L2Speed(int val);
int Fct_Ch_L2Shape(int val);
int Fct_Ch_L2Dest(int val);  
int Fct_Ch_L2Amount(int val);
int Fct_Ch_L2Sync(int val);

int Fct_Ch_DlLen(int val);
int Fct_Ch_DlAmount(int val);
int Fct_Ch_DlFeed(int val);
int Fct_Ch_DlPP(int val);
int Fct_Ch_Reverb(int val);
int Fct_Ch_RevPan(int val);

int Fct_Ch_SoundMode(int val);
int Fct_Ch_PBRange(int val);
int Fct_Ch_MDDest(int val);
int Fct_Ch_MDAmt(int val);
int Fct_Ch_ATDest(int val);
int Fct_Ch_ATAmt(int val);
int Fct_Ch_MidiRx(int val);
int Fct_Ch_Spread(int val);

int Fct_Ch_Transpose(int val);
int Fct_Ch_SVolume(int val);

int Fct_Ch_PanSpread(int val);

int Fct_Ch_FilterLoop(int val);
int Fct_Ch_AmpLoop(int val);
int Fct_Ch_PitchLoop(int val);

int Fct_Ch_FilterTrig(int val);
int Fct_Ch_AmpTrig(int val);
int Fct_Ch_PitchTrig(int val);

int Fct_Ch_Calib(int val);
int Fct_Ch_AudioIn(int val);

int Fct_Ch_ArpOnOff(int val);
int Fct_Ch_ArpHold(int val); 
int Fct_Ch_ArpSpeed(int val);
int Fct_Ch_ArpDiv(int val);  
int Fct_Ch_ArpMode(int val); 
int Fct_Ch_ArpOct(int val);  
int Fct_Ch_ArpGate(int val); 
int Fct_Ch_ArpSwing(int val);

int Fct_Ch_MidiMode(int val); 
int Fct_Ch_MidiRelCC(int val);
int Fct_Ch_MidiRelMin(int val);
int Fct_Ch_MidiRelMax(int val);
int Fct_Ch_AudioIn(int val);

int Fct_Ch_PanDecimator(int val);
int Fct_Ch_Decimator(int val);
int Fct_Ch_WDDecimator(int val);

int Fct_Ch_Distortion(int val);

int Fct_Ch_GraBegin(int val);
int Fct_Ch_GraFine(int val);
int Fct_Ch_GraSpace(int val);
int Fct_Ch_GraSize(int val);
int Fct_Ch_GraDensity(int val);
int Fct_Ch_GraAttack(int val);
int Fct_Ch_GraSustain(int val);
int Fct_Ch_GraOverlap(int val);

// To change the max for the AKWF selection -> Tab_Encoder[SECTION_BANK_MAX][POT_BANK_MAX]
#define SECTION_BANK_MAX    1
#define POT_BANK_MAX        6 

Encoder_Data    Tab_Encoder[MAX_SECTION][MAX_ENCODER]=
{
    // SECTION OSC
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE  */           
    //"WAV",  "WAVE FILE",      MIDI_CC_WAVE1,  TYPE_LIST,  &TabListNull[0][0], 0,    &IntNull,         1,    0,      127,1,                    Fct_Ch_OscWave,       
    "BEG",  "GRAIN BEGIN",    MIDI_CC_OSCVOL, TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraBegin,       1,    0,      100,1,                    Fct_Ch_GraBegin,        
    "FIN",  "GRAIN FINE",     MIDI_CC_DETUNE, TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraBeginFine,   1,    0,      127,1,                    Fct_Ch_GraFine,        
    "SPA",  "GRAIN SPACE",    MIDI_CC_WS1,    TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraSpace,       1,    0,      127,1,                    Fct_Ch_GraSpace,           
    "SIZ",  "GRAIN SIZE",     MIDI_CC_WAVE1,  TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraSize,        1,    1,      100,1,                    Fct_Ch_GraSize,           
    "DEN",  "GRAIN DENSITY",  MIDI_CC_WS2,    TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraSize,        1,    1,      10 ,1,                    Fct_Ch_GraDensity,              

    "ATT",  "GRAIN ATTACK",   MIDI_CC_SUBOSC, TYPE_LIST,  &Wave_SubName[0][0],0,    &WS.GraSizeAttack,  1,    0,      127,1,                    Fct_Ch_GraAttack,       
    "SUS",  "GRAIN SUSTAIN",  MIDI_CC_SUBVOL, TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraSizeSustain, 1,    0,      127,1,                    Fct_Ch_GraSustain,        
    "OV",   "GRAIN OVERLAP",  MIDI_CC_SUBDET, TYPE_DATA,  &TabListNull[0][0], 0,    &WS.GraOverlap,     1,    1,      100,1,                    Fct_Ch_GraOverlap,     
    "---",  "---",            0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,    &IntNull,           1,    0,      127,1,                    FctNull,              
    "---",  "---",            0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,    &IntNull,           1,    0,      127,1,                    FctNull,              
    
    "NOIS", "NOISE TYPE",   MIDI_CC_NTYPE,  TYPE_LIST,  &Noise_Name[0][0],  0,      &WS.NoiseType,    1,    0,      127,NOISE_TYPE_COUNT,     Fct_Ch_NoiseType,     
    "MIX",  "NOISE VOLUME", MIDI_CC_NOISE,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.NoiseLevel,   1,    0,      127,1,                    Fct_Ch_Noise,         
    "FLP",  "FILTER LOOP",  MIDI_CC_82,     TYPE_LIST,  &YesNo[0][0],       0,      &WS.FilterLoop,   1,    0,      127,2,                    Fct_Ch_FilterLoop,    
    "ALP",  "AMP LOOP",     MIDI_CC_83,     TYPE_LIST,  &YesNo[0][0],       0,      &WS.AmpLoop,      1,    0,      127,2,                    Fct_Ch_AmpLoop,       
    "PLP",  "PITCH LOOP",   MIDI_CC_84,     TYPE_LIST,  &YesNo[0][0],       0,      &WS.PitchLoop,    1,    0,      127,2,                    Fct_Ch_PitchLoop,     

    "BAK",  "AKWF BANK",    MIDI_CC_BK,     TYPE_DATA,  &TabListNull[0][0], 0,      &WS.OscBank,      1,    0,      63,1,                     Fct_Ch_Bank,          
    "WAV",  "AKWF WAVE",    MIDI_CC_WA,     TYPE_DATA,  &TabListNull[0][0], 0,      &WS.AKWFWave,     1,    0,      99,1,                     Fct_Ch_Wave,          
    "FTR",  "FILTER TRIG",  MIDI_CC_87,     TYPE_LIST,  &YesNo[0][0],       0,      &WS.FilterTrig,   1,    0,      127,2,                    Fct_Ch_FilterTrig,    
    "ATR",  "AMP TRIG",     MIDI_CC_88,     TYPE_LIST,  &YesNo[0][0],       0,      &WS.AmpTrig,      1,    0,      127,2,                    Fct_Ch_AmpTrig,       
    "PTR",  "PITCH TRIG",   MIDI_CC_89,     TYPE_LIST,  &YesNo[0][0],       0,      &WS.PitchTrig,    1,    0,      127,2,                    Fct_Ch_PitchTrig,     


    // SECTION FILTER
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE   */          
    "FC ",  "CUTOFF",       MIDI_CC_CUTOFF, TYPE_DATA,  &TabListNull[0][0], 0,      &WS.Cutoff,       1,    0,      127,1,                    Fct_Ch_Cutoff,        
    "RES",  "RESONANCE",    MIDI_CC_RES,    TYPE_DATA,  &TabListNull[0][0], 0,      &WS.Resonance,    1,    0,      127,1,                    Fct_Ch_Resonance,     
    "KBT",  "KB TRACK",     MIDI_CC_FOLLOW, TYPE_DATA,  &TabListNull[0][0], 0,      &WS.KbTrack,      1,    0,      127,1,                    Fct_Ch_KbTrack,       
    "VEL",  "VEL FILTER",   MIDI_CC_FVELO,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.FVelo,        1,    0,      127,1,                    Fct_Ch_FVelo,         
    "TYP",  "FILTER TYPE",  MIDI_CC_FTYPE,  TYPE_LIST,  &Filter_Type[0][0], 0,      &WS.FType,        1,    0,      127,MAX_FLT_TYPE,         Fct_Ch_FType,         

    "ATT",  "FL ATTACK",    MIDI_CC_FLT_A,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.FEgAttack,    1,    0,      127,1,                    Fct_Ch_FlAttack,      
    "DEC",  "FL DECAY",     MIDI_CC_FLT_D,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.FEgDecay,     1,    0,      127,1,                    Fct_Ch_FlDecay,       
    "REL",  "FL RELEASE",   MIDI_CC_FLT_R,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.FEgRelease,   1,    0,      127,1,                    Fct_Ch_FlRelease,     
    "AMT",  "FL AMOUNT",    MIDI_CC_FLT_Q,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.FEgAmount,    1,    0,      127,1,                    Fct_Ch_FlAmount,      
    "---",  "---",          0xFF,           TYPE_DATA,  &TabListNull[0][0], 0,      &IntNull,         1,    0,      127,1,                    FctNull,              

    // SECTION EG 
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE     */       
    "ATT",  "AM ATTACK",    MIDI_CC_AMP_A,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.AEgAttack,    1,    0,      127,1,                    Fct_Ch_AmAttack,     
    "DEC",  "AM DECAY",     MIDI_CC_AMP_D,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.AEgDecay,     1,    0,      127,1,                    Fct_Ch_AmDecay,      
    "SUS",  "AM SUSTAIN",   MIDI_CC_AMP_S,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.AEgSustain,   1,    0,      127,1,                    Fct_Ch_AmSustain,    
    "REL",  "AM RELEASE",   MIDI_CC_AMP_R,  TYPE_DATA,  &TabListNull[0][0], 0,      &WS.AEgRelease,   1,    0,      127,1,                    Fct_Ch_AmRelease,    
    "VEL",  "VEL VOLUME",   MIDI_CC_AMPVEL, TYPE_DATA,  &TabListNull[0][0], 0,      &WS.AmpVelo,      1,    0,      127,1,                    Fct_Ch_AmVelo,       

    "ATT",  "PI ATTACK",    MIDI_CC_PITC_A,  TYPE_DATA,  &TabListNull[0][0], 0,     &WS.PEgAttack,    1,    0,      127,1,                    Fct_Ch_PiAttack,     
    "DEC",  "PI DECAY",     MIDI_CC_PITC_D,  TYPE_DATA,  &TabListNull[0][0], 0,     &WS.PEgDecay,     1,    0,      127,1,                    Fct_Ch_PiDecay,      
    "REL",  "PI RELEASE",   MIDI_CC_PITC_R,  TYPE_DATA,  &TabListNull[0][0], 0,     &WS.PEgRelease,   1,    0,      127,1,                    Fct_Ch_PiRelease,    
    "AMT",  "PI AMOUNT",    MIDI_CC_PITC_Q,  TYPE_DATA,  &TabListNull[0][0], 0,     &WS.PEgAmount,    1,    0,      127,1,                    Fct_Ch_PiAmount,     
    "POR",  "PORTAMENTO",   MIDI_CC_PORTA,   TYPE_DATA,  &TabListNull[0][0], 0,     &WS.Portamento,   1,    0,      127,1,                    Fct_Ch_Portamento,   


    // SECTION LFO
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE    */         
    "SPE",  "LFO1 SPEED",   MIDI_CC_LFO1_SPEED,  TYPE_DATA,  &TabListNull[0][0],  0,  &WS.LFO1Speed,   1,  0,       127,1,                    Fct_Ch_L1Speed,       
    "SHA",  "LFO1 SHAPE",   MIDI_CC_LFO1_SHAPE,  TYPE_LIST,  &Wave_LfoName[0][0], 0,  &WS.LFO1Shape,   1,  0,       127,WAVE_LFO_COUNT,       Fct_Ch_L1Shape,       
    "DES",  "LFO1 DEST",    MIDI_CC_LFO1_DEST,   TYPE_LIST,  &Dest_Name[0][0],    0,  &WS.LFO1Dest,    1,  0,       127,DEST_TYPE_COUNT,      Fct_Ch_L1Dest,        
    "AMT",  "LFO1 AMOUNT",  MIDI_CC_LFO1_AMT,    TYPE_DATA,  &TabListNull[0][0],  0,  &WS.LFO1Amount,  1,  0,       127,1,                    Fct_Ch_L1Amount,      
    "SYN",  "LFO1 SYNC",    MIDI_CC_LFO1_SYNC,   TYPE_LIST,  &Wave_LfoSync[0][0], 0,  &WS.LFO1Sync,    1,  0,       127,WAVE_LFO_SYNC,        Fct_Ch_L1Sync,        

    "SPE",  "LFO2 SPEED",   MIDI_CC_LFO2_SPEED,  TYPE_DATA,  &TabListNull[0][0],  0,  &WS.LFO2Speed,   1,  0,       127,1,                    Fct_Ch_L2Speed,       
    "SHA",  "LFO2 SHAPE",   MIDI_CC_LFO2_SHAPE,  TYPE_LIST,  &Wave_LfoName[0][0], 0,  &WS.LFO2Shape,   1,  0,       127,WAVE_LFO_COUNT,       Fct_Ch_L2Shape,       
    "DES",  "LFO2 DEST",    MIDI_CC_LFO2_DEST,   TYPE_LIST,  &Dest_Name[0][0],    0,  &WS.LFO2Dest,    1,  0,       127,DEST_TYPE_COUNT,      Fct_Ch_L2Dest,        
    "AMT",  "LFO2 AMOUNT",  MIDI_CC_LFO2_AMT,    TYPE_DATA,  &TabListNull[0][0],  0,  &WS.LFO2Amount,  1,  0,       127,1,                    Fct_Ch_L2Amount,      
    "SYN",  "LFO2 SYNC",    MIDI_CC_LFO2_SYNC,   TYPE_LIST,  &Wave_LfoSync[0][0], 0,  &WS.LFO2Sync,    1,  0,       127,WAVE_LFO_SYNC,        Fct_Ch_L2Sync,        

    // SECTION FX 
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE  */                     
    "LEN",  "DELAY LEN",    MIDI_CC_DEL_LENGHT,    TYPE_DATA,  &TabListNull[0][0], 0, &WS.DelayLen,     1,  0,      127,1,                    Fct_Ch_DlLen,         
    "AMT",  "DELAY AMOUNT", MIDI_CC_DEL_LEVEL,     TYPE_DATA,  &TabListNull[0][0], 0, &WS.DelayAmount,  1,  0,      127,1,                    Fct_Ch_DlAmount,      
    "FEE",  "DELAY FEEBACK",MIDI_CC_DEL_FEEDBACK,  TYPE_DATA,  &TabListNull[0][0], 0, &WS.DelayFeedback,1,  0,      127,1,                    Fct_Ch_DlFeed,        
    "PAN",  "DELAY PAN",    MIDI_CC_DEL_PP,        TYPE_DATA,  &TabListNull[0][0], 0, &WS.DelayPP,      1,  0,      127,1,                    Fct_Ch_DlPP,          
    "DIS",  "DISTORSION",   MIDI_CC_DIST,          TYPE_DATA,  &TabListNull[0][0], 0, &WS.Distortion,   1,  1,      127,1,                    Fct_Ch_Distortion,              

    "REV",  "REVERB LEVEL", MIDI_CC_REVERB_LEVEL,  TYPE_DATA,  &TabListNull[0][0], 0, &WS.ReverbLevel,  1,  0,      127,1,                    Fct_Ch_Reverb,        
    "PAN",  "REVERB PAN",   MIDI_CC_REVERB_PAN,    TYPE_DATA,  &TabListNull[0][0], 0, &WS.ReverbPan,    1,  0,      127,1,                    Fct_Ch_RevPan,        
    "DEC",  "DECIMATOR",    MIDI_CC_DECIMATOR,     TYPE_DATA,  &TabListNull[0][0], 0, &WS.Decimator,    1,  1,      12,1,                     Fct_Ch_Decimator,     
    "WDR",  "WET-DRY DEC",  MIDI_CC_WDDECIM,       TYPE_DATA,  &TabListNull[0][0], 0, &WS.WDDecimator,  1,  0,      100,1,                    Fct_Ch_WDDecimator,              
    "PAN",  "PAN DEC",      MIDI_CC_PANDECIM,      TYPE_DATA,  &TabListNull[0][0], 0, &WS.PanDecimator, 1,  0,      127,1,                    Fct_Ch_PanDecimator,              
    
  // SECTION ARPEGIATOR   

    /* Name                 MIDICC                  TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE    */          
    " ON",  "ARP ON/OFF",   MIDI_CC_ARP_ON,         TYPE_LIST,  &YesNo[0][0],        0,  &WS.ArpOnOff,        1,  0,      127,2,               Fct_Ch_ArpOnOff,   
    "HLD",  "ARP HOLD",     MIDI_CC_ARP_HLD,        TYPE_LIST,  &YesNo[0][0],        0,  &WS.ArpHold,         1,  0,      127,2,               Fct_Ch_ArpHold,    
    "SPE",  "ARP SPEED",    MIDI_CC_ARP_SPE,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.ArpSpeed,        1,  0,      127,1,               Fct_Ch_ArpSpeed,   
    "DIV",  "ARP DIV",      MIDI_CC_ARP_DIV,        TYPE_LIST,  &ArpDiv[0][0],       0,  &WS.ArpDiv,          1,  0,      127,MAXARPDIV,       Fct_Ch_ArpDiv,     
    "MOD",  "ARP MODE",     MIDI_CC_ARP_MOD,        TYPE_LIST,  &ArpMode[0][0],      0,  &WS.ArpMode,         1,  0,      127,MAXARPMODE,      Fct_Ch_ArpMode,    

    "---",  "ARP ---",      MIDI_CC_ARP_OCT,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.ArpOct,          1,  1,      3,  1,                Fct_Ch_ArpOct,     
    "GAT",  "ARP GATE",     MIDI_CC_ARP_GAT,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.ArpGate,         1,  0,      100,1,                Fct_Ch_ArpGate,    
    "SWI",  "ARP SWING",    MIDI_CC_ARP_SWI,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.ArpSwing,        1,  0,      127,1,                Fct_Ch_ArpSwing,   
    "---",  "ARP ---",      MIDI_CC_ARP_9,          TYPE_DATA,  &TabListNull[0][0],  0,  &IntNull,            1,  0,      127,1,                FctNull,          
    "---",  "ARP ---",      MIDI_CC_ARP_10,         TYPE_DATA,  &TabListNull[0][0],  0,  &IntNull,            1,  1,      16, 1,                FctNull,         

    // SECTION SYSTEM               
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE   */                    
    "MOD",  "SOUND MODE",   MIDI_CC_SOUND_MODE,    TYPE_LIST,  &Sound_Mode[0][0],   0,  &WS.SoundMode,  1,  0,      127,MAX_SND_MODE,           Fct_Ch_SoundMode,     
    "PBR",  "PB RANGE",     MIDI_CC_PB_RANGE,      TYPE_DATA,  &TabListNull[0][0],  0,  &WS.PBRange,    1,  0,      12, 1,                      Fct_Ch_PBRange,       
    "SPE",  "SPREAD",       MIDI_CC_SPREAD,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.Spread,     1,  0,      127,1,                      Fct_Ch_Spread,        
    "TRP",  "TRANSPOSE",    MIDI_CC_OCTAVE,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.Transpose,  1,  -24,    24, 1,                      Fct_Ch_Transpose,     
    "VOL",  "SOUND VOLUME", MIDI_CC_SVOLUME,       TYPE_DATA,  &TabListNull[0][0],  0,  &WS.SVolume,    1,  0,      127,1,                      Fct_Ch_SVolume,       

    "MWD",  "MW DEST",      MIDI_CC_MD_DEST,       TYPE_LIST,  &ModName[0][0],      0,  &WS.MWDest,     1,  0,      127,MOD_MAX,                Fct_Ch_MDDest,        
    "MWA",  "MW AMT",       MIDI_CC_MD_AMT,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.MWAmt,      1,  0,      127,1,                      Fct_Ch_MDAmt,         
    "AFD",  "AT DEST",      MIDI_CC_AT_DEST,       TYPE_LIST,  &ModName[0][0],      0,  &WS.ATDest,     1,  0,      127,MOD_MAX,                Fct_Ch_ATDest,        
    "AFA",  "AT AMT",       MIDI_CC_AT_AMT,        TYPE_DATA,  &TabListNull[0][0],  0,  &WS.ATAmt,      1,  0,      127,1,                      Fct_Ch_ATAmt,         
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0, &IntNull,         1,  0,      127,1,                      FctNull,              
    
    // SECTION MIDI               
    /* Name                 MIDICC          TYPE        LIST                INDEX   VALUE             SIZE  MIN     MAX                     STEP    CHANGE */                    
    "MRX",  "MIDI RX",      MIDI_CC_MIDI_RX,       TYPE_DATA,  &TabListNull[0][0],  0,  &WSMidiRx,      1,  1,      16,1,                       Fct_Ch_MidiRx,       
    "MOD",  "MIDI MODE",    MIDI_CC_MIDIMODE,      TYPE_LIST,  &Midi_Mode[0][0],    0,  &MidiMode,      1,  1,      127,MIDI_MODE_MAX,          Fct_Ch_MidiMode,     
    "RCC",  "RELATIC CC",   MIDI_CC_MIDIRELCC,     TYPE_DATA,  &TabListNull[0][0],  0,  &MidiRelCC,     1,  0,      127,1,                      Fct_Ch_MidiRelCC,    
    "MIN",  "RELATIV MIN",  MIDI_CC_MIDIRELMIN,    TYPE_DATA,  &TabListNull[0][0],  0,  &MidiRelMin,    1,  0,      127,1,                      Fct_Ch_MidiRelMin,   
    "MAX",  "RELATIV MAX",  MIDI_CC_MIDIRELMAX,    TYPE_DATA,  &TabListNull[0][0],  0,  &MidiRelMax,    1,  0,      127,1,                      Fct_Ch_MidiRelMax,   

    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0, &IntNull,         1,  0,      127,1,                      FctNull,             
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0, &IntNull,         1,  0,      127,1,                      FctNull,             
    "---",  "---",          0xFF,                  TYPE_DATA,  &TabListNull[0][0], 0, &IntNull,         1,  0,      127,1,                      FctNull,             
    "AIN",  "AUDIO IN",     MIDI_CC_AIN,           TYPE_LIST,  &YesNo[0][0],       0, &IntAudioIn,      1,  0,      127,2,                      Fct_Ch_AudioIn,        
    "CAL",  "SCREEN CAL",   MIDI_CC_CAL,           TYPE_LIST,  &YesNo[0][0],       0, &IntCalib,        1,  0,      127,2,                      Fct_Ch_Calib,        
};
#else
IHM_EXTRN char Tab_Section_Name[MAX_SECTION][20];
IHM_EXTRN Encoder_Data    Tab_Encoder[MAX_SECTION][MAX_ENCODER];

IHM_EXTRN int16_t valtofs(int16_t val,int8_t min,int8_t max,int16_t fs);
IHM_EXTRN int16_t fstoval(int16_t val,int8_t min,int8_t max,int16_t fs);

IHM_EXTRN int Fct_Ch_OscWave(int val);
IHM_EXTRN int Fct_Ch_SubWave(int val);
IHM_EXTRN int Fct_Ch_Noise(int val); 
IHM_EXTRN int Fct_Ch_Detune(int val);
IHM_EXTRN int Fct_Ch_WS1(int val);   
IHM_EXTRN int Fct_Ch_OscMix(int val);
IHM_EXTRN int Fct_Ch_SubMix(int val);
IHM_EXTRN int Fct_Ch_SubOct(int val);

IHM_EXTRN int Fct_Ch_Cutoff(int val);  
IHM_EXTRN int Fct_Ch_Resonance(int val);
IHM_EXTRN int Fct_Ch_KbTrack(int val); 
IHM_EXTRN int Fct_Ch_FVelo(int val); 
IHM_EXTRN int Fct_Ch_FType(int val); 
IHM_EXTRN int Fct_Ch_FlAttack(int val);
IHM_EXTRN int Fct_Ch_FlDecay(int val);
IHM_EXTRN int Fct_Ch_FlRelease(int val);
IHM_EXTRN int Fct_Ch_FlAmount(int val);

IHM_EXTRN int Fct_Ch_AmAttack(int val);
IHM_EXTRN int Fct_Ch_AmDecay(int val);
IHM_EXTRN int Fct_Ch_AmSustain(int val);
IHM_EXTRN int Fct_Ch_AmRelease(int val);
IHM_EXTRN int Fct_Ch_AmVelo(int val);

IHM_EXTRN int Fct_Ch_PiAttack(int val);
IHM_EXTRN int Fct_Ch_PiDecay(int val);
IHM_EXTRN int Fct_Ch_PiRelease(int val);
IHM_EXTRN int Fct_Ch_PiAmount(int val);
IHM_EXTRN int Fct_Ch_Portamento(int val);

IHM_EXTRN int Fct_Ch_L1Speed(int val);
IHM_EXTRN int Fct_Ch_L1Shape(int val);
IHM_EXTRN int Fct_Ch_L1Dest(int val);
IHM_EXTRN int Fct_Ch_L1Amount(int val);
IHM_EXTRN int Fct_Ch_L2Speed(int val);
IHM_EXTRN int Fct_Ch_L2Shape(int val);
IHM_EXTRN int Fct_Ch_L2Dest(int val);  
IHM_EXTRN int Fct_Ch_L2Amount(int val);

IHM_EXTRN int Fct_Ch_DlLen(int val);
IHM_EXTRN int Fct_Ch_DlAmount(int val);
IHM_EXTRN int Fct_Ch_DlFeed(int val);

IHM_EXTRN int Fct_Ch_SoundMode(int val);
IHM_EXTRN int Fct_Ch_PBRange(int val);
IHM_EXTRN int Fct_Ch_MDDest(int val);
IHM_EXTRN int Fct_Ch_MDAmt(int val);
IHM_EXTRN int Fct_Ch_ATDest(int val);
IHM_EXTRN int Fct_Ch_ATAmt(int val);
IHM_EXTRN int Fct_Ch_MidiRx(int val);
IHM_EXTRN int Fct_Ch_Spread(int val);

IHM_EXTRN int Fct_Ch_Transpose(int val);
IHM_EXTRN int Fct_Ch_SVolume(int val);
IHM_EXTRN int Fct_Ch_PanSpread(int val);
IHM_EXTRN int Fct_Ch_FilterLoop(int val);
IHM_EXTRN int Fct_Ch_AmpLoop(int val);
IHM_EXTRN int Fct_Ch_PitchLoop(int val);

IHM_EXTRN int Fct_Ch_AudioIn(int val);


#endif


