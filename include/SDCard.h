
#include <Arduino.h>

#ifdef __SDCARD__
#define SDCARD_EXTRN
#else
#define SDCARD_EXTRN extern
#endif

SDCARD_EXTRN    uint8_t SDCard_Init(void);
SDCARD_EXTRN    void SDCard_SaveSound(uint8_t snd);
SDCARD_EXTRN    void SDCard_LoadSound(uint8_t snd);
SDCARD_EXTRN    void SDCard_LoadSndName(void);
SDCARD_EXTRN    void SDCard_SaveSndName(uint8_t snd);


