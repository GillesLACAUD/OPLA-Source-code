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
    Serial.printf("ARP FILTER : Nb %3d->",u8_ArpNbKeyOn);
    for(uint8_t i =0;i<u8_ArpNbKeyOn;i++)
    {
        Serial.printf("%03d ",u8_ArpTabFilterKeys[i]);
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
				u8_ArpTabFilterKeys[cpttarget] = u8_ArpTabKeys[cptparse];
            }
			// Arrange tab from lower to higher note
			else
            {
				u8_ArpTabFilterKeys[cpttarget] = cptparse;
            }
			cpttarget++;
			if(cpttarget>u8_ArpNbKeyOn)
				return(0);
		}
	}
    Arp_Filter_Print();
    if(!u8_ArpNbKeyOn)
    {	
		// PLay the first note
        if(SoundMode !=SND_MODE_MONO)
            Synth_NoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],90);
        else
            Synth_MonoNoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],90);
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

    FlipPan = !FlipPan;
    if(Arp_Debug)
    {
        Serial.printf("Cpt %03d Max %03d\n",u8_ArpCptStep,u8_ArpNbKeyOn);
    }
    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOff(u8_ArpTabFilterKeys[u8_ArpCptStep]);
    else
        Synth_MonoNoteOff(u8_ArpTabFilterKeys[u8_ArpCptStep]);

    u8_ArpWay=1;
	switch(WS.ArpMode)
	{
		case ARP_MODE_UP:
		u8_ArpCptStep+=u8_ArpWay;
		if(u8_ArpCptStep>=u8_ArpNbKeyOn)
		{
			Arp_Filter_Note();
			u8_ArpCptStep=0;
		}
		break;
	}

    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],90);
    else
        Synth_MonoNoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],90);

    return(0);
}
