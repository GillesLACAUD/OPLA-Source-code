#include <Arduino.h>

#ifdef __SYNTH__
#define SYNTH_EXTRN
#else
#define SYNTH_EXTRN extern
#endif

//#define FILTER_1		// Filtre standart max poly4
#define FILTER_7		// Filtre standart max poly4


//#define FILTER_2		// Bons resultats 4 voies ok / Rev+Del HS / Del HS / Rev HS / 5 voies HS
						// En 2 poles 
						//	-4 voies effect ok
						//	-resonance faiblarde, son bien 5 voies possible sans effects
						//  -HPF/BPF possible
/*
 * Following defines can be changed for different puprposes
 */
#define MAX_POLY_VOICE	8                               /* max single voices, can use multiple osc */
														/* 8 or more for para mode				   */

#define OSC_PER_VOICE	3                               /* max single voices, can use multiple osc */
#define MAX_POLY_OSC	(MAX_POLY_VOICE)*(OSC_PER_VOICE)    /* osc polyphony, always active reduces single voices max poly */




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
#define DEST_TYPE_COUNT	    8

#define MAX_SND_MODE		3

#define SND_MODE_POLY		0
#define SND_MODE_PARA		1
#define SND_MODE_MONO		2


#define MAX_FLT_TYPE		4


#define FILTER_LPF			0
#define FILTER_HPF			1
#define FILTER_BPF			2
#define FILTER_NPF			3

#define WAVEFORM_TYPE_COUNT	7

#define WAVE_SINE       0
#define WAVE_SAW        1
#define WAVE_SQUARE     2
#define WAVE_PULSE      3
#define WAVE_TRI        4
#define WAVE_NOISE      5
#define WAVE_SILENCE    6

SYNTH_EXTRN WorkSound	WS;

SYNTH_EXTRN uint8_t oldCurrentSound;
SYNTH_EXTRN uint8_t CurrentSound;

SYNTH_EXTRN uint8_t SoundMode;

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

unsigned char VoiceData[1024] = {
	0x8E, 0x01, 0x20, 0x04, 0x27, 0x06, 0xD3, 0x07, 0xEA, 0x08, 0x00, 0x0A,
	0x3C, 0x0B, 0xBF, 0x0C, 0x3E, 0x0E, 0xE6, 0x0F, 0x89, 0x11, 0x45, 0x13,
	0x35, 0x15, 0x4E, 0x17, 0xC4, 0x19, 0x53, 0x1C, 0xCF, 0x1E, 0x43, 0x21,
	0x61, 0x24, 0x75, 0x28, 0xD8, 0x2D, 0xFB, 0x33, 0xF1, 0x39, 0x82, 0x3F,
	0xFE, 0x43, 0x89, 0x47, 0xB2, 0x4A, 0xF5, 0x4D, 0xEE, 0x51, 0x41, 0x56,
	0xCD, 0x59, 0xF4, 0x5B, 0xE5, 0x5C, 0x41, 0x5D, 0xF1, 0x5D, 0x41, 0x5F,
	0x63, 0x61, 0x32, 0x64, 0x91, 0x67, 0x6C, 0x6B, 0x00, 0x70, 0xB6, 0x74,
	0x11, 0x79, 0x7B, 0x7C, 0x6E, 0x7E, 0x74, 0x7F, 0xDA, 0x7F, 0xFF, 0x7F,
	0xD0, 0x7F, 0x18, 0x7F, 0xA7, 0x7D, 0x7F, 0x7B, 0xC6, 0x78, 0xCC, 0x75,
	0x93, 0x72, 0x7D, 0x6F, 0x48, 0x6C, 0x18, 0x69, 0x1D, 0x66, 0x5F, 0x63,
	0xF8, 0x60, 0x80, 0x5E, 0x40, 0x5B, 0x1C, 0x57, 0x5C, 0x52, 0x67, 0x4D,
	0x52, 0x49, 0x08, 0x46, 0x65, 0x43, 0xEF, 0x40, 0x10, 0x3E, 0xCE, 0x3A,
	0x32, 0x37, 0x1D, 0x33, 0x9E, 0x2E, 0xEB, 0x29, 0xDA, 0x24, 0x31, 0x20,
	0xE3, 0x1B, 0x1E, 0x18, 0xF0, 0x14, 0xDE, 0x11, 0xE9, 0x0E, 0xCC, 0x0B,
	0x85, 0x08, 0x51, 0x05, 0xAF, 0x02, 0x80, 0x00, 0xCA, 0xFE, 0x06, 0xFD,
	0xA8, 0xFA, 0xC8, 0xF7, 0x65, 0xF4, 0x1D, 0xF1, 0x51, 0xEE, 0xDF, 0xEB,
	0xE9, 0xE9, 0x82, 0xE8, 0xAC, 0xE7, 0x75, 0xE7, 0x7A, 0xE7, 0x53, 0xE7,
	0xB3, 0xE6, 0x54, 0xE5, 0x5C, 0xE3, 0x0D, 0xE1, 0x8C, 0xDE, 0x19, 0xDC,
	0xCC, 0xD9, 0x80, 0xD7, 0x57, 0xD5, 0x7D, 0xD3, 0xDA, 0xD1, 0x90, 0xD0,
	0x6A, 0xCF, 0x9B, 0xCE, 0x40, 0xCE, 0x9D, 0xCE, 0x87, 0xCF, 0xD1, 0xD0,
	0x09, 0xD2, 0xCF, 0xD2, 0xF8, 0xD2, 0xA9, 0xD2, 0x47, 0xD2, 0x25, 0xD2,
	0x59, 0xD2, 0xD5, 0xD2, 0x69, 0xD3, 0x05, 0xD4, 0x75, 0xD4, 0xA4, 0xD4,
	0x8F, 0xD4, 0x65, 0xD4, 0x66, 0xD4, 0xD2, 0xD4, 0xC3, 0xD5, 0x50, 0xD7,
	0x63, 0xD9, 0xDA, 0xDB, 0xC5, 0xDE, 0xE8, 0xE1, 0x4B, 0xE5, 0xDD, 0xE8,
	0x3C, 0xEC, 0xA8, 0xEF, 0xE1, 0xF2, 0x03, 0xF6, 0x2B, 0xF9, 0x2E, 0xFC,
	0xD4, 0xFE, 0x36, 0x01, 0x21, 0x03, 0xCC, 0x04, 0x5E, 0x06, 0x14, 0x08,
	0x37, 0x0A, 0xCC, 0x0C, 0xB6, 0x0F, 0xDA, 0x12, 0xEA, 0x15, 0x97, 0x18,
	0xFF, 0x1A, 0x0D, 0x1D, 0xE8, 0x1E, 0xB2, 0x20, 0x54, 0x22, 0xDF, 0x23,
	0x1F, 0x25, 0xF8, 0x25, 0x69, 0x26, 0x72, 0x26, 0x2A, 0x26, 0xB1, 0x25,
	0x2F, 0x25, 0xB7, 0x24, 0x58, 0x24, 0x02, 0x24, 0x86, 0x23, 0xB4, 0x22,
	0x7E, 0x21, 0xE8, 0x1F, 0xFC, 0x1D, 0xCD, 0x1B, 0x43, 0x19, 0x75, 0x16,
	0x94, 0x13, 0x96, 0x10, 0xD5, 0x0D, 0x1D, 0x0B, 0x5E, 0x08, 0xB0, 0x05,
	0xFB, 0x02, 0x74, 0x00, 0x63, 0xFE, 0x97, 0xFC, 0x06, 0xFB, 0x7A, 0xF9,
	0xB1, 0xF7, 0xD3, 0xF5, 0xBE, 0xF3, 0x98, 0xF1, 0x69, 0xEF, 0x20, 0xED,
	0xB7, 0xEA, 0x6B, 0xE8, 0x2C, 0xE6, 0x09, 0xE4, 0xFC, 0xE1, 0xD2, 0xDF,
	0xAA, 0xDD, 0x77, 0xDB, 0x4E, 0xD9, 0x55, 0xD7, 0x9A, 0xD5, 0x09, 0xD4,
	0xD7, 0xD2, 0xEB, 0xD1, 0x40, 0xD1, 0xCD, 0xD0, 0x71, 0xD0, 0x28, 0xD0,
	0xEB, 0xCF, 0xB4, 0xCF, 0x7A, 0xCF, 0x25, 0xCF, 0x8D, 0xCE, 0x98, 0xCD,
	0x2F, 0xCC, 0x61, 0xCA, 0x6D, 0xC8, 0x73, 0xC6, 0xC9, 0xC4, 0x91, 0xC3,
	0xD6, 0xC2, 0x8B, 0xC2, 0x9D, 0xC2, 0xE2, 0xC2, 0x56, 0xC3, 0xF8, 0xC3,
	0xEC, 0xC4, 0x11, 0xC6, 0x5B, 0xC7, 0x82, 0xC8, 0x52, 0xC9, 0xDB, 0xC9,
	0x49, 0xCA, 0xC9, 0xCA, 0x89, 0xCB, 0x83, 0xCC, 0xA0, 0xCD, 0xD6, 0xCE,
	0xFD, 0xCF, 0x25, 0xD1, 0x72, 0xD2, 0xD8, 0xD3, 0x6A, 0xD5, 0x24, 0xD7,
	0xCA, 0xD8, 0x70, 0xDA, 0x08, 0xDC, 0x87, 0xDD, 0x56, 0xDF, 0x60, 0xE1,
	0xD8, 0xE3, 0x8D, 0xE6, 0x2E, 0xE9, 0xCA, 0xEB, 0x81, 0xEE, 0x52, 0xF1,
	0x61, 0xF4, 0x97, 0xF7, 0x97, 0xFA, 0x69, 0xFD, 0xB3, 0xFF, 0xAE, 0x01,
	0xA8, 0x03, 0xAC, 0x05, 0xEA, 0x07, 0x37, 0x0A, 0x69, 0x0C, 0x6D, 0x0E,
	0x5E, 0x10, 0x16, 0x12, 0xC7, 0x13, 0x3C, 0x15, 0x88, 0x16, 0xAB, 0x17,
	0x9D, 0x18, 0x52, 0x19, 0xE8, 0x19, 0x61, 0x1A, 0xE5, 0x1A, 0x7C, 0x1B,
	0x05, 0x1C, 0x80, 0x1C, 0xDA, 0x1C, 0x14, 0x1D, 0x3F, 0x1D, 0x47, 0x1D,
	0x34, 0x1D, 0xD6, 0x1C, 0x15, 0x1C, 0xD4, 0x1A, 0x25, 0x19, 0x4E, 0x17,
	0x54, 0x15, 0x46, 0x13, 0x3D, 0x11, 0x0F, 0x0F, 0x15, 0x0D, 0x3C, 0x0B,
	0xA9, 0x09, 0x51, 0x08, 0x3F, 0x07, 0x27, 0x06, 0x1D, 0x05, 0x0E, 0x04,
	0x0C, 0x03, 0x21, 0x02, 0x33, 0x01, 0x2B, 0x00, 0x26, 0xFF, 0x02, 0xFE,
	0xDB, 0xFC, 0xB8, 0xFB, 0x89, 0xFA, 0x84, 0xF9, 0x84, 0xF8, 0xA0, 0xF7,
	0xBD, 0xF6, 0xD0, 0xF5, 0xCC, 0xF4, 0xBD, 0xF3, 0xB2, 0xF2, 0xD5, 0xF1,
	0x37, 0xF1, 0xDE, 0xF0, 0xB1, 0xF0, 0x9D, 0xF0, 0x91, 0xF0, 0x8B, 0xF0,
	0x94, 0xF0, 0xBE, 0xF0, 0x19, 0xF1, 0x9D, 0xF1, 0x2F, 0xF2, 0x8F, 0xF2,
	0xA0, 0xF2, 0x57, 0xF2, 0xAD, 0xF1, 0xDD, 0xF0, 0xF6, 0xEF, 0x3F, 0xEF,
	0x91, 0xEE, 0x02, 0xEE, 0x68, 0xED, 0xDD, 0xEC, 0x74, 0xEC, 0x31, 0xEC,
	0xFD, 0xEB, 0xB1, 0xEB, 0x4C, 0xEB, 0xE5, 0xEA, 0x9E, 0xEA, 0x69, 0xEA,
	0x47, 0xEA, 0x19, 0xEA, 0xE7, 0xE9, 0xA7, 0xE9, 0x79, 0xE9, 0x54, 0xE9,
	0x5F, 0xE9, 0x72, 0xE9, 0xA1, 0xE9, 0xDB, 0xE9, 0x3D, 0xEA, 0xA5, 0xEA,
	0xEF, 0xEA, 0x03, 0xEB, 0xE5, 0xEA, 0xB7, 0xEA, 0x95, 0xEA, 0x9A, 0xEA,
	0xE0, 0xEA, 0x6B, 0xEB, 0x43, 0xEC, 0x58, 0xED, 0x92, 0xEE, 0xD7, 0xEF,
	0x3F, 0xF1, 0xBA, 0xF2, 0x69, 0xF4, 0x4E, 0xF6, 0x56, 0xF8, 0x7A, 0xFA,
	0x73, 0xFC, 0x28, 0xFE, 0x93, 0xFF, 0xE2, 0x00, 0x26, 0x02, 0xA0, 0x03,
	0x1A, 0x05, 0x8B, 0x06, 0xC4, 0x07, 0xB5, 0x08, 0x6C, 0x09, 0xF7, 0x09,
	0x63, 0x0A, 0xA4, 0x0A, 0xCD, 0x0A, 0xDD, 0x0A, 0xF9, 0x0A, 0x20, 0x0B,
	0x45, 0x0B, 0x78, 0x0B, 0xAB, 0x0B, 0xDB, 0x0B, 0xF8, 0x0B, 0xDD, 0x0B,
	0xAE, 0x0B, 0x4E, 0x0B, 0xE8, 0x0A, 0x42, 0x0A, 0x6E, 0x09, 0x39, 0x08,
	0x90, 0x06, 0x91, 0x04, 0x86, 0x02, 0x88, 0x00, 0xC8, 0xFE, 0x0B, 0xFD,
	0x36, 0xFB, 0x95, 0xF9, 0x2F, 0xF8, 0x19, 0xF7, 0x1A, 0xF6, 0xFE, 0xF4,
	0xBD, 0xF3, 0x6C, 0xF2, 0x16, 0xF1, 0xE1, 0xEF, 0xC8, 0xEE, 0xD8, 0xED,
	0x0B, 0xED, 0x53, 0xEC, 0x86, 0xEB, 0x72, 0xEA, 0x24, 0xE9, 0xCE, 0xE7,
	0xC8, 0xE6, 0x19, 0xE6, 0x98, 0xE5, 0x11, 0xE5, 0x54, 0xE4, 0x85, 0xE3,
	0xD4, 0xE2, 0x30, 0xE2, 0x9B, 0xE1, 0xE4, 0xE0, 0x50, 0xE0, 0x18, 0xE0,
	0x41, 0xE0, 0x94, 0xE0, 0xBF, 0xE0, 0xB6, 0xE0, 0x86, 0xE0, 0x63, 0xE0,
	0x64, 0xE0, 0xAE, 0xE0, 0x30, 0xE1, 0xAD, 0xE1, 0xF2, 0xE1, 0x0D, 0xE2,
	0xF8, 0xE1, 0xB0, 0xE1, 0x10, 0xE1, 0x52, 0xE0, 0xA6, 0xDF, 0x78, 0xDF,
	0xAC, 0xDF, 0x18, 0xE0, 0x69, 0xE0, 0x7C, 0xE0, 0x41, 0xE0, 0xCB, 0xDF,
	0x6C, 0xDF, 0x6F, 0xDF, 0xFE, 0xDF, 0xE1, 0xE0, 0xF1, 0xE1, 0xDD, 0xE2,
	0xF8, 0xE3, 0x53, 0xE5, 0x09, 0xE7, 0xB7, 0xE8, 0x1F, 0xEA, 0xF4, 0xEA,
	0xB6, 0xEB, 0x04, 0xED, 0x2F, 0xEF, 0x10, 0xF2, 0x8F, 0xF4, 0x3B, 0xF6,
	0x12, 0xF7, 0x7E, 0xF7, 0xD8, 0xF7, 0x67, 0xF8, 0x4E, 0xF9, 0xF3, 0xFA,
	0x0D, 0xFD, 0x66, 0xFF
};

unsigned char EpData[1024] = {
	0x21, 0x00, 0xE3, 0x00, 0xB1, 0x01, 0x7C, 0x02, 0x5C, 0x03, 0x10, 0x04,
	0xE9, 0x04, 0xB1, 0x05, 0x70, 0x06, 0x2E, 0x07, 0xFB, 0x07, 0xC8, 0x08,
	0x91, 0x09, 0x4C, 0x0A, 0xF8, 0x0A, 0x9C, 0x0B, 0x66, 0x0C, 0x0F, 0x0D,
	0xB2, 0x0D, 0x4F, 0x0E, 0xE2, 0x0E, 0x8C, 0x0F, 0x13, 0x10, 0xAC, 0x10,
	0x4D, 0x11, 0xC1, 0x11, 0x2E, 0x12, 0xAB, 0x12, 0x21, 0x13, 0x8B, 0x13,
	0x00, 0x14, 0x4F, 0x14, 0xB6, 0x14, 0x13, 0x15, 0x4B, 0x15, 0xB9, 0x15,
	0x0D, 0x16, 0x3D, 0x16, 0x7C, 0x16, 0xAF, 0x16, 0x06, 0x17, 0x2C, 0x17,
	0x70, 0x17, 0x9A, 0x17, 0xC8, 0x17, 0xEF, 0x17, 0x13, 0x18, 0x2E, 0x18,
	0x58, 0x18, 0x78, 0x18, 0x94, 0x18, 0xA9, 0x18, 0xC1, 0x18, 0xE9, 0x18,
	0xF4, 0x18, 0x09, 0x19, 0x30, 0x19, 0x3E, 0x19, 0x57, 0x19, 0x6D, 0x19,
	0x7A, 0x19, 0xA6, 0x19, 0xBD, 0x19, 0xD8, 0x19, 0xE7, 0x19, 0xF8, 0x19,
	0x0E, 0x1A, 0x39, 0x1A, 0x4E, 0x1A, 0x6B, 0x1A, 0xA2, 0x1A, 0xBB, 0x1A,
	0xE9, 0x1A, 0x0D, 0x1B, 0x30, 0x1B, 0x62, 0x1B, 0xA9, 0x1B, 0xCA, 0x1B,
	0x0E, 0x1C, 0x57, 0x1C, 0x7C, 0x1C, 0xBE, 0x1C, 0x15, 0x1D, 0x62, 0x1D,
	0x9B, 0x1D, 0x04, 0x1E, 0x4F, 0x1E, 0xA1, 0x1E, 0x17, 0x1F, 0x77, 0x1F,
	0xDF, 0x1F, 0x47, 0x20, 0xB4, 0x20, 0x32, 0x21, 0x9A, 0x21, 0x1E, 0x22,
	0xB1, 0x22, 0x35, 0x23, 0xBD, 0x23, 0x65, 0x24, 0xBF, 0x24, 0x8B, 0x25,
	0x35, 0x26, 0xBF, 0x26, 0x94, 0x27, 0x23, 0x28, 0x0A, 0x29, 0xB2, 0x29,
	0x93, 0x2A, 0x47, 0x2B, 0x2A, 0x2C, 0x1F, 0x2D, 0x0C, 0x2E, 0x05, 0x2F,
	0xEC, 0x2F, 0xF2, 0x30, 0x00, 0x32, 0x07, 0x33, 0x35, 0x34, 0x4B, 0x35,
	0x69, 0x36, 0xA1, 0x37, 0xD0, 0x38, 0x21, 0x3A, 0x55, 0x3B, 0xD3, 0x3C,
	0x27, 0x3E, 0x74, 0x3F, 0xE4, 0x40, 0x6D, 0x42, 0xD6, 0x43, 0x70, 0x45,
	0xF3, 0x46, 0x7E, 0x48, 0x15, 0x4A, 0xA7, 0x4B, 0x4B, 0x4D, 0xDE, 0x4E,
	0x85, 0x50, 0x17, 0x52, 0xB4, 0x53, 0x45, 0x55, 0xE3, 0x56, 0x60, 0x58,
	0xE6, 0x59, 0x71, 0x5B, 0xCB, 0x5C, 0x32, 0x5E, 0x8D, 0x5F, 0xD2, 0x60,
	0x13, 0x62, 0x39, 0x63, 0x5A, 0x64, 0x5E, 0x65, 0x54, 0x66, 0x49, 0x67,
	0x03, 0x68, 0xBE, 0x68, 0x88, 0x69, 0x11, 0x6A, 0x62, 0x6A, 0x83, 0x6A,
	0x57, 0x6A, 0xF5, 0x69, 0x28, 0x69, 0xC0, 0x67, 0xD2, 0x65, 0x1C, 0x63,
	0x53, 0x5F, 0x93, 0x5A, 0x71, 0x54, 0xED, 0x4C, 0xFD, 0x43, 0x9E, 0x39,
	0xDA, 0x2D, 0xEA, 0x20, 0xE5, 0x12, 0x1E, 0x04, 0xF6, 0xF4, 0xC2, 0xE5,
	0xBE, 0xD6, 0xA0, 0xC8, 0x90, 0xBB, 0xCB, 0xAF, 0x6A, 0xA5, 0xD6, 0x9C,
	0xE5, 0x95, 0xCC, 0x90, 0x81, 0x8D, 0x7F, 0x8B, 0x32, 0x8B, 0x15, 0x8C,
	0x14, 0x8E, 0xE4, 0x90, 0x82, 0x94, 0x5A, 0x98, 0x76, 0x9C, 0xDE, 0xA0,
	0x1E, 0xA5, 0x56, 0xA9, 0x2B, 0xAD, 0xBC, 0xB0, 0x1A, 0xB4, 0xEE, 0xB6,
	0x84, 0xB9, 0xD3, 0xBB, 0xCD, 0xBD, 0x6C, 0xBF, 0xAF, 0xC0, 0xC4, 0xC1,
	0xB2, 0xC2, 0x6B, 0xC3, 0xF1, 0xC3, 0x6E, 0xC4, 0xA4, 0xC4, 0xE4, 0xC4,
	0x1E, 0xC5, 0x4A, 0xC5, 0x6A, 0xC5, 0x92, 0xC5, 0xE6, 0xC5, 0x24, 0xC6,
	0x75, 0xC6, 0xE4, 0xC6, 0x37, 0xC7, 0xBA, 0xC7, 0x14, 0xC8, 0xAB, 0xC8,
	0x48, 0xC9, 0xD8, 0xC9, 0x7F, 0xCA, 0x1A, 0xCB, 0xDD, 0xCB, 0x88, 0xCC,
	0x3E, 0xCD, 0xF6, 0xCD, 0x8C, 0xCE, 0x5A, 0xCF, 0x10, 0xD0, 0xCF, 0xD0,
	0x7A, 0xD1, 0x1C, 0xD2, 0xDA, 0xD2, 0x66, 0xD3, 0x0A, 0xD4, 0x9A, 0xD4,
	0x30, 0xD5, 0xE8, 0xD5, 0x65, 0xD6, 0xD9, 0xD6, 0x84, 0xD7, 0x0E, 0xD8,
	0x91, 0xD8, 0x0F, 0xD9, 0xA3, 0xD9, 0x03, 0xDA, 0x9D, 0xDA, 0xF9, 0xDA,
	0x6D, 0xDB, 0x0B, 0xDC, 0x6B, 0xDC, 0xDD, 0xDC, 0x44, 0xDD, 0xD4, 0xDD,
	0x2D, 0xDE, 0xB3, 0xDE, 0x2A, 0xDF, 0xB0, 0xDF, 0x39, 0xE0, 0xA3, 0xE0,
	0x38, 0xE1, 0xBC, 0xE1, 0x61, 0xE2, 0xE6, 0xE2, 0x97, 0xE3, 0x1D, 0xE4,
	0xD1, 0xE4, 0x8E, 0xE5, 0x42, 0xE6, 0xE2, 0xE6, 0x9F, 0xE7, 0x73, 0xE8,
	0x26, 0xE9, 0xF6, 0xE9, 0xC2, 0xEA, 0xAA, 0xEB, 0x8A, 0xEC, 0x5E, 0xED,
	0x53, 0xEE, 0x39, 0xEF, 0x33, 0xF0, 0x39, 0xF1, 0x36, 0xF2, 0x1D, 0xF3,
	0x1D, 0xF4, 0x2A, 0xF5, 0x24, 0xF6, 0x46, 0xF7, 0x4C, 0xF8, 0x6B, 0xF9,
	0x6B, 0xFA, 0x8C, 0xFB, 0xA5, 0xFC, 0xBD, 0xFD, 0xC0, 0xFE, 0xD8, 0xFF,
	0xD6, 0x00, 0xE2, 0x01, 0xFB, 0x02, 0xFD, 0x03, 0x17, 0x05, 0x06, 0x06,
	0x2C, 0x07, 0x19, 0x08, 0x25, 0x09, 0x1A, 0x0A, 0x23, 0x0B, 0x43, 0x0C,
	0x20, 0x0D, 0x38, 0x0E, 0x36, 0x0F, 0x32, 0x10, 0x2B, 0x11, 0x15, 0x12,
	0x0C, 0x13, 0x12, 0x14, 0x21, 0x15, 0x0A, 0x16, 0xF1, 0x16, 0xF7, 0x17,
	0xF6, 0x18, 0xD5, 0x19, 0xD4, 0x1A, 0xE1, 0x1B, 0xC8, 0x1C, 0xD6, 0x1D,
	0xCA, 0x1E, 0xCE, 0x1F, 0xBA, 0x20, 0xDB, 0x21, 0xD7, 0x22, 0xDA, 0x23,
	0xDC, 0x24, 0xE8, 0x25, 0xF8, 0x26, 0x1B, 0x28, 0x3D, 0x29, 0x62, 0x2A,
	0x8D, 0x2B, 0xAF, 0x2C, 0xE6, 0x2D, 0x0B, 0x2F, 0x5A, 0x30, 0x86, 0x31,
	0xD2, 0x32, 0x01, 0x34, 0x70, 0x35, 0xBB, 0x36, 0x0C, 0x38, 0x51, 0x39,
	0xB2, 0x3A, 0xED, 0x3B, 0x6D, 0x3D, 0xB3, 0x3E, 0x08, 0x40, 0xB8, 0x41,
	0xF1, 0x42, 0x64, 0x44, 0xA2, 0x45, 0x29, 0x47, 0x90, 0x48, 0x0C, 0x4A,
	0x8A, 0x4B, 0x00, 0x4D, 0x5F, 0x4E, 0xC6, 0x4F, 0x0F, 0x51, 0x32, 0x52,
	0x2A, 0x53, 0xFC, 0x53, 0x8D, 0x54, 0xC5, 0x54, 0x6D, 0x54, 0x76, 0x53,
	0xD0, 0x51, 0x4F, 0x4F, 0xB6, 0x4B, 0xE5, 0x46, 0xBF, 0x40, 0x5B, 0x39,
	0x87, 0x30, 0x52, 0x26, 0xF6, 0x1A, 0x3A, 0x0E, 0xB5, 0x00, 0x95, 0xF2,
	0xE3, 0xE3, 0x57, 0xD5, 0xF7, 0xC6, 0x68, 0xB9, 0xFD, 0xAC, 0xBA, 0xA1,
	0xFE, 0x97, 0xD7, 0x8F, 0x47, 0x89, 0x9E, 0x84, 0x84, 0x81, 0x07, 0x80,
	0x0D, 0x80, 0x33, 0x81, 0x8D, 0x83, 0x9D, 0x86, 0x2F, 0x8A, 0x2D, 0x8E,
	0x4A, 0x92, 0x8E, 0x96, 0x9F, 0x9A, 0xA0, 0x9E, 0x5B, 0xA2, 0xC9, 0xA5,
	0x05, 0xA9, 0xE3, 0xAB, 0x70, 0xAE, 0xD6, 0xB0, 0x08, 0xB3, 0xF6, 0xB4,
	0xBB, 0xB6, 0x45, 0xB8, 0xAF, 0xB9, 0x0B, 0xBB, 0x57, 0xBC, 0x6C, 0xBD,
	0x89, 0xBE, 0x7E, 0xBF, 0x92, 0xC0, 0x91, 0xC1, 0x8F, 0xC2, 0xAC, 0xC3,
	0xB4, 0xC4, 0xCE, 0xC5, 0xE6, 0xC6, 0x18, 0xC8, 0x41, 0xC9, 0x6F, 0xCA,
	0xA5, 0xCB, 0xE1, 0xCC, 0x0C, 0xCE, 0x5B, 0xCF, 0x96, 0xD0, 0xD1, 0xD1,
	0xF8, 0xD2, 0x2B, 0xD4, 0x58, 0xD5, 0x84, 0xD6, 0x93, 0xD7, 0xA8, 0xD8,
	0xC1, 0xD9, 0xC2, 0xDA, 0xEC, 0xDB, 0xE5, 0xDC, 0xD6, 0xDD, 0xCE, 0xDE,
	0xB7, 0xDF, 0x8C, 0xE0, 0x68, 0xE1, 0x46, 0xE2, 0x03, 0xE3, 0xC9, 0xE3,
	0x8C, 0xE4, 0x36, 0xE5, 0xED, 0xE5, 0xB0, 0xE6, 0x4B, 0xE7, 0xDA, 0xE7,
	0x80, 0xE8, 0x36, 0xE9, 0xC3, 0xE9, 0x33, 0xEA, 0xCD, 0xEA, 0x69, 0xEB,
	0x09, 0xEC, 0x86, 0xEC, 0x19, 0xED, 0x99, 0xED, 0x1E, 0xEE, 0xA5, 0xEE,
	0x30, 0xEF, 0xCA, 0xEF, 0x32, 0xF0, 0xDE, 0xF0, 0x5C, 0xF1, 0xDE, 0xF1,
	0x77, 0xF2, 0x0F, 0xF3, 0xAC, 0xF3, 0x2E, 0xF4, 0xD3, 0xF4, 0x79, 0xF5,
	0x1F, 0xF6, 0xC5, 0xF6, 0x61, 0xF7, 0x23, 0xF8, 0xC6, 0xF8, 0x6E, 0xF9,
	0x26, 0xFA, 0xE0, 0xFA, 0xB3, 0xFB, 0x5E, 0xFC, 0x27, 0xFD, 0xF8, 0xFD,
	0xB0, 0xFE, 0x8C, 0xFF
};

unsigned char Ac1Data[1024] = {
	0x66, 0x02, 0xB6, 0x07, 0x2F, 0x0D, 0xCF, 0x12, 0xBA, 0x18, 0xD0, 0x1E,
	0xE6, 0x24, 0x4B, 0x2B, 0x28, 0x31, 0x86, 0x37, 0xB1, 0x3D, 0x7F, 0x43,
	0x5F, 0x49, 0xF8, 0x4E, 0x36, 0x54, 0x3D, 0x59, 0x0B, 0x5E, 0x77, 0x62,
	0x9D, 0x66, 0x2B, 0x6A, 0x8A, 0x6D, 0x84, 0x70, 0x3A, 0x73, 0xAE, 0x75,
	0xAD, 0x77, 0x58, 0x79, 0xE0, 0x7A, 0x10, 0x7C, 0xFB, 0x7C, 0xDA, 0x7D,
	0x7B, 0x7E, 0xE4, 0x7E, 0x3C, 0x7F, 0x89, 0x7F, 0xBF, 0x7F, 0xC6, 0x7F,
	0xEC, 0x7F, 0xF6, 0x7F, 0xFB, 0x7F, 0xE5, 0x7F, 0xE6, 0x7F, 0xAE, 0x7F,
	0x5E, 0x7F, 0x22, 0x7F, 0xEF, 0x7E, 0x96, 0x7E, 0xFE, 0x7D, 0x78, 0x7D,
	0xD7, 0x7C, 0x06, 0x7C, 0x34, 0x7B, 0x47, 0x7A, 0x42, 0x79, 0x1E, 0x78,
	0xBB, 0x76, 0x32, 0x75, 0x8E, 0x73, 0xC2, 0x71, 0xCC, 0x6F, 0x98, 0x6D,
	0x5E, 0x6B, 0x0B, 0x69, 0x44, 0x66, 0x66, 0x63, 0x6E, 0x60, 0x4F, 0x5D,
	0x1F, 0x5A, 0x79, 0x56, 0xC2, 0x52, 0xE6, 0x4E, 0xF0, 0x4A, 0xAA, 0x46,
	0x5F, 0x42, 0xD5, 0x3D, 0x11, 0x39, 0x4D, 0x34, 0x3E, 0x2F, 0x54, 0x2A,
	0x0E, 0x25, 0xA2, 0x1F, 0x4C, 0x1A, 0x0C, 0x15, 0x7E, 0x0F, 0xF2, 0x09,
	0x89, 0x04, 0xFB, 0xFE, 0xCA, 0xF9, 0x77, 0xF4, 0x48, 0xEF, 0x3D, 0xEA,
	0x86, 0xE5, 0xF4, 0xE0, 0x6C, 0xDC, 0x23, 0xD8, 0x41, 0xD4, 0x8D, 0xD0,
	0xEA, 0xCC, 0xD5, 0xC9, 0xBF, 0xC6, 0x00, 0xC4, 0x67, 0xC1, 0xE5, 0xBE,
	0xAE, 0xBC, 0x79, 0xBA, 0x8B, 0xB8, 0xCB, 0xB6, 0xEE, 0xB4, 0x1A, 0xB3,
	0x6A, 0xB1, 0xA3, 0xAF, 0x0E, 0xAE, 0x83, 0xAC, 0xDB, 0xAA, 0x4F, 0xA9,
	0xD0, 0xA7, 0x1D, 0xA6, 0xC0, 0xA4, 0x49, 0xA3, 0xB7, 0xA1, 0x5E, 0xA0,
	0xEC, 0x9E, 0xAA, 0x9D, 0x58, 0x9C, 0x2A, 0x9B, 0xFE, 0x99, 0xB1, 0x98,
	0x79, 0x97, 0x6B, 0x96, 0x5D, 0x95, 0x29, 0x94, 0x27, 0x93, 0x2F, 0x92,
	0x0D, 0x91, 0xF5, 0x8F, 0xF9, 0x8E, 0xEB, 0x8D, 0xEC, 0x8C, 0x17, 0x8C,
	0x30, 0x8B, 0x65, 0x8A, 0x96, 0x89, 0xEF, 0x88, 0x80, 0x88, 0xFB, 0x87,
	0x95, 0x87, 0x6C, 0x87, 0x64, 0x87, 0x5F, 0x87, 0xA3, 0x87, 0xF9, 0x87,
	0x60, 0x88, 0xE8, 0x88, 0x79, 0x89, 0x18, 0x8A, 0xCB, 0x8A, 0x70, 0x8B,
	0x1A, 0x8C, 0xF5, 0x8C, 0xA5, 0x8D, 0x50, 0x8E, 0xF2, 0x8E, 0x8F, 0x8F,
	0x44, 0x90, 0xB2, 0x90, 0x2F, 0x91, 0xC2, 0x91, 0x44, 0x92, 0xAA, 0x92,
	0x44, 0x93, 0xE2, 0x93, 0x6B, 0x94, 0x0D, 0x95, 0xED, 0x95, 0xE2, 0x96,
	0xC0, 0x97, 0xB6, 0x98, 0xEB, 0x99, 0x50, 0x9B, 0xBB, 0x9C, 0x4C, 0x9E,
	0x12, 0xA0, 0xE5, 0xA1, 0xCE, 0xA3, 0xA7, 0xA5, 0xDC, 0xA7, 0x21, 0xAA,
	0x42, 0xAC, 0x95, 0xAE, 0xFD, 0xB0, 0x5B, 0xB3, 0xC1, 0xB5, 0x51, 0xB8,
	0xE8, 0xBA, 0x94, 0xBD, 0x4F, 0xC0, 0x1F, 0xC3, 0xD4, 0xC5, 0x71, 0xC8,
	0x69, 0xCB, 0x75, 0xCE, 0x4F, 0xD1, 0x7D, 0xD4, 0xAB, 0xD7, 0x79, 0xDA,
	0xA4, 0xDD, 0x89, 0xE0, 0x78, 0xE3, 0x81, 0xE6, 0x38, 0xE9, 0x17, 0xEC,
	0xD5, 0xEE, 0x69, 0xF1, 0x03, 0xF4, 0xAF, 0xF6, 0x38, 0xF9, 0xEB, 0xFB,
	0x7B, 0xFE, 0x01, 0x01, 0xBE, 0x03, 0x82, 0x06, 0x25, 0x09, 0x18, 0x0C,
	0x17, 0x0F, 0x48, 0x12, 0x70, 0x15, 0xAE, 0x18, 0x40, 0x1C, 0x7E, 0x1F,
	0xDF, 0x22, 0x9E, 0x26, 0x2E, 0x2A, 0x6A, 0x2D, 0xE5, 0x30, 0x3A, 0x34,
	0x60, 0x37, 0x92, 0x3A, 0x67, 0x3D, 0x44, 0x40, 0xE2, 0x42, 0x43, 0x45,
	0xAB, 0x47, 0xCC, 0x49, 0xB7, 0x4B, 0xD3, 0x4D, 0xDC, 0x4F, 0xC9, 0x51,
	0x8E, 0x53, 0x3F, 0x55, 0x3D, 0x57, 0x42, 0x59, 0x2D, 0x5B, 0x31, 0x5D,
	0x69, 0x5F, 0x66, 0x61, 0x53, 0x63, 0x7B, 0x65, 0x71, 0x67, 0x6B, 0x69,
	0x27, 0x6B, 0xAC, 0x6C, 0x35, 0x6E, 0x73, 0x6F, 0x8C, 0x70, 0x52, 0x71,
	0xB8, 0x71, 0x1F, 0x72, 0x3C, 0x72, 0x2A, 0x72, 0xC8, 0x71, 0x3D, 0x71,
	0x96, 0x70, 0xB7, 0x6F, 0xCD, 0x6E, 0xD3, 0x6D, 0x03, 0x6D, 0xF4, 0x6B,
	0xDC, 0x6A, 0xE8, 0x69, 0xE5, 0x68, 0xEB, 0x67, 0xE1, 0x66, 0xFB, 0x65,
	0xEE, 0x64, 0xD2, 0x63, 0x94, 0x62, 0x36, 0x61, 0xB0, 0x5F, 0xBF, 0x5D,
	0x8D, 0x5B, 0x2F, 0x59, 0x58, 0x56, 0x4D, 0x53, 0xFB, 0x4F, 0x37, 0x4C,
	0x72, 0x48, 0x7A, 0x44, 0x23, 0x40, 0x07, 0x3C, 0xB9, 0x37, 0x62, 0x33,
	0x6A, 0x2F, 0x72, 0x2B, 0xDD, 0x27, 0x8E, 0x24, 0x82, 0x21, 0x0A, 0x1F,
	0xEF, 0x1C, 0x13, 0x1B, 0xA6, 0x19, 0x95, 0x18, 0xAF, 0x17, 0x34, 0x17,
	0xBD, 0x16, 0x50, 0x16, 0xD2, 0x15, 0x1D, 0x15, 0x56, 0x14, 0x73, 0x13,
	0xE8, 0x11, 0x0B, 0x10, 0xED, 0x0D, 0x4A, 0x0B, 0x74, 0x08, 0x10, 0x05,
	0x59, 0x01, 0x48, 0xFD, 0x05, 0xF9, 0xA4, 0xF4, 0x21, 0xF0, 0x8A, 0xEB,
	0x2A, 0xE7, 0xFE, 0xE2, 0xC2, 0xDE, 0xF4, 0xDA, 0x8A, 0xD7, 0x62, 0xD4,
	0xB0, 0xD1, 0x65, 0xCF, 0x69, 0xCD, 0xD8, 0xCB, 0xBB, 0xCA, 0xD6, 0xC9,
	0x3A, 0xC9, 0xF6, 0xC8, 0x96, 0xC8, 0x97, 0xC8, 0x75, 0xC8, 0x71, 0xC8,
	0xA8, 0xC8, 0x8B, 0xC8, 0x8A, 0xC8, 0x5E, 0xC8, 0x2C, 0xC8, 0x32, 0xC8,
	0xEA, 0xC7, 0xA5, 0xC7, 0x95, 0xC7, 0x9A, 0xC7, 0xC2, 0xC7, 0x00, 0xC8,
	0x7C, 0xC8, 0x5B, 0xC9, 0x52, 0xCA, 0x74, 0xCB, 0xC3, 0xCC, 0x92, 0xCE,
	0x86, 0xD0, 0x8E, 0xD2, 0x0E, 0xD5, 0x56, 0xD7, 0xFA, 0xD9, 0xA9, 0xDC,
	0x08, 0xDF, 0xBA, 0xE1, 0x47, 0xE4, 0x9F, 0xE6, 0xEE, 0xE8, 0xEC, 0xEA,
	0xC9, 0xEC, 0x93, 0xEE, 0xED, 0xEF, 0x67, 0xF1, 0x93, 0xF2, 0x62, 0xF3,
	0x52, 0xF4, 0x31, 0xF5, 0xD7, 0xF5, 0x7B, 0xF6, 0x33, 0xF7, 0xE5, 0xF7,
	0x9E, 0xF8, 0x4C, 0xF9, 0x25, 0xFA, 0x33, 0xFB, 0xFF, 0xFB, 0xFF, 0xFC,
	0x20, 0xFE, 0x36, 0xFF, 0x21, 0x00, 0x03, 0x01, 0xED, 0x01, 0xCC, 0x02,
	0x79, 0x03, 0x1B, 0x04, 0xA3, 0x04, 0xF7, 0x04, 0x4F, 0x05, 0x71, 0x05,
	0x7C, 0x05, 0x69, 0x05, 0x8B, 0x05, 0x6F, 0x05, 0x34, 0x05, 0x0F, 0x05,
	0x39, 0x05, 0x4D, 0x05, 0x89, 0x05, 0x03, 0x06, 0x94, 0x06, 0x6F, 0x07,
	0x5A, 0x08, 0x76, 0x09, 0xB5, 0x0A, 0x24, 0x0C, 0xAF, 0x0D, 0x60, 0x0F,
	0xE8, 0x10, 0xB6, 0x12, 0x5B, 0x14, 0xE4, 0x15, 0x5B, 0x17, 0x9A, 0x18,
	0xB3, 0x19, 0x7C, 0x1A, 0x06, 0x1B, 0x08, 0x1B, 0xFF, 0x1A, 0x53, 0x1A,
	0x2D, 0x19, 0xE1, 0x17, 0xE9, 0x15, 0xA8, 0x13, 0x1A, 0x11, 0x0A, 0x0E,
	0xD0, 0x0A, 0x13, 0x07, 0x21, 0x03, 0x48, 0xFF, 0x12, 0xFB, 0xC1, 0xF6,
	0xCA, 0xF2, 0x68, 0xEE, 0x4A, 0xEA, 0x80, 0xE6, 0xC2, 0xE2, 0x45, 0xDF,
	0xE5, 0xDB, 0xF9, 0xD8, 0x63, 0xD6, 0x29, 0xD4, 0x28, 0xD2, 0xBB, 0xD0,
	0x80, 0xCF, 0x94, 0xCE, 0x22, 0xCE, 0xE8, 0xCD, 0x05, 0xCE, 0x4F, 0xCE,
	0x03, 0xCF, 0xD2, 0xCF, 0xBB, 0xD0, 0xF3, 0xD1, 0x3A, 0xD3, 0x7C, 0xD4,
	0xCA, 0xD5, 0x2B, 0xD7, 0x84, 0xD8, 0xDE, 0xD9, 0x10, 0xDB, 0x30, 0xDC,
	0x26, 0xDD, 0xEF, 0xDD, 0xA7, 0xDE, 0x35, 0xDF, 0xA6, 0xDF, 0xDE, 0xDF,
	0xF8, 0xDF, 0xFE, 0xDF, 0xD9, 0xDF, 0xB1, 0xDF, 0x78, 0xDF, 0x2B, 0xDF,
	0xD8, 0xDE, 0x7E, 0xDE, 0x20, 0xDE, 0x03, 0xDE, 0x07, 0xDE, 0x1A, 0xDE,
	0x68, 0xDE, 0xE4, 0xDE, 0x91, 0xDF, 0x97, 0xE0, 0x03, 0xE2, 0x95, 0xE3,
	0x9E, 0xE5, 0xE7, 0xE7, 0xA2, 0xEA, 0xEA, 0xED, 0x36, 0xF1, 0x08, 0xF5,
	0x2D, 0xF9, 0xD9, 0xFD
};



uint8_t selWaveForm1=0;
uint8_t selWaveForm2=0;

uint8_t globalrank=0;

float oscdetune=0;

char Filter_Type[MAX_FLT_TYPE][MAX_LABEL] =
{"LPF","HPF","BPF","NPF"};

char Sound_Mode[MAX_SND_MODE][MAX_LABEL] = 
{"POL","PAR","MON"};

char Wave_Name[WAVEFORM_TYPE_COUNT][MAX_LABEL] = 
{"SIN","SAW","SQU","PUL","TRI","NOI","NOT"};

char Dest_Name[DEST_TYPE_COUNT][MAX_LABEL] = 
{"AMP","CUT","PIT","NOI","PAN","WS1","LxS","LxA"};

/*
 * do not forget to enter the waveform pointer addresses here
 */
float *waveFormLookUp[WAVEFORM_TYPE_COUNT] = {&sine[0], &saw[0], &square[0], &pulse[0], &tri[0], &noise[0], &silence[0]};
float *selectedWaveForm =  &sine[0];
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

float MixOsc = 1;
float MixSub = 1;
float SubTranspose = -12.0;

float pitchBendValue = 0.0f;
float pitchMultiplier = 1.0f;

float AmpVel=0.5;
float FilterVel=1.0;

#else

extern char Filter_Type[MAX_FLT_TYPE][MAX_LABEL];
extern char Wave_Name[WAVEFORM_TYPE_COUNT][MAX_LABEL];
extern char Dest_Name[DEST_TYPE_COUNT][MAX_LABEL];
extern char Sound_Mode[3][MAX_LABEL];

extern float sine[WAVEFORM_CNT];
extern float saw[WAVEFORM_CNT];
extern float square[WAVEFORM_CNT];
extern float pulse[WAVEFORM_CNT];
extern float tri[WAVEFORM_CNT];
extern float noise[WAVEFORM_CNT];
extern float silence[WAVEFORM_CNT];
extern float wavework[WAVEFORM_CNT];
extern float wavetrash[WAVEFORM_CNT];

extern unsigned char VoiceData[1024];
extern unsigned char EpData[1024];
extern unsigned char Ac1Data[1024];


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
extern float soundFiltReso;
extern float filtReso;
extern float filterEG;
extern float filterKBtrack;

extern float NoiseLevel;
extern float WaveShapping1;
extern float OldWaveShapping1Mod;

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

extern float SubTranspose;

extern float MixOsc;
extern float MixSub;

extern float pitchBendValue;
extern float pitchMultiplier;

extern float AmpVel;
extern float FilterVel;


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
SYNTH_EXTRN int Synth_SetRotary(uint8_t rotary, int val);

#ifdef FILTER_5
SYNTH_EXTRN void Filter_Calculate(float c, float reso, struct filterCoeffT *const  filterC);
#endif

