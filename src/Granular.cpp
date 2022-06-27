/*
 * This is a implementation of Granular Oscillator
 *
 * Author: Gilles Lacaud
 */
#include <SPI.h>
#include "FS.h"
#include "SD_MMC.h"

#include "typdedef.h"
#include "easysynth.h"
#include "ihm.h"
#include "simple_delay.h"

#include "Nextion.h"
#include "SDCard.h"
#define __GRANULAR__
#include "Granular.h"


/**************************************************/
/* Transpose Pitch shifting                       */
/*                                                */
/*                                                */
/*                                                */
/**************************************************/
uint32_t Granular_TransposeStereo(notePlayerT *voice)
{
uint32_t u32_whole=0;
uint32_t u32_rest=0;

    
    // En entier 1000 = 1.0 -> on pert des decimales
    int32_t add;
    add =voice->u32_speed;
    add += (int32_t)(((float)voice->u32_speed)*PitchMod);

    voice->u32_cumulspeed +=add;
    u32_whole= voice->u32_cumulspeed/1000;
    u32_rest = voice->u32_cumulspeed - u32_whole*1000;
    voice->u32_cumulspeed = u32_rest;

    switch(u8_GraReverse)
    {
        case 0:
        voice->i32_cumulWhole +=2*u32_whole;        
        if(voice->i32_cumulWhole>=(Gra_BufferSize-4))
        {
            voice->u32_cumulspeed = 0;
            voice->i32_cumulWhole =0;
            return(u32_rest);
        }        
        break;
        case 1:
        voice->i32_cumulWhole -=2*u32_whole;        
        if(voice->i32_cumulWhole<=4)
        {
            voice->u32_cumulspeed = 0;
            voice->i32_cumulWhole =Gra_BufferSize;
            return(u32_rest);
        }        
        break;
        case 2:
        if(!voice->i8_reverse)
        {
            voice->i32_cumulWhole +=2*u32_whole;        
            if(voice->i32_cumulWhole>=(Gra_BufferSize-4))
            {
                voice->u32_cumulspeed = 0;
                voice->i8_reverse=1;
                return(u32_rest);
            }
        }
        else
        {
            voice->i32_cumulWhole -=2*u32_whole;        
            if(voice->i32_cumulWhole<=4)
            {
                voice->u32_cumulspeed = 0;
                voice->i32_cumulWhole =0;
                voice->i8_reverse =0;
                return(u32_rest);
            }
        }        
        break;
    }
    u32_rest=voice->i32_cumulWhole;
    return(u32_rest);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
double Granular_MidiNoteRatio(int midiNote)
{

    midiNote +=GraTranspose;
    int distanceFromCenter = midiNote - 60; // 60 is the central midi note

    if ( distanceFromCenter < 0 )
    {
        int diffAmount = -distanceFromCenter;
        int octaves = diffAmount / 12;
        int intervals = diffAmount % 12;

        return pow( 0.5, octaves ) / chromaticRatios[intervals];
    }
    else
    {
        int octaves = distanceFromCenter / 12;
        int intervals = distanceFromCenter % 12;
        return pow( 2, octaves ) * chromaticRatios[intervals];
    }
}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Granular_Init(void)
{
    Serial.printf("---- BEFORE GRANULAR MALLOC ----\n");
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

    ptGraMemory = (int16_t *)ps_malloc(GRA_MEMORY_SIZE);                // 10s Stereo 44100
    if (ptGraMemory == NULL)
    {
        Serial.printf("No more heap memory for Gra Memory!\n");
        while(1);
    }
    /*
    ptGraWorkingBuffer = (int16_t *)ps_malloc(GRA_BUFFER_SIZE*2);
    if (ptGraWorkingBuffer == NULL)
    {
        Serial.printf("No more heap memory for Gra Buffer working!\n");
        while(1);
    }
    */

    ptGraPlayingBuffer = (int16_t *)ps_malloc(GRA_BUFFER_SIZE);   // 10s Stereo 44100
    if (ptGraPlayingBuffer == NULL)
    {
        Serial.printf("No more heap memory for Gra Buffer playing!\n");
        while(1);
    }

    ptGraAddMemory = (int16_t *)ps_malloc(GRA_FS_SAMPLE);   // 0.5s Stereo 44100
    if (ptGraAddMemory == NULL)
    {
        Serial.printf("No more heap memory for Gra Buffer Add!\n");
        while(1);
    }

    Serial.printf("---- AFTER GRANULAR MALLOC ----\n");
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

    //Granular_Reset();

    ptdst=ptGraPlayingBuffer;
    ptsrc=ptGraMemory;
    oldCurrentGraWave=CurrentGraWave;

    Gra_BufferSize      = Gra_Size/*+(Gra_Density-1)*Gra_OverlapSpl/2*/;
    Gra_NewBufferSize   = Gra_BufferSize;
    memset(ptGraPlayingBuffer,0,Gra_BufferSize*2);

    Gra_AttackCoeff = GRA_EG_FULLSCALE/(Gra_SizeAttack+1);
    Gra_ReleaseCoeff = GRA_EG_FULLSCALE/(Gra_Size-1-Gra_SizeSustain);

    ptWave=ptGraMemory;
    ptPlay=ptGraPlayingBuffer;
    ptGraGrain=ptGraPlayingBuffer;
    ptGrain=ptGraGrain;
    CptGrain=0;

    Serial.printf("Granular Init Done\n");

}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Granular_Reset(void)
{
    for (uint32_t i = 0; i < GRA_MEMORY_SIZE; i++)
    {
        ptGraMemory[i] = 0;
    }
    Serial.printf("Gra Memory Reset Done\n");
}

/***************************************************/
/* Load the first second of a wave file in the mem */
/* return the number of bytes read                 */
/*                                                 */
/***************************************************/
uint32_t Granular_LoadWave(char* name)
{
char path[50];
uint32_t wr;
int16_t*    pt;
char mess[50];

    //strcpy(Tab_Section_Name[0],name);
    //sprintf(messnex,"page0.ts0.txt=%c%s%c",0x22,Tab_Section_Name[0],0x22);
    //Nextion_Send(messnex);

    pt=ptGraMemory;
    sprintf(path,"/wave/%s",name);
    File file = SD_MMC.open(path,"rb");
    if(file)
    {

        //sprintf(messnex,"page2.setup_name.font=2");
        //Nextion_Send(messnex);

        sprintf(mess,"Load...Wait...");
        sprintf(messnex,"page2.Setup_Name.txt=%c%s%c",0x22,mess,0x22);
        Serial1.print(messnex);
        Serial1.write(0xff);
        Serial1.write(0xff);
        Serial1.write(0xff);

        wr=file.read((uint8_t*)pt,44); 
        Serial.printf("Load wav file %s read %d bytes\n",name,wr);
        wr = 0;
        for(int i=0;i<GRA_NB_SECONDS;i++)
        {
            wr+=file.read((uint8_t*)pt,(GRA_FS_SAMPLE*GRA_NB_BYTES*GRA_NB_CHANNELS));   // read the data in bytes 1 second 2 Channels
            pt += GRA_FS_SAMPLE*GRA_NB_CHANNELS;                                        // inc the pointer in int
            /*
            strcat(mess,".");
            sprintf(messnex,"page2.Setup_Name.txt=%c%s%c",0x22,mess,0x22);
            Serial1.print(messnex);
            Serial1.write(0xff);
            Serial1.write(0xff);
            Serial1.write(0xff);
            */
        }
        Serial.printf("End Load wave file\n");
        ptdst=ptGraPlayingBuffer;
        ptsrc=ptGraMemory;
        oldCurrentGraWave=CurrentGraWave;

        // FAKE INIT
        //Gra_Begin=0x00;
        //Gra_Space=0;                  // Play the same grain
        //Gra_Space=GRA_MAX_SIZE;       // All the grains are contigue
        //Gra_Space=0;
        //Gra_Density=1;
        //Gra_Size            = GRA_MAX_SIZE;        // MAX GRA_MAX_SIZE
        //Gra_OverlapPc        = 100;
        //Gra_SizeAttack      = (5*Gra_Size)/100;
        //Gra_SizeSustain     = (98*Gra_Size)/100;
        //Gra_SizeAttack      = 0;
        //Gra_SizeSustain     = Gra_Size;;
        //Gra_OverlapSpl      = (Gra_Size*Gra_OverlapPc)/100;

        Gra_BufferSize      = Gra_Size/*+(Gra_Density-1)*Gra_OverlapSpl/2*/;
        Gra_NewBufferSize   = Gra_BufferSize;
        memset(ptGraPlayingBuffer,0,Gra_BufferSize*2);

        Gra_AttackCoeff = GRA_EG_FULLSCALE/(Gra_SizeAttack+1);
        Gra_ReleaseCoeff = GRA_EG_FULLSCALE/(Gra_Size-1-Gra_SizeSustain);

        // END FAKE INIT

        ptWave=ptGraMemory;
        ptPlay=ptGraPlayingBuffer;
        ptGraGrain=ptGraPlayingBuffer;
        ptGrain=ptGraGrain;
        CptGrain=0;

        // Restore the name
        sprintf(messnex,"page2.setup_name.font=1");
        Nextion_Send(messnex);
        sprintf(messnex,"page2.Setup_Name.txt=%c%s%c",0x22,SndName,0x22);
        Nextion_Send(messnex);

        return(wr);
    }
    else
    {
        Serial.printf("GRANULAR ERROR File not present %s\n",name);
        while(1);
        return(0);
    }

}

/***************************************************/
/* Add another wave file to the memory             */
/*                                                 */
/*                                                 */
/***************************************************/
uint32_t Granular_AddWave(char* name)
{
char path[50];
uint32_t wr;
int16_t*    pt;
int16_t*    ptadd;
int16_t*    savpt;
int16_t*    savptadd;

char mess[50];
uint32_t nbbytesread;

    pt=ptGraMemory;
    ptadd = ptGraAddMemory;
    sprintf(path,"/wave/%s",name);
    File file = SD_MMC.open(path,"rb");

    savpt=pt;
    savptadd=ptadd;

    if(file)
    {
        sprintf(mess,"Add...Wait...");
        sprintf(messnex,"page2.Setup_Name.txt=%c%s%c",0x22,mess,0x22);
        Serial1.print(messnex);
        Serial1.write(0xff);
        Serial1.write(0xff);
        Serial1.write(0xff);

        wr=file.read((uint8_t*)ptadd,44); 
        Serial.printf("Load wav file %s read %d bytes\n",name,wr);
        wr = 0;
        for(int i=0;i<GRA_NB_SECONDS*2;i++)
        {
            wr+=file.read((uint8_t*)ptadd,(GRA_FS_SAMPLE));   // read the data in bytes 1 second 2 Channels
            // Add the two buffer
            for(uint32_t s=0;s<GRA_FS_SAMPLE/2;s++)
            {
                *pt = *pt/2 + *ptadd/2;   
                pt++;
                ptadd++;
            }
            ptadd = ptGraAddMemory;
        }
        Serial.printf("End Load wave file read %d Mem %d Add %d\n",wr,pt-savpt,ptadd-savptadd);


        // Restore the name
        sprintf(messnex,"page2.setup_name.font=1");
        Nextion_Send(messnex);
        sprintf(messnex,"page2.Setup_Name.txt=%c%s%c",0x22,SndName,0x22);
        Nextion_Send(messnex);
        return(wr);
    }
    else
    {
        Serial.printf("GRANULAR ERROR File not present %s\n",name);
        while(1);
        return(0);
    }

}



/***************************************************/
/* Only for debug                                  */
/*                                                 */
/*                                                 */
/***************************************************/
void Granular_Dump(void)
{
    Serial.printf("Gra_MaxPlay           %06X\r\n",Gra_Maxplay);
    Serial.printf("Gra_Size              %06d\r\n",Gra_Size);
    Serial.printf("Gra_Density           %06d\r\n",Gra_Density);
    Serial.printf("Gra_SpacePourcent     %06d\r\n",Gra_OverlapPc);
    Serial.printf("Gra_SpaceSample       %06d\r\n",Gra_OverlapSpl);
    Serial.printf("Gra_SizeAttack        %06d\r\n",Gra_SizeAttack);
    Serial.printf("Gra_SizeSustain       %06d\r\n",Gra_SizeSustain);
    Serial.printf("Gra_Space             %06d\r\n",Gra_Space);
    Serial.printf("Gra_BufferSize        %06d\r\n",Gra_BufferSize);
    Serial.printf("MAX Gra_BufferSize    %06d\r\n",GRA_MAX_SIZE*GRAIN_MAX);
}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Granular_Process(uint8_t set)
{
int32_t i32_spl;
uint16_t ui16_coeff;    
uint8_t ui8_div;    
uint16_t ui16_multi=1000;    
static uint8_t cptstep=0;
static uint8_t step;
static uint8_t firstg;
static uint8_t lastg;
uint8_t stepnbgrain=1;
static uint16_t cptloop=0;
static uint16_t cptkey=0;

    // Can only compute x Grain at one time
    // 01-06 Fist step
    // 07-12 Second Step
    // 12-18 Third Step

    /* bof pas bon
    if(set==1)
    {
        cptstep=step+1;
        CptGrain=0;
        ptGrain=ptGraGrain;
        Gra_BufferSize=Gra_NewBufferSize;
        Granular_UpdateVal();

        cptkey++;
        printf("%03d KEY\n",cptkey);
    }
    */
    
    step=Gra_Density/stepnbgrain;
    firstg=stepnbgrain*cptstep;
    lastg=stepnbgrain*(cptstep+1);
    if(lastg>Gra_Density)
        lastg=Gra_Density;

    //firstg=0;
    //lastg=1;

    /* Refresh playing buffer*/
    // Gra_Ask_RefreshPlaying always true for now
    // CptGrain is increase for each time until its value is > Gra_Size
    if(1)
    //if(Gra_Ask_RefreshPlaying)
    {
        //-------------------------------------------------
        // At the end of the grain paster we have build
        // stepnbgrain and we continue with the next one
        //-------------------------------------------------
        //Serial.printf("Cpt %d size %d\r\n",CptGrain,Gra_Size);
        if(CptGrain>=Gra_Size)
        {
            CptGrain=0;
            //Gra_Ask_RefreshPlaying=0;
            ptGrain=ptGraGrain;
            cptstep++;
            if(cptstep>=step)
            {
                Gra_BufferSize=Gra_NewBufferSize;
                cptstep=0;
                //Serial.printf("-END-%03d-%d-----\r\n",cptloop,Gra_Size);
            }
            Granular_UpdateVal();
            cptloop++;
        }
        else
        {
            //-------------------------------------------------
            // Compute AR Enveloppe
            //-------------------------------------------------
            if(CptGrain<Gra_SizeAttack)
            {
                ui16_coeff=CptGrain*ui16_multi/(Gra_SizeAttack+1);
            }
            else
            {
                if(CptGrain<Gra_SizeSustain)
                {
                    ui16_coeff=ui16_multi;
                }
                else
                {
                    if(CptGrain<Gra_Size)
                    {
                        ui16_coeff=(CptGrain-Gra_SizeSustain)*ui16_multi/(Gra_Size-Gra_SizeSustain);
                        ui16_coeff=ui16_multi-ui16_coeff;
                    }
                }
            }
            //-------------------------------------------------
            // Apply the EG and add only 2 grains
            //-------------------------------------------------
            for(uint8_t g1=firstg;g1<lastg;g1++)
            {
                //ui8_div = g1*g1+2;
                //ui8_div = g1+2;
                ui8_div = 2;
                // Left
                pt=ptWave+(str_tabgrain[g1].u32_beginpos+CptGrain);
                ptdst = ptGrain+g1*Gra_OverlapSpl;
                i32_spl = (int32_t)(*pt)/ui8_div;
                i32_spl *=ui16_coeff;
                i32_spl /=ui16_multi;
                *ptdst=(int16_t)i32_spl;
                // Overlap
                if(0)
                {
                    if(g1<(Gra_Density-1)/* && CptGrain>=Gra_OverlapSpl*/)
                    {
                        pt=ptWave+(str_tabgrain[g1+1].u32_beginpos+CptGrain);
                        i32_spl = (int32_t)(*pt)/ui8_div;
                        i32_spl *=ui16_coeff;
                        i32_spl /=ui16_multi;
                        *ptdst=*ptdst/ui8_div + (int16_t)i32_spl/ui8_div;
                    }
                }
                
                // Right
                pt=ptWave+1+(str_tabgrain[g1].u32_beginpos+CptGrain);
                ptdst++;
                i32_spl = (int32_t)(*pt)/ui8_div;
                i32_spl *=ui16_coeff;
                i32_spl /=ui16_multi;
                *ptdst=(int16_t)i32_spl;
                // Overlap
                if(0)
                {
                    if(g1<(Gra_Density-1)/*&& CptGrain>=Gra_OverlapSpl*/)
                    {
                        pt=ptWave+(str_tabgrain[g1+1].u32_beginpos+CptGrain);
                        i32_spl = (int32_t)(*pt)/ui8_div;
                        i32_spl *=ui16_coeff;
                        i32_spl /=ui16_multi;
                        *ptdst=*ptdst/ui8_div + (int16_t)i32_spl/ui8_div;
                    }
                }
            }
            ptGrain++;
            CptGrain++;
        }
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Granular_UpdateVal(void)
{
static uint8_t  first=5;


    uint32_t s=0;    
    // Init grains positions
    int32_t ModBegin = Gra_Begin+Gra_Fine+Gra_ModBegin;
    int32_t ModSize = Gra_Size+Gra_ModSize;
    if(ModBegin<0)
        ModBegin=0;
    if(ModSize<2000)
        ModSize=2000;
    if(ModSize>GRA_MAX_SIZE)
        ModSize=GRA_MAX_SIZE;

    //Serial.printf("ModBegin %d\n",ModBegin);

    for(uint8_t g=0;g<Gra_Density;g++)
    {
        str_tabgrain[g].u32_beginpos = ModBegin+g*(Gra_Space/*+Gra_Size*/);
        if(str_tabgrain[g].u32_beginpos>GRA_BUFFER_SIZE)
        {
            str_tabgrain[g].u32_beginpos -=GRA_BUFFER_SIZE;
        }

        str_tabgrain[g].u32_size = 441;  // 100ms - do notchange anything for now 24.05.2022
        str_tabgrain[g].u8_ident = g; 
    }    
    Gra_OverlapSpl      = (ModSize*Gra_OverlapPc)/100;
    Gra_NewBufferSize=ModSize+(Gra_Density-1)*Gra_OverlapSpl;

    // Add Grains - fill the playing buffer

    if(!first)
    {    
    // Copy Memory to play buffer
    /*
    for(uint32_t s=0;s<GRA_BUFFER_SIZE;s++)
    {
        *ptdst=*ptsrc;
        ptdst++;
        ptsrc++;
    }
    */

    // TODO
    // GRAIN SPACE EN %  de la longueur totale
    // LECTUEE EN REVERSE
    // REVOIR LE COMPTEUR CPTPLAY






    // Init the playing buffer
    
        first=1;
 
        memset(ptGraPlayingBuffer,0,Gra_BufferSize*2);

        //Granular_Dump();


        for(;s<Gra_SizeAttack;s++)
        {
            for(uint8_t g=0;g<Gra_Density;g++)
            {
                pt = ptsrc+str_tabgrain[g].u32_beginpos+s;
                if(pt<ptsrc+GRA_MEMORY_SIZE)
                    *(ptdst+g*Gra_OverlapSpl)+=(*pt/Gra_Density)>>(16-((s)/(Gra_SizeAttack/16)));
            }
            ptdst++;
        }
        for(;s<Gra_SizeSustain;s++)
        {
            for(uint8_t g=0;g<Gra_Density;g++)
            {
                pt = ptsrc+str_tabgrain[g].u32_beginpos+s;
                if(pt<ptsrc+GRA_MEMORY_SIZE)
                    *(ptdst+g*Gra_OverlapSpl)+=(*pt/Gra_Density);
            }
            ptdst++;
        }
        for(;s<=Gra_Size;s++)
        {
            for(uint8_t g=0;g<Gra_Density;g++)
            {
                pt = ptsrc+str_tabgrain[g].u32_beginpos+s;
                if(pt<ptsrc+GRA_MEMORY_SIZE)
                    *(ptdst+g*Gra_OverlapSpl)+=(*pt/Gra_Density)>>(((s-Gra_SizeSustain)/(Gra_SizeSustain/16)));
            }
            ptdst++;
        }    
        //Serial.printf("TOTAL BUFFER WRITE %06d\r\n",(ptdst+(Gra_Density-1)*Gra_Space)-ptGraPlayingBuffer);
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void Granular_Trash(void)
{
int32_t     val;
int16_t*    pt;

    if(0)
    {
        // TEST ALGO EG ASR
        // Fill with 100
        Gra_Size = 100;
        //Gra_SizeAttack  = Gra_Size/3;
        //Gra_SizeSustain = 2*Gra_Size/3;
        Gra_SizeAttack  = 0;
        Gra_SizeSustain = 0;

        for(uint32_t s=0;s<100;s++)
        {
            *ptsrc = 10000;
            *ptdst = *ptsrc;
            ptsrc++;
            ptdst++;

        }
        ptdst=ptGraPlayingBuffer;
        ptsrc=ptGraMemory;
        uint32_t s=0;
        uint8_t tab_shift[100];
        Gra_Density=1;
        for(;s<Gra_SizeAttack;s++)
        {
            val=0;
            for(uint8_t g=0;g<Gra_Density;g++)
            {
                pt = ptsrc+str_tabgrain[g].u32_beginpos+s;
                if(pt<ptsrc+GRA_MEMORY_SIZE)
                    val+=(*pt)>>16-(s/(Gra_SizeAttack/16));
            }
            val /=Gra_Density;
            *ptdst=(uint16_t)val;
            ptdst++;
        }
        for(;s<Gra_SizeSustain;s++)
        {
            val=0;
            for(uint8_t g=0;g<Gra_Density;g++)
            {
                pt = ptsrc+str_tabgrain[g].u32_beginpos+s;
                if(pt<ptsrc+GRA_MEMORY_SIZE)
                    val+=(*pt);
            }
            val /=Gra_Density;
            *ptdst=(uint16_t)val;
            ptdst++;
        }
        for(;s<=Gra_Size;s++)
        {
            val=0;
            for(uint8_t g=0;g<Gra_Density;g++)
            {
                pt = ptsrc+str_tabgrain[g].u32_beginpos+s;
                if(pt<ptsrc+GRA_MEMORY_SIZE)
                    val+=(*pt)>>(s-Gra_SizeSustain)/((Gra_Size-Gra_SizeSustain)/16);
            }
            val /=Gra_Density;
            *ptdst=(uint16_t)val;
            ptdst++;
        }    
        ptdst=ptGraPlayingBuffer;
        for(uint32_t s=0;s<100;s++)
        {
            Serial.printf("%02d %04d \r\n",s,*ptdst);
            ptdst++;
        }
        while(1);
    }
}