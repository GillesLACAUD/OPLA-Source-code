#include <Arduino.h>

#ifdef __ARPSEQ__
#define ARP_EXTRN
#else
#define ARP_EXTRN   extern
#endif

#define TABARPDIVMIN        0
#define TABARPDIVMAX        1
#define TABARPDIVDELTA      2


#ifdef __ARPSEQ__
uint16_t TabDiv[8][3]={
 // Min  Max  Delta    
    2000,250, 2000-250,
    1000,125, 1000-125,
    500 , 65, 500-65,
    250 , 32, 250-32,
    2000,250, 2000-250,
    1000,125, 1000-125,
    500 , 65, 500-65,
    250 , 32, 250-32,
};
#else
extern uint16_t TabDiv[8][3];
#endif


ARP_EXTRN uint8_t u8_ArpTrig;
ARP_EXTRN uint8_t u8_ArpSpeed;
ARP_EXTRN uint8_t u8_ArpOn;
ARP_EXTRN uint8_t u8_ArpHold;
ARP_EXTRN uint8_t u8_ArpDiv;
ARP_EXTRN uint8_t u8_ArpMode;

ARP_EXTRN uint32_t u32_ArpTime;
ARP_EXTRN uint32_t u32_ArpTimeOff;

#define MAX_ARP_DELAY_HITKEYS		25
ARP_EXTRN volatile uint8_t u8_ArpCptHitKey;					            // Wait x ms before start seq to wait all hit keys

#define MAX_ARP_KEYS		127
#define MAX_ARP_FLT_KEYS	20

#define ARP_UP              0
#define ARP_DOWN            1

ARP_EXTRN uint8_t u8_ArpTabKeys[MAX_ARP_KEYS];					// Store all the key on with an order
ARP_EXTRN uint8_t u8_ArpTabKeysVel[MAX_ARP_KEYS];				// Store all the key velocity 
ARP_EXTRN uint8_t u8_ArpTabFilterKeys[MAX_ARP_FLT_KEYS];	    // Place the key in order
ARP_EXTRN uint8_t u8_ArpTabFilterKeysVel[MAX_ARP_FLT_KEYS];	    // Place the key in order
ARP_EXTRN uint8_t u8_ArpNbKeyOn;								// Cpt for nb key on
ARP_EXTRN int8_t  u8_ArpCptStep;								// Cpt for current step playing
ARP_EXTRN uint8_t u8_ArpUpDwn;								    // Up or Down
ARP_EXTRN int8_t  i8_ArpWay;	
ARP_EXTRN int8_t  u8_ArpRepeat;	    							// 

#define ARP_MODE_UP         0
#define ARP_MODE_DWN        1
#define ARP_MODE_INC        2
#define ARP_MODE_EXC        3
#define ARP_MODE_RND        4
#define ARP_MODE_ORDER      5
#define ARP_MODE_UP2        6
#define ARP_MODE_DWN2       7

ARP_EXTRN uint8_t Arp_Filter_Note();
ARP_EXTRN uint8_t Arp_Play_Note();
ARP_EXTRN uint8_t Arp_Stop_Note();
ARP_EXTRN uint8_t Arp_Filter_Print();

