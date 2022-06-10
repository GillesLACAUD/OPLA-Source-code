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
#else
GRANULAR_EXTRN double chromaticRatios[];
#endif



GRANULAR_EXTRN void     Granular_Init(void);
GRANULAR_EXTRN void     Granular_Reset(void);
GRANULAR_EXTRN uint32_t Granular_LoadWave(char* name);
GRANULAR_EXTRN uint32_t Granular_AddWave(char* name);
GRANULAR_EXTRN void 	Granular_UpdateVal(void);
GRANULAR_EXTRN void 	Granular_Process(void);
GRANULAR_EXTRN void 	Granular_Dump(void);
GRANULAR_EXTRN double 	Granular_MidiNoteRatio(int midiNote);
GRANULAR_EXTRN uint32_t	Granular_TransposeStereo(notePlayerT *voice);


