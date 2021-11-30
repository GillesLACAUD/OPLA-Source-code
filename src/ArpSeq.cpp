#include "Arduino.h"
#include "typdedef.h"

#define __ARPSEQ__
#include "ArpSeq.h"

#include "easysynth.h"

uint8_t Arp_Debug=1;

/***************************************************/
/* Only for debug                                  */
/*                                                 */
/*                                                 */
/***************************************************/
uint8_t Arp_Filter_Print()
{
    Serial.printf("ARP FILTER : ");
    for(uint8_t i =0;i<u8_ArpNbKeyOn;i++)
    {
        Serial.printf("%03d ",u8_ArpTabFilterKeys[!u8_ArpCurrenttab][i]);
    }
    Serial.printf("\n");
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
// Re arrange a new u8_ArpTabFilterKeys on every note on or off
uint8_t Arp_Filter_Note()
{
uint8_t cptparse=0;	
uint8_t cpttarget=0;	
	// Parse the ArpTabKeys to build a new u8_ArpTabFilterKeys
	for(cptparse=0;cptparse<MAX_ARP_KEYS;cptparse++)
	{
		if(u8_ArpTabKeys[cptparse])
		{
			// Arrange tab in order note
			if(WS.ArpMode == ARP_MODE_ORDER)
            {
				u8_ArpTabFilterKeys[!u8_ArpCurrenttab][cpttarget] = u8_ArpTabKeys[cptparse];
            }
			// Arrange tab from lower to higher note
			else
            {
				u8_ArpTabFilterKeys[!u8_ArpCurrenttab][cpttarget] = cptparse;
            }
			cpttarget++;
			if(cpttarget>u8_ArpNbKeyOn)
				return(0);
		}
	}
    Arp_Filter_Print();
    if(!u8_ArpNbKeyOn)
    {
        if(SoundMode !=SND_MODE_MONO)
            Synth_NoteOn(u8_ArpTabFilterKeys[!u8_ArpCurrenttab][u8_ArpCptStep],90);
        else
            Synth_MonoNoteOn(u8_ArpTabFilterKeys[!u8_ArpCurrenttab][u8_ArpCptStep],90);
    }
                
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
// Play the new note
uint8_t Arp_Play_Note()
{
	//PlayNote(u8_ArpTabFilterKeys[u8_ArpCptStep]);

    if(Arp_Debug)
    {
        Serial.printf("Cpt %03d Max %03d\n",u8_ArpCptStep,u8_ArpNbKeyOn);
    }
    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOff(u8_ArpTabFilterKeys[!u8_ArpCurrenttab][u8_ArpCptStep]);
    else
        Synth_MonoNoteOff(u8_ArpTabFilterKeys[!u8_ArpCurrenttab][u8_ArpCptStep]);

    u8_ArpWay=1;
	switch(WS.ArpMode)
	{
		case ARP_MODE_UP:
		u8_ArpCptStep+=u8_ArpWay;
		if(u8_ArpCptStep>=u8_ArpNbKeyOn)
			u8_ArpCptStep=0;
		break;
		case ARP_MODE_DWN:
		u8_ArpCptStep+=u8_ArpWay;
		if(u8_ArpCptStep==0)
			u8_ArpCptStep=u8_ArpNbKeyOn;
		break;
		case ARP_MODE_EXC:
		u8_ArpCptStep+=u8_ArpWay;
		if(u8_ArpCptStep>u8_ArpNbKeyOn)
			u8_ArpWay = -1;
		if(u8_ArpCptStep==0)
		{
			u8_ArpWay = +1;
		}
		break;		
		case ARP_MODE_INC:
		break;		
		case ARP_MODE_ORDER:
		u8_ArpCptStep+=u8_ArpWay;
		if(u8_ArpCptStep>u8_ArpNbKeyOn)
			u8_ArpCptStep=0;
		break;		
		case ARP_MODE_UP2:
		break;		
		case ARP_MODE_DWN2:
		break;		
		case ARP_MODE_RND:
		break;		
	}

    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOn(u8_ArpTabFilterKeys[!u8_ArpCurrenttab][u8_ArpCptStep],90);
    else
        Synth_MonoNoteOn(u8_ArpTabFilterKeys[!u8_ArpCurrenttab][u8_ArpCptStep],90);

    return(0);
}
