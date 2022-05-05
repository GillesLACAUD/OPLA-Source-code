#include <Arduino.h>

#ifdef __GRANULAR__
#define GRANULAR_EXTRN
#else
#define GRANULAR_EXTRN extern
#endif

#define GRAIN_MAX			10		    // Max number of grain

#define GRA_NB_SECONDS      10           // Buffer in second
#define GRA_FS_SAMPLE       44100        // Fs sample
#define GRA_NB_CHANNELS     2            // Stereo
#define GRA_NB_BYTES        2            // 16 bits/sample

#define GRA_MAX_SIZE        44100        // Max size of a grain 500ms

#define GRA_MEMORY_SIZE     GRA_NB_SECONDS*GRA_FS_SAMPLE*GRA_NB_CHANNELS        // in int

#define GRA_BUFFER_SIZE     GRA_MAX_SIZE*GRAIN_MAX         				// 10000ms     
//#define GRA_BUFFER_SIZE     GRA_FS_SAMPLE*2                           // 500ms
//#define GRA_BUFFER_SIZE     GRA_FS_SAMPLE/5                           // 100ms
//#define GRA_BUFFER_SIZE     GRA_FS_SAMPLE/25                          // 20ms


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

GRANULAR_EXTRN uint32_t Gra_Begin;
GRANULAR_EXTRN uint32_t Gra_Space;
GRANULAR_EXTRN uint32_t Gra_Size;           // Size of a grain max GRA_MAX_SIZE
GRANULAR_EXTRN uint8_t  Gra_Density;      	// Number of grain Max GRAIN_MAX 1 Mini
GRANULAR_EXTRN uint32_t Gra_SizeAttack;     // 
GRANULAR_EXTRN uint32_t Gra_SizeSustain;
GRANULAR_EXTRN uint8_t 	Gra_OverlapPc;      // Space between 2 Grains in % of Gra_Size
GRANULAR_EXTRN uint32_t Gra_OverlapSpl;     // Space between 2 Grains in sample



GRANULAR_EXTRN str_Grain	str_tabgrain[GRAIN_MAX];

GRANULAR_EXTRN uint32_t     Cptplay;
GRANULAR_EXTRN uint32_t     Maxplay;

GRANULAR_EXTRN int16_t*    ptkeep;
GRANULAR_EXTRN int16_t*    ptGraMemory;
GRANULAR_EXTRN int16_t*    ptGraWorkingBuffer;
GRANULAR_EXTRN int16_t*    ptGraPlayingBuffer;

GRANULAR_EXTRN void     Granular_Init(void);
GRANULAR_EXTRN void     Granular_Reset(void);
GRANULAR_EXTRN uint32_t Granular_LoadWave(char* name);

