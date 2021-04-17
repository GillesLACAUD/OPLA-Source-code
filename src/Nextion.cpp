#include "Arduino.h"
#include "typdedef.h"

#define __NEXTION__
#include "Nextion.h"

#include "Ihm.h"

#include "midi_interface.h"
#include "easysynth.h"
#include "Lfo.h"

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
unsigned char Nextion_Send(char *ptmess)
{
uint8_t m;
    
    if(1)
    {
        for(uint8_t m=0;m<MAX_MESS_TASK0;m++)
        {
            if(messnexfree[m]==0)
            {
                sprintf(messnextask[m],ptmess);    
                messnexfree[m]=1;
                m=88;
            }
        }
        if(m>=MAX_MESS_TASK0)
        {
            while(1)
                Serial.printf("MESS TASK OVER`\n");
        }
    }
    else
    {
        /*
        Serial1.print(ptmess);
        Serial1.write(0xff);
        Serial1.write(0xff);
        Serial1.write(0xff);
        */
    }
    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_Init()
{    
    Serial1.begin(115200, SERIAL_8N1, RXD_NEX, TXD_NEX);
    Nextion_Send("rest");
    Nextion_Send("rest");
    delay(2000);

    //sprintf(messnex,"touch_j");
	//Nextion_Send(messnex);

    sprintf(messnex,"page0.Setup_Name.txt=%cESP32%c",0x22,0x22);
    Nextion_Send(messnex);
    sprintf(messnex,"page0.CCInfo.txt=%cAUDIO%c",0x22,0x22);
    Nextion_Send(messnex);
    sprintf(messnex,"page0.CCVal.txt=%cKIT%c",0x22,0x22);
    Nextion_Send(messnex);

    Nextion_PrintLabel();

}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_Process()
{
   
    if (Serial1.available())
	{
    	if(!Nextion_Cmd_Receive)
    	{
			Nextion_Mess[cptm]=Serial1.read();

            //Serial.printf("%02X-",Nextion_Mess[cptm]);

			if(Nextion_Mess[cptm]==0xF3 && !Nextion_Begin_Receive )
			{
				cptm=0;
				Nextion_Mess[cptm]=0x01;
				Nextion_Begin_Receive=1;
			}
			if(Nextion_Mess[cptm]==0xF5 && cptm==5)
			{
				Nextion_Cmd_Receive=1;
				Nextion_Begin_Receive=0;
				cptm=0;
                //Serial.printf("END\n");
			}
			cptm++;
			if(cptm==30)
				cptm=0;
    	}
	}
    if(Nextion_Cmd_Receive)
	{
		Nextion_Parse();
	}
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_PrintCC(uint8_t cc,int data,uint8_t source)
{
static uint8_t oldcc;    
uint8_t s,e;

    //-------------------------------------------
    // Update the overview panel
    //-------------------------------------------

    //Serial.printf("oldcc %d newcc %d\n",oldcc,cc);

    if(oldcc != cc)
    {
        //----------------------------
        // Update the Name
        //----------------------------
        // From Nextion
        if(source==1)
        {
            sprintf(messnex,"page1.CCInfo.txt=%c%s%c",0x22,Tab_Encoder[gui_Section][gui_Param].LgName,0x22);
        }
        // From Midi
        else
        {
            // Search the CC
            for(s=0;s<MAX_SECTION;s++)
            {
                for(e=0;e<MAX_ENCODER;e++)
                {
                    if(Tab_Encoder[s][e].MidiCC==cc)
                    {
                        sprintf(messnex,"page1.CCInfo.txt=%c%s%c",0x22,Tab_Encoder[s][e].LgName,0x22);
                        //Serial.printf("CC %03d Section %d pot %d Name %s\n",cc,s,e,Tab_Encoder[s][e].LgName);
                        gui_Section = s;    // Update the section
                        goto trouve;
                    }
                }           
            }
        }
        trouve:
        Nextion_Send(messnex);  
    }
    oldcc = cc;
    //----------------------------
    // Update the Value
    //----------------------------
    for(e=0;e<MAX_ENCODER;e++)
    {
        if(Tab_Encoder[gui_Section][e].MidiCC==cc)
        {
            if(Tab_Encoder[gui_Section][e].Type==TYPE_DATA)
            {
                sprintf(messnex,"page1.CCVal.txt=%c%03d%c",0x22,data,0x22);
                Nextion_Send(messnex);
                
            }
            else
            {
                sprintf(messnex,"page1.CCVal.txt=%c%s%c",0x22,Tab_Encoder[gui_Section][e].ptTabList+MAX_LABEL*Tab_Encoder[gui_Section][e].Index,0x22);
                Nextion_Send(messnex);
            }
            e=0x55;
            s=0x55;
        }
    }   
}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_PrintLabel()
{
    sprintf(messnex,"page0.ts0.txt=%c%s%c",0x22,Tab_Section_Name[gui_Section],0x22);
    Nextion_Send(messnex);

    for(uint8_t l=0;l<NEXTION_MAX_LABEL;l++)
    {
        sprintf(messnex,"page0.la%d.txt=%c%s%c",l,0x22,Tab_Encoder[gui_Section][l].Name,0x22);
        Nextion_Send(messnex);
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_PrintValues()
{
char msec[30];
  
    for(uint8_t l=0;l<NEXTION_MAX_LABEL;l++)
    {
        if(Tab_Encoder[gui_Section][l].Type==TYPE_DATA)
        {
            sprintf(msec,"%03d",Tab_EncoderVal[gui_Section][l]);   
            Nextion_PotTxt(l,msec);
        }
        else
        {
            sprintf(msec,"%s",Tab_Encoder[gui_Section][l].ptTabList+MAX_LABEL*Tab_Encoder[gui_Section][l].Index);
            Nextion_PotTxt(l,msec);
        }
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_PotValue(uint8_t value)
{
	sprintf(messnex,"page1.CCPot.val=%d",value);
    Nextion_Send(messnex);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_PotTxt(uint8_t pot,char* ms)
{
    sprintf(messnex,"page0.se%d.txt=%c%s%c",pot,0x22,ms,0x22);
    Nextion_Send(messnex);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Nextion_Parse()
{
int val;
int cas;


    // NEXTION SIDE
    //  prints 0xF3,1
    //  print "T"
    //  prints 0x02,1
    //  prints Trem_Rate.val,1
    //  prints 0x00,1
    //  prints 0xF5,1

    //  Nextion_Mess[0]     Begin Frame
    //  Nextion_Mess[1]     Char ident
    //  Nextion_Mess[2]     Int number
    //  Nextion_Mess[3]     Value LSB
    //  Nextion_Mess[4]     Value MSB
    //  Nextion_Mess[4]     End MSB

    // F3 'O' 1 68 0 F5     Change value pot Oscillors 1 value 68
	
	val = Nextion_Mess[4]*255+Nextion_Mess[3];
	Nextion_Cmd_Receive=0;
    
	cas = (int)Nextion_Mess[1];
  	switch(cas)
	{
        // V Section inc or dec
		case 0x56:
        if(Nextion_Mess[2]==0)
        {
            if(gui_Section)
                gui_Section--;
            else
                gui_Section=MAX_SECTION-1;
        }
		if(Nextion_Mess[2]==4)
        {
            if(gui_Section<(MAX_SECTION-1))
                gui_Section++;
            else
                gui_Section=0;
        }
        Nextion_PrintLabel();
        Nextion_PrintValues();
		break;

        // L Select a parameter
		case 0x4C:
        gui_Param = Nextion_Mess[2];
        uint8_t cc = Tab_Encoder[gui_Section][gui_Param].MidiCC;
        if(cc !=0xFF)
        {
            Nextion_PrintCC(cc,0,1);
            sprintf(messnex,"page 1");
            Nextion_Send(messnex);
            //delay(2);
        }
        break;
	}
}