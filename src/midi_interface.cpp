/*
 * a simple implementation to use midi
 *
 * Author: Marcel Licence
 */

#include <Arduino.h>
#include "typdedef.h"

#define __MIDI__
#include "midi_interface.h"
#include "easysynth.h"
#include "Nextion.h"


/* use define to dump midi data */
//#define DUMP_SERIAL2_TO_SERIAL

/* constant to normalize midi value to 0.0 - 1.0f */
#define NORM127MUL	0.007874f
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

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Midi_NoteOn(uint8_t note,uint8_t vel)
{
    if(note)
        Synth_NoteOn(note,vel);
    else
    {
        Midi_Dump();
    }
}
/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Midi_NoteOff(uint8_t note,uint8_t vel)
{
    Synth_NoteOff(note);
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

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
inline void Midi_ControlChange(uint8_t channel, uint8_t data1, uint8_t data2)
{
    if(!overon)
    {
        overon = true;
        /*
        sprintf(messnex,"vis CCInfo,1");
        Nextion_Send(messnex);
        sprintf(messnex,"vis CCVal,1");
        Nextion_Send(messnex);
        */
       Nextion_PrintCC(data1,data2,0);
       sprintf(messnex,"page 1");
       Nextion_Send(messnex);
       //delay(10);
       
    }
    overcpt=0;
    // get the new value data2 midi to data2 min/max 
    int ret=Synth_SetRotary(data1,data2);
    Nextion_PotValue(data2);
    Nextion_PrintCC(data1,ret,0);
    
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
                Midi_NoteOn(data[1],data[2]);
            }
            else
            {
                Midi_NoteOff(data[1],data[2]);
            }
            break;
        /* note off */
        case 0x80:
            Midi_NoteOff(data[1],data[2]);
            break;
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

    //Choose Serial1 or Serial2 as required

    if (Serial2.available())
    {
        uint8_t incomingByte = Serial2.read();

#ifdef DUMP_SERIAL2_TO_SERIAL 
        Serial.printf("%02x\n", incomingByte);
#endif
        /* ignore live messages */
        if ((incomingByte & 0xF0) == 0xF0)
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

        if (inMsgIndex >= 3)
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

