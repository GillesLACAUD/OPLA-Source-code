/*
 * a simple implementation to use midi
 *
 * Author: Marcel Licence/Gilles Lacaud
 */

#include <Arduino.h>
#include "typdedef.h"

#define __MIDI__
#include "midi_interface.h"
#include "easysynth.h"
#include "Nextion.h"
#include "Modulator.h"
#include "SDCard.h"
#include "Ihm.h"
#include "ArpSeq.h"

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL	0.007874f7
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Midi_Dump()
{
        Serial.printf("------- DUMP --------\n");
        for (int i = 0; i < MAX_POLY_VOICE ; i++)
        {
            Serial.printf("Actif %d Midi %03d Phase %d voc: %02d, osc: %02d Grank : %02d\n",voicePlayer[i].active,voicePlayer[i].midiNote,voicePlayer[i].phase,voc_act, osc_act,globalrank);
        }

        if(globalrank>4)
        {
            Serial.printf("ERROR\n");
            while(1);
        }

}

void PrintnoteArp()
{
uint8_t i;

    Serial.printf("NEXT--------------");
    for(i=0;i<u8_ArpNextNbKeyOn;i++)    
    {
        Serial.printf("%02d-%03d ",i,st_TabNextArpKeys[i].note);
    }
    Serial.printf("\n");
}

void GetnoteoffArp(uint8_t note)
{
uint8_t i;    
    // search the note
    for(i=0;i<u8_ArpNbKeyOn;i++)    
    {
        if(st_TabArpKeys[i].note==note)
        {
            st_TabArpKeys[i].note=0;
            break;
        }
    }
    for(i=i;i<u8_ArpNbKeyOn;i++)
    {
        st_TabArpKeys[i].note = st_TabArpKeys[i+1].note;
        st_TabArpKeys[i].vel = st_TabArpKeys[i+1].vel;
    }    
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Midi_NoteOn(uint8_t note,uint8_t vel)
{
uint8_t slotav=0;
uint8_t n=0;

    Midi_KeyOn++;
    if(note)
    {
        FlipPan = !FlipPan;
        if(u8_ArpOn)
        {
            if(!u8_ArpNbKeyOn)
            {
                u8_ArpCptHitKey=0;
                u8_ArpTrig=0;
            }
            if(!u8_ArpHold)
            {
                st_TabArpKeys[u8_ArpNbKeyOn].note   = note;
                st_TabArpKeys[u8_ArpNbKeyOn].vel    = vel;
                u8_ArpNbKeyOn++;                            
            }
            else
            {
                if(!u8_ArpTrig)
                {
                    st_TabArpKeys[u8_ArpNbKeyOn].note   = note;
                    st_TabArpKeys[u8_ArpNbKeyOn].vel    = vel;
                    u8_ArpNbKeyOn++;                            
                }
                else
                {
                    u8_ArpNextTrig=1;
                    st_TabNextArpKeys[u8_ArpNextNbKeyOn].note   = note;
                    st_TabNextArpKeys[u8_ArpNextNbKeyOn].vel    = vel;
                    u8_ArpNextNbKeyOn++;
                    //PrintnoteArp();
                }
            }
            return;
        }
        if(SoundMode !=SND_MODE_MONO)
        {
            Synth_NoteOn(note,vel);
        }
        else
        {
            Synth_MonoNoteOn(note,vel);
            MonoCptNote++;

            // Keep alway the last two notes
            if(MonoCptNote>=MONO_MAX_KEEP_NOTE)
            {
                Serial.printf("You play like a monkey MonoCptNote = %d Index %d\n",MonoCptNote,MonoIndexNote);                    

                //MonoKeepNote[0]=MonoKeepNote[1];
                //MonoKeepNote[1]=note;
            }
            else
            {
               		// search if a slot is available
                for(n=0;n<MonoCptNote;n++)
                {
                    if(MonoKeepNote[n]==0x00)
                    {
                        MonoKeepNote[n]=note;
                        MonoKeepVel[n]=vel;
                        slotav = 1;
                    }
                    if(!slotav)
                    {
                        MonoKeepNote[MonoIndexNote]=note;
                        MonoKeepVel[MonoIndexNote]=vel;
                    }
                }
            }
            if(n>MonoIndexNote)
		        MonoIndexNote++;
            //Midi_NotePrint(1,note,vel);
            sprintf(messnex,"page0.b2.txt=%c%d%c",0x22,MonoCptNote,0x22);
            Nextion_Send(messnex);
        }
    }
    //Serial.printf("--MonoCptNote  ON= %d Index %d\n",MonoCptNote,MonoIndexNote);                    
}





/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Midi_NoteOff(uint8_t note,uint8_t vel)
{
uint8_t n;
uint8_t offnumber;

    if(Midi_KeyOn)
        Midi_KeyOn--;

    if(u8_ArpOn)
    {
        if(u8_ArpHold)
        {
            //if(u8_ArpNextNbKeyOn)
            //    u8_ArpNextNbKeyOn--;
        }
        if(!u8_ArpHold)
        {
            GetnoteoffArp(note);
            if(u8_ArpNbKeyOn)
                u8_ArpNbKeyOn--;
            if(!u8_ArpNbKeyOn)
            {
                u8_ArpTrig=0;
                // Off all note
                for(uint8_t i=0;i<=MAX_ARP_FLT_KEYS;i++)
                {
                    if(SoundMode !=SND_MODE_MONO)
                        Synth_NoteOff(u8_ArpTabFilterKeys[i]);
                    else
                        Synth_MonoNoteOff(u8_ArpTabFilterKeys[i]);
                }
            }
        }
        //else
        //    PrintnoteArp();
        return;
    }
    if(SoundMode !=SND_MODE_MONO)
        Synth_NoteOff(note);
    else
    {
        MonoCptNote--;

        if(MonoKeepNote[MonoIndexNote-1]==note)
        {
            if(!MonoCptNote)
            {
                Synth_MonoNoteOff(MonoKeepNote[MonoIndexNote-1]);
                MonoKeepNote[MonoIndexNote]=0;
                MonoKeepVel[MonoIndexNote]=0;
                MonoIndexNote=0;
            }
            else
            {
                MonoIndexNote--;
                for(n=MonoIndexNote-1;n>=0;n--)
                {
                    if(MonoKeepNote[n]!=0)
                    {
                        Synth_MonoNoteOn(MonoKeepNote[n],MonoKeepVel[n]);
                        break;
                    }
                    MonoIndexNote--;
                    if(!MonoIndexNote)
                    {
                        Synth_MonoNoteOff(MonoKeepNote[MonoIndexNote]);
                        break;
                    }
                }
            }
        }
        else
        {

            if(!MonoCptNote)
            {
                Synth_MonoNoteOff(MonoKeepNote[MonoIndexNote-1]);
                MonoKeepNote[MonoIndexNote-1]=0;
                MonoKeepVel[MonoIndexNote-1]=0;
                MonoIndexNote=0;
            }
            else
            {
                // search the note
                for(uint8_t n=0;n<MONO_MAX_KEEP_NOTE;n++)
                {
                    if(MonoKeepNote[n]==note)
                    {
                        MonoKeepNote[n]=0;
                        MonoKeepVel[n]=0;
                        break;
                    }
                }
                
            }
        }
        //Serial.printf("--MonoCptNote OFF= %d Index %d\n",MonoCptNote,MonoIndexNote);                    
        sprintf(messnex,"page0.b2.txt=%c%d%c",0x22,MonoCptNote,0x22);
        Nextion_Send(messnex);
        if(MonoCptNote==0 && MonoIndexNote==0)
        {
            Serial.printf("--KILL ALL NOTE\n");                    
            for(n=0;n<MONO_MAX_KEEP_NOTE;n++)
                Synth_MonoNoteOff(MonoKeepNote[n]);
        }

        //Midi_NotePrint(0,note,vel);
    }
    
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Midi_PitchBend(uint8_t channel,uint8_t data1, uint8_t data2)
{
int16_t bend;

    // 14 bits
    // 0x3FFF 
    // https://www.pgmusic.com/forums/ubbthreads.php?ubb=showflat&Number=490405 

    pitchMultiplier = 1.0;
    bend = (uint16_t)data2<<8;
    bend +=(uint16_t)data1;

    // value from -2 to +2 semitones
    float value = ((float)bend - 8192.0f) * (0.5f / 8192.0f) - 0.5f;
    value *=WS.PBRange;      
    pitchMultiplier = pow(2.0f, value / 12.0f);

    //Serial.printf("Lsb %02x Msb %02x Bend %d Value %3.2f Pitch mul %3.2f\n",data1,data2,bend,value,pitchMultiplier);

}

void ChangePage(uint8_t cc)
{
    if(cc==MIDI_CC_BK || cc==MIDI_CC_WA)
        sprintf(messnex,"page 4");
    else
        sprintf(messnex,"page 2");
    Nextion_Send(messnex);            
}

void IRAM_ATTR ChangePot(uint8_t cc,int16_t va)
{
uint16_t size;
int16_t tmp;

    uint8_t notassign=0;
    notassign=Nextion_PrintCC(cc,va,0);
    if(!notassign)
    {
        if(Tab_Encoder[gui_Section][gui_Param].Type==TYPE_DATA)
        {
            size=(Tab_Encoder[gui_Section][gui_Param].MaxData - Tab_Encoder[gui_Section][gui_Param].MinData);
            va = va-Tab_Encoder[gui_Section][gui_Param].MinData;
            va *= 127;
            va /=size;
        }
        Nextion_PotValue(va);
    }
    else
    {
        sprintf(messnex,"page1.CCVal.txt=%c---%c",0x22,0x22);
        Nextion_Send(messnex);
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Midi_ControlChange(uint8_t channel, uint8_t data1, uint8_t data2)
{
static uint8_t RelCC=0;         // Get the CC 
static uint8_t BoolNewVal=0;    // A new val is present
static int newval;
int ret;
static uint8_t RelPhase=0;
static uint8_t incdecval=0;

    // Mod Wheel
    if(data1 == 1)
    {
        ModWheelValue = (float)data2/128.0;
        return;            
    }

    overcpt=0;

    //----------------------------------------------
    // OFF MODE
    //----------------------------------------------
    if(RealMidiMode==MIDI_MODE_OFF)
    {
        return;
    }

    //----------------------------------------------
    // ABSOLUT MODE
    //----------------------------------------------
    if(RealMidiMode==MIDI_MODE_ABS)
    {
        if(!overon)
        {
            overon = true;
            RelCC=data1;
            Nextion_PrintCC(data1,data2,0);
            ChangePage(RelCC);
        }
        RelCC=data1;
        ret =Synth_SetRotary(data1,data2);
        ChangePot(RelCC,ret);
        return;    
    }
    //----------------------------------------------
    // NRPN MODE
    //----------------------------------------------
    if(RealMidiMode==MIDI_MODE_NRPN)
    {
        if(data1==0x62)                          // CC LSB
        {
            RelCC=data2;
        }
        if(data1==0x26)                          // Value LSB
        {
            newval = data2;
            BoolNewVal=1;
        }
        if(data1==0x60)                          // Inc Value
        {
            newval=Synth_GetandSet(RelCC,1,1);
            BoolNewVal=1;
        }
        if(data1==0x61)                         // Dec Value
        {
            newval=Synth_GetandSet(RelCC,1,-1);
            BoolNewVal=1;
        }
        if(BoolNewVal==1)
        {
            ChangePot(RelCC,newval);
            BoolNewVal=0;
            if(!overon)
            {
                overon = true;
                ChangePage(RelCC);
            }
        }
        return;    
    }
    //----------------------------------------------
    // REL MODE
    //----------------------------------------------
    if(RealMidiMode==MIDI_MODE_REL1)
    {
        // Get the CC to change
        switch(RelPhase)
        {
            case 0:
            if(data2==MidiRelCC)
            {
                RelCC=data1;
                RelPhase=1;
            }
            break;
            case 1:
            if(data1==RelCC)            
            {
                if(data2 <= MidiRelMin)
                {
                    incdecval= 1+(MidiRelMin-data2);
                    newval=Synth_GetandSet(RelCC,incdecval,-1);
                }
                if(data2 >= MidiRelMax)
                {
                    incdecval= 1+(data2-MidiRelMax);
                    newval=Synth_GetandSet(RelCC,incdecval,1);
                }
                RelPhase=0;
                ChangePot(data1,newval);
            }
            if(!overon)
            {
                overon = true;
                ChangePage(RelCC);
            }
            break;
        }
        return;            
    }
}

// Sysex Midi clock and Real time
/****************************************************/
/*                                                  */
/*                                                  */
/*                                                  */
/*                                                  */
/****************************************************/
inline void HandleRealTimeMsg(uint8_t realtime)
{
uint8_t incomingByte;

  switch(realtime)
  {
    case MIDI_CLOCK:
    /*
    if(!gu8_MidiClock)
    {
      gu8_MidiClock=1;
      gu8_MidiCptClock=0;
    }
    gu16_TimerMidiClock=0;
    */
    break;
    
    case MIDI_SYSTEM_EXCLUSIVE:
    incomingByte=0x00;
    #ifdef DUMP_SERIAL2_TO_SERIAL
    Serial.printf("SYSEX 0xF0 ");
    #endif
    while(incomingByte !=0xF7)      // Wait end sysex
    {
      if (Serial2.available())
      {
        incomingByte = Serial2.read();
        #ifdef DUMP_SERIAL2_TO_SERIAL
        Serial.printf("0X%02X ",incomingByte);
        #endif
      }
    }
    #ifdef DUMP_SERIAL2_TO_SERIAL
    Serial.printf("\n");
    #endif
    break;
    
    case MIDI_START:
    break;
    
    case MIDI_STOP:
    break;

    case MIDI_SONGPOS:
    break;

    case MIDI_SONGSELECT:
    break;
    
  }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void HandleByteMsg(uint8_t *data)
{
    switch (data[0] & 0xF0)
    {
        // Aftertouch
        case 0xD0:
            AfterTouchValue = (float)data[1]/128.0;
            //Serial.printf("AT %d\n",data[1]);
            break;
        // Program changed
        case 0xC0:
            if(data[1]<100)
            {
                SDCard_LoadSound(data[1],1);
                Nextion_PrintValues();
                Serial.printf("Program Change %d\n",data[1]);
            }
            break;
    }
}
            

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void HandleShortMsg(uint8_t *data)
{
    uint8_t ch = data[0] & 0x0F;

    switch (data[0] & 0xF0)
    {
        case 0x90:
            if (data[2] > 0)
            {
                Midi_NoteOn(data[1]+WS.Transpose,data[2]);
            }
            else
            {
                Midi_NoteOff(data[1]+WS.Transpose,data[2]);
            }
            break;
        /* note off */
        case 0x80:
            Midi_NoteOff(data[1]+WS.Transpose,data[2]);
            break;
        /* Midi control change */
        case 0xb0:
            Midi_ControlChange(ch, data[1], data[2]);
            break;        
        case MIDI_PITCH_BEND:
            Midi_PitchBend(ch,data[1], data[2]);
            break;

    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Midi_Setup()
{
    Serial2.begin(31250, SERIAL_8N1, RXD2, TXD2);
    pinMode(RXD2, INPUT_PULLUP);  /* 25: GPIO 16, u2_RXD */
}

//#define DUMP_SERIAL2_TO_SERIAL
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Midi_Process()
{
    /*
     * watchdog to avoid getting stuck by receiving incomplete or wrong data
     */
    static uint32_t inMsgWd = 0;
    static uint8_t inMsg[3];
    static uint8_t inMsgIndex = 0;
    static uint8_t lenMsg=3;
    static uint8_t Msg;
    static uint8_t mrx;

    //Choose Serial1 or Serial2 as required

    if (Serial2.available())
    {
        uint8_t incomingByte = Serial2.read();

        #ifdef DUMP_SERIAL2_TO_SERIAL 
        if(incomingByte != 0xFE)
            Serial.printf("%02x\n", incomingByte);
        #endif

        /* System or real time messages */
        if ((incomingByte >= 0xF0))
        {
            // Active sensing and time clock
            if(incomingByte!=0xFE && incomingByte!=0xF8)
                inMsg[0]=0xFF;
            HandleRealTimeMsg(incomingByte);
            return;
        }
        
        if(incomingByte & 0x80)
        {
            Msg=incomingByte & 0xF0;
            
            mrx=incomingByte & 0x0F;
            
            if((mrx+1)!=MidiRx)
            {
                inMsgIndex=-1;
                return;
            }
            inMsgIndex=0;
            switch(Msg)
            {
                case 0xD0:
                case 0xC0:
                lenMsg=2;
                break;
                
                case 0x80:
                case 0xB0:
                case 0x90:
                case 0xE0:
                lenMsg=3;
                break;
            }
        }

        if((mrx+1)!=MidiRx)
        {
            return;
        }

        if (inMsgIndex == 0)
        {
            if ((incomingByte & 0x80) != 0x80)
            {
                inMsgIndex = 1;
            }
        }

        inMsg[inMsgIndex] = incomingByte;
        inMsgIndex += 1;

        if (lenMsg==2 && inMsgIndex >= 2)
        {
            HandleByteMsg(inMsg);
            inMsgIndex = 0;
        }
        if (lenMsg==3 && inMsgIndex >= 3)
        {
            #ifdef DUMP_SERIAL2_TO_SERIAL
            Serial.printf(">%02x %02x %02x\n", inMsg[0], inMsg[1], inMsg[2]);
            #endif
            HandleShortMsg(inMsg);
            inMsgIndex = 0;
        }
        /*
         * reset watchdog to allow new bytes to be received
         */
        inMsgWd = 0;
    }
    else
    {
        if (inMsgIndex > 0)
        {
            inMsgWd++;
            if (inMsgWd == 0xFFF)
            {
                inMsgIndex = 0;
            }
        }
    }

}

