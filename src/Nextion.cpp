#include "Arduino.h"
#include "typdedef.h"

#define __NEXTION__
#include "Nextion.h"

#include "Ihm.h"

#include "midi_interface.h"
#include "easysynth.h"
#include "Lfo.h"
#include "SDCard.h"

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
unsigned char Nextion_Send(char *ptmess)
{
uint8_t m=0;
    
    if(1)
    {
        for(uint8_t m=0;m<MAX_MESS_TASK0;m++)
        {
            if(messnexfree[m]==0)
            {
                sprintf(messnextask[m],ptmess);    
                messnexfree[m]=1;
                //Serial.printf("MESS %d %s\n",m,messnextask[m]);
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
    delay(1000);

    sprintf(messnex,"page 1");
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
			if(Nextion_Mess[cptm]==0xF5 && Nextion_Begin_Receive)
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
uint8_t Nextion_PrintCC(uint8_t cc,int data,uint8_t source)
{
static uint8_t oldcc=0xFF;    
uint8_t s,e;
static uint8_t notassign=0;

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
            sprintf(messnex,"page1.CCInfo.txt=%c%03d %s%c",0x22,cc,Tab_Encoder[gui_Section][gui_Param].LgName,0x22);
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
                        sprintf(messnex,"page1.CCInfo.txt=%c%03d %s%c",0x22,cc,Tab_Encoder[s][e].LgName,0x22);
                        //Serial.printf("ENCODER FIND %s\n",messnex);
                        gui_Section = s;    // Update the section
                        gui_Param = e;
                        notassign=0;
                        goto trouve;
                    }
                }           
            }
            sprintf(messnex,"page1.CCInfo.txt=%c%03d %s%c",0x22,cc,"NOT ASSIGN",0x22);
            notassign=1;
        }
        trouve:
        //Serial.printf("ENCODER FIND %s\n",messnex);
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
    return(notassign);
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
int tmp;
uint8_t range;
float factor;
int16_t newval=0;
  
    for(uint8_t l=0;l<NEXTION_MAX_LABEL;l++)
    {
        if(Tab_Encoder[gui_Section][l].Type==TYPE_DATA)
        {
            newval = *Tab_Encoder[gui_Section][l].Data;
            range = Tab_Encoder[gui_Section][l].MaxData-Tab_Encoder[gui_Section][l].MinData;
            factor = (float)range/127;
            newval = Tab_Encoder[gui_Section][l].MinData + (int)((float)newval*factor);
            sprintf(msec,"%03d",newval);   
            Nextion_PotTxt(l,msec);
        }
        else
        {
            tmp = (*Tab_Encoder[gui_Section][l].Data)*Tab_Encoder[gui_Section][l].Step*NORM127MUL;
            Tab_Encoder[gui_Section][l].Index = tmp;
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
void Nextion_Plot()
{
int s;
uint8_t tabplot[256];
struct oscillatorT *osc;

    // Built the tab
    s=0;
    for(int16_t i=0;i<254;i++)
    {
        tabplot[i] = (uint8_t)((waveAKWF[s]+1.0)*64);
        s+=4;
    }

    sprintf (messnex,"addt 3,0,253");
    //sprintf (messnex,"addt 3,0,254/4");
    Serial1.printf(messnex);
    Serial1.write(0xff);
    Serial1.write(0xff);
    Serial1.write(0xff);
    delayMicroseconds(100);

    for(int16_t value=253;value>=0;value--)
    //for(int16_t value=254/4;value>=0;value--)
    {
        Serial1.write(tabplot[value]);
    }
    Serial1.write(0xff);
    Serial1.write(0xff);
    Serial1.write(0xff);
    selectedWaveForm = &waveAKWF[0];
    for(uint8_t o=0;o<=osc_act;o+=3)
    {
        osc = &oscPlayer[o+0];
        osc->waveForm = selectedWaveForm;

        osc = &oscPlayer[o+1];
        osc->waveForm = selectedWaveForm;
    }
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
int ret;
uint8_t cc;


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

    // W Select bank/wave AKWF
    // X Select sound
    // Y Select bank/wave AKWF inc/dec
    // S Select section
    // V Section inc/dec
	// P Value of a parameter
    // L Select a parameter
    // Q Quit
    // N Name sound
    // D Draw AKWF
    // A Select sound
    // B Confirm save yes/no    B0 yes B1 No


	val = Nextion_Mess[4]*255+Nextion_Mess[3];
	Nextion_Cmd_Receive=0;
    
	cas = (int)Nextion_Mess[1];
    //Serial.printf("NEXTION %02X\n",cas);
  	switch(cas)
	{

        // A Select sound
        // Select page always in the Nextion IHM
        case 0x41:      
        SDCard_Display10SndName();
        sprintf(messnex,"page2.b%d.bco=65535",CurrentSound);
        Nextion_Send(messnex);
        sprintf(messnex,"page2.b%d.pco=0",CurrentSound);
        Nextion_Send(messnex);
        sprintf(messnex,"page 3");
        Nextion_Send(messnex);
        break;

        // D Draw AKWF
        case 0x44:
         if(selWaveForm1 == WAVE_AKWF)
        {
            sprintf(messnex,"page3.BK.txt=%c%03d%c",0x22,WS.OscBank,0x22);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);

            sprintf(messnex,"page3.BKPOT.val=%d",WS.OscBank);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
            Nextion_Send(messnex);

            sprintf(messnex,"page3.BKNAME.txt=%c%s%c",0x22,SampleDIR[WS.OscBank].name,0x22);
            Nextion_Send(messnex);
           

            trigloadwave=1;
            Cptloadwave=0;
                
            sprintf(messnex,"page 4");
            Nextion_Send(messnex);
        }
        else
        {
            sprintf(messnex,"page0.b2.txt=%cSELECT AKWF WAVE FIRST%c",0x22,0x22);
            Nextion_Send(messnex);
        }
        
        break;

        // W Select wave
        case 0x57:
        overcpt=0;
        overon = true;
        if(Nextion_Mess[2]==0  || Nextion_Mess[2]==2)      // Bank
        {
            WS.OscBank = Nextion_Mess[3];
            sprintf(messnex,"page3.BK.txt=%c%03d%c",0x22,WS.OscBank,0x22);
            Nextion_Send(messnex);

            sprintf(messnex,"page3.BKNAME.txt=%c%s%c",0x22,SampleDIR[WS.OscBank].name,0x22);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WAPOT.maxval=%d",SampleDIR[WS.OscBank].nbr-1);
            Nextion_Send(messnex);

            Tab_Encoder[1][6].MaxData=SampleDIR[WS.OscBank].nbr-1;  // Chg the max for the MIDI CC

            sprintf(messnex,"page3.BKPOT.val=%d",WS.OscBank);
            Nextion_Send(messnex);
            WS.AKWFWave=0;
            sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);
        }
        if(Nextion_Mess[2]==1  || Nextion_Mess[2]==3)      // Wave
        {
            WS.AKWFWave = Nextion_Mess[3];
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);
        }
        // Draw the wave on the release
        if(Nextion_Mess[2]==0 || Nextion_Mess[2]==1)
        {
            SDCard_LoadWave(WS.OscBank+1,WS.AKWFWave+1);
            Nextion_Plot();
        }
        break;

        // X Select Sound
		case 0x58:
        CurrentSound=Nextion_Mess[2];
        if(oldCurrentSound!=CurrentSound)
        {
            sprintf(messnex,"page2.b%d.bco=65535",CurrentSound);
            Nextion_Send(messnex);
            sprintf(messnex,"page2.b%d.pco=0",CurrentSound);
            Nextion_Send(messnex);

            sprintf(messnex,"page2.b%d.bco=0",oldCurrentSound);
            Nextion_Send(messnex);
            sprintf(messnex,"page2.b%d.pco=2024",oldCurrentSound);
            Nextion_Send(messnex);
            oldCurrentSound=CurrentSound;

            //SDCard_LoadSound(CurrentSound+SoundNameInc10*10);
            //Nextion_PrintValues();

        }
        break;
        
        // Y Select bank and wave inc dec
		case 0x59:
        overcpt=0;
        overon = true;
        // Bank dec
        if(Nextion_Mess[2]==0)
        {
            if(WS.OscBank>0)
                WS.OscBank--;
            sprintf(messnex,"page3.BK.txt=%c%03d%c",0x22,WS.OscBank,0x22);
            Nextion_Send(messnex);

            sprintf(messnex,"page3.BKNAME.txt=%c%s%c",0x22,SampleDIR[WS.OscBank].name,0x22);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WAPOT.maxval=%d",SampleDIR[WS.OscBank].nbr-1);
            Nextion_Send(messnex);
            Tab_Encoder[1][6].MaxData=SampleDIR[WS.OscBank].nbr-1;  // Chg the max for the MIDI CC
            sprintf(messnex,"page3.BKPOT.val=%d",WS.OscBank);
            Nextion_Send(messnex);
            WS.AKWFWave=0;
            sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);
        }
        // Bank inc
        if(Nextion_Mess[2]==1)
        {
            if(WS.OscBank<AKWFMAX_BANK-1)
                WS.OscBank++;
            sprintf(messnex,"page3.BK.txt=%c%03d%c",0x22,WS.OscBank,0x22);
            Nextion_Send(messnex);

            sprintf(messnex,"page3.BKNAME.txt=%c%s%c",0x22,SampleDIR[WS.OscBank].name,0x22);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WAPOT.maxval=%d",SampleDIR[WS.OscBank].nbr-1);
            Nextion_Send(messnex);
            Tab_Encoder[1][6].MaxData=SampleDIR[WS.OscBank].nbr-1;  // Chg the max for the MIDI CC
            sprintf(messnex,"page3.BKPOT.val=%d",WS.OscBank);
            Nextion_Send(messnex);
            WS.AKWFWave=0;
            sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);

        }
        // wave dec
        if(Nextion_Mess[2]==2)
        {
            if(WS.AKWFWave>0)
                WS.AKWFWave--;
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);
            sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
            Nextion_Send(messnex);
        }
        // wave inc
        if(Nextion_Mess[2]==3)
        {
            if(WS.AKWFWave<SampleDIR[WS.OscBank].nbr-1)
                WS.AKWFWave++;
            sprintf(messnex,"page3.WA.txt=%c%03d%c",0x22,WS.AKWFWave,0x22);
            Nextion_Send(messnex);      
            sprintf(messnex,"page3.WAPOT.val=%d",WS.AKWFWave);
            Nextion_Send(messnex);      
        }
        SDCard_LoadWave(WS.OscBank+1,WS.AKWFWave+1);
        Nextion_Plot();
        Fct_Ch_WS1(WS.WaveShapping1);
        break;

        // S Section Sound save load
		case 0x53:
        overon = false;
        overcpt=0;
        // Load save page
        if(Nextion_Mess[2]==1)
        {
            sprintf(messnex,"pagekeybdA0.b%d.bco=65535",CurrentSound);
            Nextion_Send(messnex);
            sprintf(messnex,"pagekeybdA0.b%d.pco=0",CurrentSound);
            Nextion_Send(messnex);
            // Change page
            sprintf(messnex,"pagekeybdA0");
            Nextion_Send(messnex);

        }
        // Escape
        if(Nextion_Mess[2]==4)
        {
            // Change page already done in the Nextion screen
            //sprintf(messnex,"page 1");
            //Nextion_Send(messnex);
            Nextion_PrintValues();
        }
        // Save
        if(Nextion_Mess[2]==2)
        {
            sprintf(messnex,"page2.b%d.bco=32000",CurrentSound);
            Nextion_Send(messnex);
            delay(250);
            SDCard_SaveSound(CurrentSound+SoundNameInc10*10);
            sprintf(messnex,"page2.b%d.bco=65535",CurrentSound);
            Nextion_Send(messnex);
        }
        // Load
        if(Nextion_Mess[2]==3)
        {
            sprintf(messnex,"page2.b%d.bco=32000",CurrentSound);
            Nextion_Send(messnex);
            delay(250);
            Nextion_Send(messnex);
            SDCard_LoadSound(CurrentSound+SoundNameInc10*10,0);
            sprintf(messnex,"page2.b%d.bco=65535",CurrentSound);
            Nextion_Send(messnex);
            Nextion_PrintValues();
        }
        // 10 previous
        if(Nextion_Mess[2]==5)
        {
            SoundNameInc10--;
            if(SoundNameInc10<0)
                SoundNameInc10=9;
            SDCard_Display10SndName();
        }
        // 10 next
        if(Nextion_Mess[2]==6)
        {
            SoundNameInc10++;
            if(SoundNameInc10==10)
                SoundNameInc10=0;
            SDCard_Display10SndName();
        }
        break;

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
        Serial.printf("Section %d\n",gui_Section);
        Nextion_PrintLabel();
        Nextion_PrintValues();
		break;

        // P Value of a parameter
        case 0x50:
        if(!overon)
        {
            overon = true;
        }
        overcpt=0;
        gui_Value = Nextion_Mess[3];
        cc = Tab_Encoder[gui_Section][gui_Param].MidiCC;
        ret=Synth_SetRotary(cc,gui_Value);
        //Serial.printf("VALUE Pot number %d value %03d Ret %03d\n",gui_Param,gui_Value,ret);
        Nextion_PotValue(gui_Value);
        Nextion_PrintCC(cc,ret,0);
        break;

        // Q Quit
        case 0x51:
        Nextion_PrintLabel();
        Nextion_PrintValues();
        // Change page
        sprintf(messnex,"page 1");
        Nextion_Send(messnex);
        overon = false;
        overcpt=0;
        break;

        // L Select a parameter
		case 0x4C:
        if(!overon)
        {
            overon = true;
        }
        overcpt=0;

        gui_Param = Nextion_Mess[2];
        //Serial.printf("NUMBER Pot number %d value %03d\n",gui_Param,gui_Value);
        cc = Tab_Encoder[gui_Section][gui_Param].MidiCC;
        if(cc !=0xFF)
        {
            ret=Synth_SetRotary(cc,*Tab_Encoder[gui_Section][gui_Param].Data);
            Nextion_PotValue(*Tab_Encoder[gui_Section][gui_Param].Data);
            Nextion_PrintCC(cc,ret,1);
            sprintf(messnex,"page 2");
            Nextion_Send(messnex);
            //delay(2);
        }
        break;

        // N Name sound
        case 0x4E:
        Nextion_Mess[3+9] = 0; // Filter extra char
		strcpy((char*)SndName,&Nextion_Mess[3]);
        Serial.printf("Name %s\n",SndName);
        // Set the name to the memory
        SDCard_WriteSndName(CurrentSound+SoundNameInc10*10);
        SDCard_SaveSndName();
        // Refresh Display
        SDCard_Display10SndName();

		break;

	}
}