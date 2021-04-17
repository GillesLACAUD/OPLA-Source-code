#include <Arduino.h>

#ifdef __LFO__
#define LFO_EXTRN
#else
#define LFO_EXTRN   extern
#endif

#define WAVE_LFO_SIZE   1024

#define WAVE_LFO_COUNT  8

#define LFO_ID1     0
#define LFO_ID2     1
#define TIMER_1MS   2

#define LFO_MIN_CPT 50


#define LFO_MAX_TIME    1024*2

LFO_EXTRN struct stLfo Lfo1;
LFO_EXTRN struct stLfo Lfo2;

#ifdef __LFO__

char Wave_LfoName[WAVE_LFO_COUNT][5] = 
{"SIN","TRI","SAW","ISA","SQU","S&H","NOI","MUT"};

volatile uint32_t Lfo_cnt1=0;
volatile uint32_t Lfo_cnt2=0;
volatile uint32_t Timer1ms_cnt=0;

hw_timer_t * Lfo_timer1=NULL;
hw_timer_t * Lfo_timer2=NULL;
hw_timer_t * timer_1ms=NULL;

extern float *sine;

float **selectedWaveLfo1 =  &sine;
float **selectedWaveLfo2 =  &sine;
#else
extern char Wave_LfoName[WAVE_LFO_COUNT][5];
extern volatile uint32_t Lfo_cnt1;
extern volatile uint32_t Lfo_cnt2;
extern volatile uint32_t Timer1ms_cnt;
extern hw_timer_t * Lfo_timer1;
extern hw_timer_t * Lfo_timer2;
extern hw_timer_t * timer_1ms;

extern float **selectedWaveLfo1;
extern float **selectedWaveLfo2;
#endif

LFO_EXTRN void Lfo_Process(stLfo* prlfo);