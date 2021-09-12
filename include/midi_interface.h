#include <Arduino.h>

#ifdef __MIDI__
#define MIDI_EXTRN
#else
#define MIDI_EXTRN 
#endif

/*
 * look for midi interface using 1N136
 * to convert the MIDI din signal to
 * a uart compatible signal
 */
#define RXD2 22 /* U2RRXD */
#define TXD2 19


#define MIDI_CONTROL_CHANGE                  0xB0
#define MIDI_SYSTEM_EXCLUSIVE                0xF0
#define MIDI_END_OF_EXCLUSIVE                0xF7
#define MIDI_NOTE_OFF                        0x80 // Aussi Note on avec velocity a zero
#define MIDI_NOTE_ON                         0x90

#define MIDI_PROGRAM_CHANGE                  0xC0
#define MIDI_AFTERTOUCH		                 0xD0
#define MIDI_PITCH_BEND                      0xE0

#define MIDI_MOD_WHEEL	                     0xD0

#define MIDI_SYSTEM_RESET                    0xFF
#define MIDI_ALLNOTE_OFF                     0xB0

#define MIDI_CLOCK			                 0xF8
#define MIDI_START			                 0xFA
#define MIDI_CONTINUE			             0xFB
#define MIDI_STOP  							 0xFC

#define MIDI_SONGPOS						 0xF2
#define MIDI_SONGSELECT						 0xF3

#define MIDI_ACTIVESENS						 0xFE
#define MIDI_TUNEREQUEST					 0xF6
#define MIDI_SONGSELECT						 0xF3
#define MIDI_AFTERTOUCHPOLY					 0xA0

#define MIDI_TIMECODE						 0xF1




#define MIDI_CC_WAVE1           10
#define MIDI_CC_OSCVOL          11
#define MIDI_CC_DETUNE          12
#define MIDI_CC_WS1             13
#define MIDI_CC_WS2             14

#define MIDI_CC_SUBOSC          15
#define MIDI_CC_SUBVOL          16
#define MIDI_CC_SUBTR           17
#define MIDI_CC_18              18
#define MIDI_CC_PANSPR          19

#define MIDI_CC_NTYPE           80
#define MIDI_CC_NOISE           81
#define MIDI_CC_82              82
#define MIDI_CC_83              83
#define MIDI_CC_84              84

#define MIDI_CC_BK              85
#define MIDI_CC_WA              86
#define MIDI_CC_87              87
#define MIDI_CC_88              88
#define MIDI_CC_89              89


#define MIDI_CC_CUTOFF          20
#define MIDI_CC_RES             21
#define MIDI_CC_FOLLOW          22
#define MIDI_CC_FVELO           23
#define MIDI_CC_FTYPE           24

#define MIDI_CC_FLT_A           25
#define MIDI_CC_FLT_D           26
#define MIDI_CC_FLT_R           27
#define MIDI_CC_FLT_Q           28

#define MIDI_CC_AMP_A           30
#define MIDI_CC_AMP_D           31
#define MIDI_CC_AMP_S           32
#define MIDI_CC_AMP_R           33
#define MIDI_CC_AMPVEL          34

#define MIDI_CC_PITC_A          35
#define MIDI_CC_PITC_D          36
#define MIDI_CC_PITC_R          37
#define MIDI_CC_PITC_Q          38
#define MIDI_CC_PORTA           39

#define MIDI_CC_DEL_LENGHT      50
#define MIDI_CC_DEL_LEVEL       51
#define MIDI_CC_DEL_FEEDBACK    52
#define MIDI_CC_DEL_PP          53      // Ping pong

#define MIDI_CC_REVERB_LEVEL    55
#define MIDI_CC_REVERB_PAN      56

#define MIDI_CC_SOUND_MODE      60
#define MIDI_CC_PB_RANGE        61
#define MIDI_CC_SPREAD          62
#define MIDI_CC_OCTAVE          63
#define MIDI_CC_SVOLUME         64

#define MIDI_CC_MD_DEST         65
#define MIDI_CC_MD_AMT          66
#define MIDI_CC_AT_DEST         67
#define MIDI_CC_AT_AMT          68
#define MIDI_CC_MIDI_RX         69


#define MIDI_CC_LFO1_SPEED      70
#define MIDI_CC_LFO1_SHAPE      71
#define MIDI_CC_LFO1_DEST       72
#define MIDI_CC_LFO1_AMT        73
#define MIDI_CC_LFO1_SYNC       74

#define MIDI_CC_LFO2_SPEED      75
#define MIDI_CC_LFO2_SHAPE      76
#define MIDI_CC_LFO2_DEST       77
#define MIDI_CC_LFO2_AMT        78
#define MIDI_CC_LFO2_SYNC       79


/*

// Receipt a midi CC
// Know the section
// Search the corresponding pot of the Midi CC
// .....


char ListNull[0][0];

char Wave[7][10] = 
{"SIN","SAW","SQU","PUL","TRI","NOI","NOT"};

Def_Section Tab_Section[SECTION_MAX] = 
{
    SECTION_OSC,
    0           ,"OSC"          ,&Wave[0][0]       ,MIDI_CC_WAVE1,
    1           ,"DET"          ,&ListNull[0][0]   ,MIDI_CC_DETUNE,
    2           ,"SUB"          ,&ListNull[0][0]   ,MIDI_CC_SUBOSC,
    3           ,"NOI"          ,&ListNull[0][0]   ,MIDI_CC_NOISE,



};
*/


/* use define to dump midi data */
//#define DUMP_SERIAL2_TO_SERIAL

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL	0.007874f

MIDI_EXTRN void Midi_Dump();
MIDI_EXTRN inline void Midi_NoteOn(uint8_t note,uint8_t vel);
MIDI_EXTRN inline void Midi_NoteOff(uint8_t note,uint8_t vel);
MIDI_EXTRN inline void Midi_PitchBend(uint8_t channel,uint8_t data1, uint8_t data2);
MIDI_EXTRN inline void Midi_ControlChange(uint8_t channel, uint8_t data1, uint8_t data2);
MIDI_EXTRN inline void HandleShortMsg(uint8_t *data);
MIDI_EXTRN inline void HandleRealTimeMsg(uint8_t realtime);
MIDI_EXTRN void Midi_Setup();
MIDI_EXTRN void Midi_Process();