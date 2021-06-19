
#include <Arduino.h>

#ifdef __SDCARD__
#define SDCARD_EXTRN
#else
#define SDCARD_EXTRN extern
#endif

#define SDCARD_NAX_NAME   100
#define SDCARD_NAME_SIZE   10

#define SDCARD_TAB_NAME     SDCARD_NAX_NAME*SDCARD_NAME_SIZE

SDCARD_EXTRN uint8_t *tabname;

SDCARD_EXTRN uint8_t SndName[SDCARD_NAME_SIZE];

SDCARD_EXTRN    uint8_t SDCard_Init(void);
SDCARD_EXTRN    void SDCard_SaveSound(uint8_t snd);
SDCARD_EXTRN    void SDCard_LoadSound(uint8_t snd);
SDCARD_EXTRN    void SDCard_LoadSndName();                              // Load all the file in the memory
SDCARD_EXTRN    void SDCard_ReadSndName(uint8_t s);                     // Read in the memory
SDCARD_EXTRN    void SDCard_WriteSndName(uint8_t s);                    // Write in the memory
SDCARD_EXTRN    void SDCard_SaveSndName();                              // Write all the memory to the file
SDCARD_EXTRN    void SDCard_Display10SndName();



