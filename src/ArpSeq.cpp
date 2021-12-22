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
uint8_t i, j, tmp, index;    

    if(u8_ArpHold && u8_ArpNextNbKeyOn)
    {
        for (i=0; i < u8_ArpNextNbKeyOn; i++)
                st_TabArpKeys[i]=st_TabNextArpKeys[i];
        u8_ArpNbKeyOn=u8_ArpNextNbKeyOn;
        u8_ArpNextNbKeyOn=0;
    }
    for (i=0; i < u8_ArpNbKeyOn; i++)
    {
        u8_ArpTabFilterKeys[i] = st_TabArpKeys[i].note;
        u8_ArpTabFilterKeysVel[i] = st_TabArpKeys[i].vel;
    }


    if(u8_ArpMode !=ARP_MODE_ORDER)
    {
        for (i=0; i < (u8_ArpNbKeyOn-1); i++)
        {
            index = i;
            for (j=i+1; j < u8_ArpNbKeyOn; j++)
            {
                if (u8_ArpTabFilterKeys[index] > u8_ArpTabFilterKeys[j])
                index = j;
            }
            if (index != i)
            {
                tmp = u8_ArpTabFilterKeys[i];
                u8_ArpTabFilterKeys[i] = u8_ArpTabFilterKeys[index];
                u8_ArpTabFilterKeysVel[i] = u8_ArpTabFilterKeysVel[index];
                u8_ArpTabFilterKeys[index] = tmp;
            }
        }
    }
    Arp_Filter_Print();

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
    
    //Arp_Filter_Note();

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
    			Arp_Filter_Note();
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
			Arp_Filter_Note();
			u8_ArpCptStep=u8_ArpNbKeyOn;
		}
        u8_ArpCptStep+=i8_ArpWay;
		break;        

        case ARP_MODE_INC:
        if(u8_ArpUpDwn==ARP_UP)
        {
		    if(u8_ArpCptStep>=u8_ArpNbKeyOn-1)
		    {
    			Arp_Filter_Note();
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
    			Arp_Filter_Note();
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
                    Arp_Filter_Note();
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
                    Arp_Filter_Note();
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
            Arp_Filter_Note();
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
            Arp_Filter_Note();
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
        //Serial.printf("Cpt %03d Mode %d Key On %03d Max %03d\n",u8_ArpCptStep,u8_ArpUpDwn,u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpNbKeyOn);
    }

    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpTabFilterKeysVel[u8_ArpCptStep]);
    else
        Synth_MonoNoteOn(u8_ArpTabFilterKeys[u8_ArpCptStep],u8_ArpTabFilterKeysVel[u8_ArpCptStep]);

    return(0);
}
