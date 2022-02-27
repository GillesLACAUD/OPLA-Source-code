
#include <Arduino.h>

#ifdef __DISTO__
#define DISTO_EXTRN
#else
#define DISTO_EXTRN extern
#endif

DISTO_EXTRN uint8_t Decimator;

DISTO_EXTRN void Distortion(float* sol,float* sor);
