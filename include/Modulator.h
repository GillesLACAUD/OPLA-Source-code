#include <Arduino.h>

#ifdef __MODULATOR__
#define MOD_EXTRN
#else
#define MOD_EXTRN   extern
#endif


#define  MOD_AMP        0
#define  MOD_CUTOFF     1
#define  MOD_NOISE      2
#define  MOD_PAN        3
#define  MOD_WS1        4
#define  MOD_WS2        5
#define  MOD_LSPEED1    6
#define  MOD_LAMT1      7
#define  MOD_LSPEED2    8
#define  MOD_LAMT2      9
#define  MOD_MAX        10



#ifdef __MODULATOR__
char ModName[MOD_MAX][5] = 
{"AMP","CUT","NOI","PAN","WS1","WS2","LS1","LA1","LS2","LA2"};
#else
extern char ModName[MOD_MAX][5];
#endif

MOD_EXTRN uint8_t ui8_ModWheelDest;  
MOD_EXTRN uint8_t ui8_AfterTouchDest;

MOD_EXTRN float ModWheelAmount;
MOD_EXTRN float AfterTouchAmount;

MOD_EXTRN float ModWheelValue;
MOD_EXTRN float AfterTouchValue;

MOD_EXTRN void ModWheel_Process();
MOD_EXTRN void AfterTouch_Process();