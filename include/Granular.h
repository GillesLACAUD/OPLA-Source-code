#include <Arduino.h>

#ifdef __GRANULAR__
#define GRANULAR_EXTRN
#else
#define GRANULAR_EXTRN extern
#endif

//------------------------------------------------------------------------------------------------
// The real limitation is the size of the buffer when we read the wave file 10s here
// After we can have a sound with one grain of 10s
// Or a sound with 5 Grains of 2s....
//------------------------------------------------------------------------------------------------
#define GRA_NB_SECONDS      10                  // Buffer in second Read the first 10s in the wave file
#define GRAIN_MAX			10		            // Max number of grain 
#define GRA_FS_SAMPLE       44100               // Fs sample
#define GRA_NB_CHANNELS     2                   // Stereo
#define GRA_NB_BYTES        2                   // 16 bits/sample

// All size in Int (2 bytes)
#define GRA_MAX_SIZE        44100*2*5                                       // Max size of a grain x seconds stereo
#define GRA_MAX_SPACE		GRA_MAX_SIZE*1		                            // Max space between the grains 1s stereo  
#define GRA_MEMORY_SIZE     GRA_NB_SECONDS*GRA_FS_SAMPLE*GRA_NB_CHANNELS    // In int 10s stereo
#define GRA_BUFFER_SIZE     GRA_MEMORY_SIZE              				    // 10s stereo     

// For the A-S-R Granular EG
#define GRAIN_AR_STATE_ATTACK		0
#define GRAIN_AR_STATE_SUSTAIN		1
#define GRAIN_AR_STATE_RELEASE		2
#define GRAIN_AR_STATE_STANDBY		3

typedef struct
{
	uint8_t		u8_ident;		// Grain Number
	uint32_t	u32_beginpos;	//
	uint32_t	u32_size;		//
	uint16_t 	u16_panleft;	//
	uint16_t 	u16_panright;	//
	uint16_t	u16_cptAR;		// Cpt for the AMP progress
	uint32_t	u32_volA;		// Set the Vol Attack
	int32_t		s32_volR;		// Set the Vol Release
	uint8_t		u8_stateAR;		// State of the AR ENV
	
	uint16_t	u16_affpos;		// To display the Grain pos
	uint8_t		u8_affpan;		// To display the Grain pan
	
}str_Grain;

GRANULAR_EXTRN uint8_t Gra_Ask_RefreshPlaying;	// Ask to refresh playing buffer

GRANULAR_EXTRN uint32_t Gra_Maxplay;			// Size of the wav file load max  GRA_MEMORY_SIZE
GRANULAR_EXTRN uint32_t Gra_BufferSize;     	// Size of the buffer grain max GRA_BUFFER_SIZE
GRANULAR_EXTRN uint32_t Gra_NewBufferSize;		// Ask to new buffer size
GRANULAR_EXTRN uint32_t Gra_Begin;
GRANULAR_EXTRN uint32_t Gra_Fine;
GRANULAR_EXTRN uint32_t Gra_Space;
GRANULAR_EXTRN uint32_t Gra_Size;           	// Size of a grain max GRA_MAX_SIZE
GRANULAR_EXTRN uint8_t  Gra_Density;      		// Number of grain Max GRAIN_MAX 1 Mini
GRANULAR_EXTRN uint8_t  u8_GraReverse;

GRANULAR_EXTRN int8_t   GraTranspose;
GRANULAR_EXTRN int8_t   GraFineTune;
GRANULAR_EXTRN int8_t   GraGene400;
GRANULAR_EXTRN float    fGraFineTune;

// For the Modulation
GRANULAR_EXTRN int32_t Gra_ModBegin;
GRANULAR_EXTRN int32_t Gra_ModSpace;
GRANULAR_EXTRN int32_t Gra_ModSize;           	
GRANULAR_EXTRN int8_t  Gra_ModDensity;      		


#define GRA_EG_ATTACK		0
#define GRA_EG_SUSTAIN		1
#define GRA_EG_RELEASE		2

GRANULAR_EXTRN uint32_t Gra_SizeAttack;     // 
GRANULAR_EXTRN uint32_t Gra_SizeSustain;
GRANULAR_EXTRN uint8_t  Gra_EGState;  		// 

#define GRA_EG_FULLSCALE	100
GRANULAR_EXTRN float 	Gra_AttackCoeff;     // 
GRANULAR_EXTRN float 	Gra_ReleaseCoeff;    // 


GRANULAR_EXTRN uint8_t 	Gra_OverlapPc;      // Space between 2 Grains in % of Gra_Size
GRANULAR_EXTRN uint32_t Gra_OverlapSpl;     // Space between 2 Grains in sample



GRANULAR_EXTRN str_Grain	str_tabgrain[GRAIN_MAX];


GRANULAR_EXTRN int16_t*    	ptGraMemory;		// Memory with x s of the wav file fixe pointer
GRANULAR_EXTRN int16_t*		ptWave;				// Memory with x s of the wav file work pointer

GRANULAR_EXTRN int16_t*    	ptGraPlayingBuffer;	// Memory to the playing buffer fixe pointer
GRANULAR_EXTRN int16_t*    	ptPlay;				// Memory to the playing buffer work pointer see transpose
GRANULAR_EXTRN uint32_t     Cptplay;


GRANULAR_EXTRN int16_t*    	ptGraGrain;			// Memory to the Grain buffer fixe pointer
GRANULAR_EXTRN int16_t*    	ptGrain;			// Memory to the Grain buffer work pointer
GRANULAR_EXTRN uint32_t     CptGrain;

GRANULAR_EXTRN uint8_t    	GraBufferPlay;	// ???

GRANULAR_EXTRN int16_t*    	ptGraWorkingBuffer;	// ???
GRANULAR_EXTRN int16_t* 	ptdst;
GRANULAR_EXTRN int16_t* 	ptsrc;
GRANULAR_EXTRN int16_t*   	pt;    

GRANULAR_EXTRN int16_t*    	ptGraAddMemory;


#ifdef __GRANULAR__
double chromaticRatios[] = {
    1,
    1.0594630943591,
    1.1224620483089,
    1.1892071150019,
    1.2599210498937,
    1.3348398541685,
    1.4142135623711,
    1.4983070768743,
    1.5874010519653,
    1.6817928305039,
    1.7817974362766,
    1.8877486253586
};

unsigned char sine440[200] = {
	0x00, 0x00, 0x6B, 0x06, 0xCC, 0x0C, 0x29, 0x13, 0x65, 0x19, 0x95, 0x1F,
	0x9D, 0x25, 0x81, 0x2B, 0x3D, 0x31, 0xC0, 0x36, 0x13, 0x3C, 0x26, 0x41,
	0xF8, 0x45, 0x85, 0x4A, 0xC4, 0x4E, 0xB8, 0x52, 0x55, 0x56, 0x9E, 0x59,
	0x8A, 0x5C, 0x1C, 0x5F, 0x4B, 0x61, 0x1D, 0x63, 0x85, 0x64, 0x8D, 0x65,
	0x2E, 0x66, 0x64, 0x66, 0x3B, 0x66, 0xA3, 0x65, 0xA9, 0x64, 0x4B, 0x63,
	0x83, 0x61, 0x63, 0x5F, 0xD8, 0x5C, 0xF8, 0x59, 0xB9, 0x56, 0x25, 0x53,
	0x3A, 0x4F, 0x07, 0x4B, 0x7C, 0x46, 0xBA, 0x41, 0xA6, 0x3C, 0x61, 0x37,
	0xDD, 0x31, 0x2C, 0x2C, 0x49, 0x26, 0x47, 0x20, 0x1A, 0x1A, 0xE1, 0x13,
	0x83, 0x0D, 0x27, 0x07, 0xB9, 0x00, 0x52, 0xFA, 0xEA, 0xF3, 0x94, 0xED,
	0x4A, 0xE7, 0x20, 0xE1, 0x11, 0xDB, 0x26, 0xD5, 0x6B, 0xCF, 0xDB, 0xC9,
	0x86, 0xC4, 0x6B, 0xBF, 0x8F, 0xBA, 0xFE, 0xB5, 0xB1, 0xB1, 0xB9, 0xAD,
	0x0F, 0xAA, 0xBE, 0xA6, 0xC5, 0xA3, 0x2C, 0xA1, 0xEC, 0x9E, 0x17, 0x9D,
	0x9C, 0x9B, 0x8C, 0x9A, 0xE1, 0x99, 0x9A, 0x99, 0xBE, 0x99, 0x46, 0x9A,
	0x33, 0x9B, 0x8C, 0x9C, 0x3F, 0x9E, 0x60, 0xA0, 0xD6, 0xA2, 0xB0, 0xA5,
	0xE5, 0xA8, 0x6E, 0xAC, 0x4F, 0xB0, 0x7E, 0xB4, 0xF7, 0xB8, 0xBE, 0xBD,
	0xBF, 0xC2, 0x05, 0xC8, 0x7E, 0xCD, 0x2E, 0xD3, 0x08, 0xD9, 0x0B, 0xDF,
	0x2E, 0xE5, 0x6C, 0xEB, 0xBF, 0xF1, 0x24, 0xF8
};
uint8_t cpt440=0;
#else
GRANULAR_EXTRN double chromaticRatios[];
GRANULAR_EXTRN unsigned char sine440[200];
GRANULAR_EXTRN uint8_t cpt440;
#endif

GRANULAR_EXTRN void     Granular_Init(void);
GRANULAR_EXTRN void     Granular_Reset(void);
GRANULAR_EXTRN uint32_t Granular_LoadWave(char* name);
GRANULAR_EXTRN uint32_t Granular_AddWave(char* name);
GRANULAR_EXTRN void 	Granular_UpdateVal(void);
GRANULAR_EXTRN void     Granular_Process(uint8_t set);
GRANULAR_EXTRN void 	Granular_Dump(void);
GRANULAR_EXTRN double 	Granular_MidiNoteRatio(int midiNote);
GRANULAR_EXTRN uint32_t	Granular_TransposeStereo(notePlayerT *voice);
