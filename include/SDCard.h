
#include <Arduino.h>

#ifdef __SDCARD__
#define SDCARD_EXTRN
#else
#define SDCARD_EXTRN extern
#endif

#define SDCARD_NAX_NAME   100
#define SDCARD_NAME_SIZE   10

#define SDCARD_TAB_NAME     SDCARD_NAX_NAME*SDCARD_NAME_SIZE

SDCARD_EXTRN uint8_t trigloadwave;
SDCARD_EXTRN uint16_t Cptloadwave;

SDCARD_EXTRN uint8_t LastSoundNumber;

SDCARD_EXTRN uint8_t IsLoadSound;                                       // At 1 when loading sound

#define LOADWAVE_MAX_OVER_TIME  15000

SDCARD_EXTRN uint8_t *tabname;

SDCARD_EXTRN uint8_t SndName[SDCARD_NAME_SIZE];


typedef struct
{
    uint8_t id;
	char name[20];
	uint8_t nbr;
}Bank_AKWF;

#define AKWFMAX_BANK	64

const Bank_AKWF	SampleDIR[AKWFMAX_BANK] =
{
		0,"0000",100,
		1,"0001",100,
		2,"0002",100,
		3,"0003",100,
		4,"0004",100,
		5,"0005",100,
		6,"0006",100,
		7,"0007",100,
		8,"0008",100,
		9,"0009",100,
		10,"0010",100,
		11,"0011",100,
		12,"0012",100,
		13,"0013",100,
		14,"0014",100,
		15,"0015",100,
		16,"0016",100,
		17,"0017",100,
		18,"0018",100,
		19,"0019",111,
        20,"aguitare",38,
        21,"altosax",26,
		22,"birds",14,
		23,"bitreduce",40,
		24,"blended",73,
        25,"perfectwave",4,
        26,"Saw",50,
        27,"Saw Bright",10,
        28,"Saw Gap",42,
        29,"Saw Round",52,
        30,"Sine",12,
        31,"Square",100,
        32,"Square Round",52,
        33,"Triangle",25,
		34,"C604",32,
		35,"Cello",19,
		36,"Clarinet",25,
		37,"Clavinet",33,
		38,"D Bass",69,
		39,"Distorded",45,
		40,"E Bass",70,
		41,"E Guitare",22,
        42,"E Organ",127,
		43,"E Piano",73,
		44,"Flute",17,
		45,"FM Synth",122,
        46,"Granular",44,
        47,"H Draw",50,
        48,"H Voice",104,
        49,"Linear",85,
        50,"Oboe",13,
        51,"OSC Chip",128,
        52,"Overtone",44,
        53,"Piano",30,
        54,"Pluck",9,
		55,"Raw",36,
        56,"SIN Harmo",16,
        57,"Snippets",47,
        58,"String Box",6,
        59,"Symetric",17,
        60,"Theremin",26,
        61,"Game",128,
        62,"Game Basic",64,
        63,"Violin",14
		};


SDCARD_EXTRN    uint8_t SDCard_Init(void);
SDCARD_EXTRN    void SDCard_SaveSound(uint8_t snd);
SDCARD_EXTRN    void SDCard_LoadSound(uint8_t snd);
SDCARD_EXTRN    void SDCard_LoadSndName();                              // Load all the file in the memory
SDCARD_EXTRN    void SDCard_ReadSndName(uint8_t s);                     // Read in the memory
SDCARD_EXTRN    void SDCard_WriteSndName(uint8_t s);                    // Write in the memory
SDCARD_EXTRN    void SDCard_SaveSndName();                              // Write all the memory to the file
SDCARD_EXTRN    void SDCard_Display10SndName();
SDCARD_EXTRN    void SDCard_LoadLastSound();

SDCARD_EXTRN    void SDCard_SaveMidiRx();                              	// Write Midi Rx
SDCARD_EXTRN    void SDCard_LoadMidiRx();                              	// Read Midi Rx

SDCARD_EXTRN    void SDCard_LoadWave(uint8_t bk,uint8_t wa);



