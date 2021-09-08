/* max delay can be changed but changes also the memory consumption */

#include <Arduino.h>

#ifdef __REVERB__
#define REVERB_EXTRN
#else
#define REVERB_EXTRN extern
#endif

#define l_CB0 3460
#define l_CB1 2988
#define l_CB2 3882
#define l_CB3 4312
#define l_AP0 480
#define l_AP1 161
#define l_AP2 46

#define SAMPLE_BUFFER_SIZE  1

REVERB_EXTRN    float reverbPan;
REVERB_EXTRN    float RevPanMod;
REVERB_EXTRN    float RevAmtMod;

REVERB_EXTRN void Reverb_Process(float *signal_l, float *signal_r, int buffLen);
REVERB_EXTRN void Reverb_Setup(void);
REVERB_EXTRN void Reverb_SetLevel(uint8_t not_used, float value);