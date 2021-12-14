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
			if(u8_ArpMode == ARP_MODE_ORDER)
            {
				u8_ArpTabFilterKeys[u8_ArpTabKeys[cptparse]-1] = cptparse;
                u8_ArpTabFilterKeysVel[u8_ArpTabKeys[cptparse]-1]=u8_ArpTabKeysVel[cptparse];
            }
			// Arrange tab from lower to higher note
			else
            {
				u8_ArpTabFilterKeys[cpttarget] = cptparse;
                u8_ArpTabFilterKeysVel[cpttarget]=u8_ArpTabKeysVel[cptparse];
            }
			cpttarget++;
			if(cpttarget>u8_ArpNbKeyOn)
				return(0);
		}
	}
    //Arp_Filter_Print();
    if(!u8_ArpNbKeyOn)
    {	
		// PLay the first note
        if(SoundMode !=SND_MODE_MONO)
            Synth_NoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpTabFilterKeysVel[u8_ArpCptStep]);
        else
            Synth_MonoNoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpTabFilterKeysVel[u8_ArpCptStep]);
    }
                
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
// Stop the new note
uint8_t Arp_Stop_Note()
{
    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOff(u8_ArpTabFilterKeys[u8_ArpCptStep]);
    else
        Synth_MonoNoteOff(u8_ArpTabFilterKeys[u8_ArpCptStep]);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
// Play the new note
uint8_t Arp_Play_Note()
{
long rnd;    
    FlipPan = !FlipPan;
    
    Arp_Filter_Note();

	switch(u8_ArpMode)
	{
        case ARP_MODE_RND:
        rnd=random(u8_ArpNbKeyOn);
        u8_ArpCptStep=(uint8_t)rnd;
        break;        

		case ARP_MODE_UP:
        case ARP_MODE_ORDER:
        if(u8_ArpUpDwn==ARP_UP)
        {
		    if(u8_ArpCptStep>=u8_ArpNbKeyOn-1)
		    {
    			//Arp_Filter_Note();
                i8_ArpWay=+1;
                u8_ArpCptStep=0;
		    }
            else
                u8_ArpCptStep+=i8_ArpWay;
        }     
		break;
		case ARP_MODE_DWN:
		if(u8_ArpCptStep==0)
		{
			//Arp_Filter_Note();
			u8_ArpCptStep=u8_ArpNbKeyOn;
		}
        u8_ArpCptStep+=i8_ArpWay;
		break;        

        case ARP_MODE_INC:
        if(u8_ArpUpDwn==ARP_UP)
        {
		    if(u8_ArpCptStep>=u8_ArpNbKeyOn-1)
		    {
    			//Arp_Filter_Note();
                i8_ArpWay=-1;
                u8_ArpCptStep =u8_ArpNbKeyOn-1;
                if(u8_ArpRepeat==1)
                {
                    u8_ArpRepeat=0;
                    u8_ArpUpDwn=ARP_DOWN;
                }
                u8_ArpRepeat++;
		    }
            else
                u8_ArpCptStep+=i8_ArpWay;
        }
        else
        {
            if(u8_ArpCptStep<=0)
		    {
    			//Arp_Filter_Note();
                i8_ArpWay=+1;
                u8_ArpCptStep=0;
                if(u8_ArpRepeat==1)
                {
                    u8_ArpRepeat=0;
                    u8_ArpUpDwn=ARP_UP;
                }
                u8_ArpRepeat++;
		    }
            else
                u8_ArpCptStep+=i8_ArpWay;
        }
        break;

        case ARP_MODE_EXC:
        if(u8_ArpNbKeyOn!=1)
        {
            if(u8_ArpUpDwn==ARP_UP)
            {
                if(u8_ArpCptStep>=u8_ArpNbKeyOn-1)
                {
                    //Arp_Filter_Note();
                    u8_ArpUpDwn=ARP_DOWN;
                    i8_ArpWay=-1;
                    u8_ArpCptStep =u8_ArpNbKeyOn-2;
                }
                else
                {
                    u8_ArpCptStep+=i8_ArpWay;
                }
            }
            else
            {
                if(u8_ArpCptStep<=0)
                {
                    //Arp_Filter_Note();
                    u8_ArpUpDwn=ARP_UP;
                    i8_ArpWay=+1;
                    u8_ArpCptStep=1;
                }
                else
                    u8_ArpCptStep+=i8_ArpWay;
            }
        }
        else
            u8_ArpCptStep=0;
        break;

		case ARP_MODE_UP2:
        if(u8_ArpCptStep>=u8_ArpNbKeyOn-1)
        {
            //Arp_Filter_Note();
            i8_ArpWay=+1;
            if(u8_ArpRepeat==2)
            {
                u8_ArpRepeat=0;
                u8_ArpCptStep=0;
            }
            u8_ArpRepeat++;
        }
        else
        {
            if(u8_ArpRepeat==2)
            {
                u8_ArpRepeat=0;
                u8_ArpCptStep+=i8_ArpWay;
            }
            u8_ArpRepeat++;
        }
		break;

		case ARP_MODE_DWN2:
        if(u8_ArpCptStep<=0)
        {
            //Arp_Filter_Note();
            i8_ArpWay=-1;
            if(u8_ArpRepeat==2)
            {
                u8_ArpRepeat=0;
                u8_ArpCptStep=u8_ArpNbKeyOn-1;
            }
            u8_ArpRepeat++;
        }
        else
        {
            if(u8_ArpRepeat==2)
            {
                u8_ArpRepeat=0;
                u8_ArpCptStep+=i8_ArpWay;
            }
            u8_ArpRepeat++;
        }
		break;        
	}
    if(Arp_Debug)
    {
        Serial.printf("Cpt %03d Mode %d Key On %03d Max %03d\n",u8_ArpCptStep,u8_ArpUpDwn,u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpNbKeyOn);
    }

    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpTabFilterKeysVel[u8_ArpCptStep]);
    else
        Synth_MonoNoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpTabFilterKeysVel[u8_ArpCptStep]);

    return(0);
}
