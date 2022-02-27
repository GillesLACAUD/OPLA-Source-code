
#include <Arduino.h>

#ifdef __DISTO__
#define DISTO_EXTRN
#else
#define DISTO_EXTRN extern
#endif

DISTO_EXTRN    float wetdrydec;           // For the decimator ...ok it is not a good place :-)
DISTO_EXTRN    float panlrdec;              // For the decimator ...ok it is not a good place :-)
DISTO_EXTRN    int16_t trash;              // For the decimator ...ok it is not a good place :-)

DISTO_EXTRN void Distortion(float* sol,float* sor);
