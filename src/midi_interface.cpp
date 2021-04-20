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


inline void Midi_NoteOn(uint8_t note)
{
    if(note)
        Synth_NoteOn(note);
    else
    {
        Midi_Dump();
    }
}

inline void Midi_NoteOff(uint8_t note)
{
    Synth_NoteOff(note);
}

/*
 * this function will be called when a control change message has been received
 */
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
    /*
    switch(data1)
    {

        case MIDI_CC_WAVE1: 
        case MIDI_CC_DETUNE:   
        case MIDI_CC_SUBOSC:                        
        case MIDI_CC_NOISE:                        
        
        case MIDI_CC_CUTOFF:  
        case MIDI_CC_RES:   
        case MIDI_CC_FOLLOW:        
        case MIDI_CC_AMP_A:         
        case MIDI_CC_AMP_D:         
        case MIDI_CC_AMP_S:         
        case MIDI_CC_AMP_R:         
        case MIDI_CC_FLT_A:         
        case MIDI_CC_FLT_D:         
        case MIDI_CC_FLT_R:         
        case MIDI_CC_FLT_Q:         
        case MIDI_CC_DEL_LENGHT:    
        case MIDI_CC_DEL_LEVEL:     
        case MIDI_CC_DEL_FEEDBACK:  
        case MIDI_CC_LFO1_SPEED:      
        case MIDI_CC_LFO1_SHAPE:      
        case MIDI_CC_LFO1_DEST:       
        case MIDI_CC_LFO1_AMT:         
        case MIDI_CC_LFO2_SPEED:      
        case MIDI_CC_LFO2_SHAPE:      
        case MIDI_CC_LFO2_DEST:       
        case MIDI_CC_LFO2_AMT:         
        case MIDI_CC_WS1:         
        case MIDI_CC_PAGE1_5:         
        Synth_SetRotary(data1,data2);
        Nextion_PrintCC(data1,data2,0);
        break;
    }
    */

}

/*
 * function will be called when a short message has been received over midi
 */
inline void HandleShortMsg(uint8_t *data)
{
    uint8_t ch = data[0] & 0x0F;

    switch (data[0] & 0xF0)
    {
    /* note on */
    case 0x90:
        Midi_NoteOn(data[1]);
        break;
    /* note off */
    case 0x80:
        Midi_NoteOff(data[1]);
        break;
    case 0xb0:
        Midi_ControlChange(ch, data[1], data[2]);
        break;
    }
}

void Midi_Setup()
{
    Serial2.begin(31250, SERIAL_8N1, RXD2, TXD2);
    pinMode(RXD2, INPUT_PULLUP);  /* 25: GPIO 16, u2_RXD */
}
/*
 * this function should be called continuously to ensure that incoming messages can be processed
 */
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

