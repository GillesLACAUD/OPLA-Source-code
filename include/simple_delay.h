/* max delay can be changed but changes also the memory consumption */
//#define MAX_DELAY	11100
//#define MAX_DELAY	9000
#define MAX_DELAY	22050*2

#include <Arduino.h>

#ifdef __DELAY__
#define DELAY_EXTRN
#else
#define DELAY_EXTRN extern
#endif

/*
 * module variables
 */
DELAY_EXTRN float *delayLine_l;
DELAY_EXTRN float *delayLine_r;

#ifdef __DELAY__
float delayToMix = 0.0;
float delayFeedback = 0.0;
uint32_t delayLen = MAX_DELAY-2;
uint32_t delayIn = 0;
uint32_t delayOut = 0;
uint8_t pingpong = 0;
float delayPan = 0.0;
#else
extern float delayToMix;
extern float delayFeedback;
extern uint32_t delayLen;
extern uint32_t delayIn;
extern uint32_t delayOut;
extern uint8_t pingpong;
extern float delayPan;
#endif

DELAY_EXTRN void Delay_Init();
DELAY_EXTRN void Delay_Reset();
DELAY_EXTRN void Delay_Process(float *signal_l, float *signal_r);
DELAY_EXTRN void Delay_SetFeedback(float value);
DELAY_EXTRN void Delay_SetLevel(float value);
DELAY_EXTRN void Delay_SetLength(float value);


