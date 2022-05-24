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
    //voice->i8_reverse=1;
    voice->u32_cumulspeed +=voice->u32_speed;
    u32_whole= voice->u32_cumulspeed/1000;
    u32_rest = voice->u32_cumulspeed - u32_whole*1000;
    voice->u32_cumulspeed = u32_rest;
    voice->u32_cumulWhole +=2*u32_whole;
    if(voice->u32_cumulWhole>=(Gra_BufferSize))
    {
        voice->u32_cumulWhole =0;
        voice->u32_cumulspeed = 0;
        /*
        if(voice->i8_reverse==1)
            voice->i8_reverse =-1;
        else
            voice->i8_reverse =1;
        */
    }
    if(voice->i8_reverse==1)
    {
        u32_rest=voice->u32_cumulWhole;
        //Serial.printf(" 1 Ret %06d\r\n",u32_rest);
    }
    else
    {
        u32_rest=Gra_BufferSize-voice->u32_cumulWhole;
        //Serial.printf("-1 Ret %06d\r\n",u32_rest);
    }
    return(u32_rest);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
double Granular_MidiNoteRatio(int midiNote)
{
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

    ptGraMemory = (int16_t *)ps_malloc(GRA_MEMORY_SIZE*2);
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

    ptGraPlayingBuffer = (int16_t *)ps_malloc(GRA_BUFFER_SIZE*2);
    if (ptGraPlayingBuffer == NULL)
    {
        Serial.printf("No more heap memory for Gra Buffer playing!\n");
        while(1);
    }

    Serial.printf("---- AFTER GRANULAR MALLOC ----\n");
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

    //Granular_Reset();
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
GRANULAR_EXTRN int16_t*    pt;

    pt=ptGraMemory;
    sprintf(path,"/wave/%s",name);
    File file = SD_MMC.open(path,"rb");
    if(file)
    {
        wr=file.read((uint8_t*)pt,44); 
        wr = 0;
        for(int i=0;i<GRA_NB_SECONDS*2;i++)
        {
            wr+=file.read((uint8_t*)pt,(GRA_FS_SAMPLE*2));   // read the data in bytes 1 second
            pt += GRA_FS_SAMPLE;                            // inc the pointer in int
        }
        Serial.printf("Load wav file %s read %d bytes\n",name,wr);

        ptdst=ptGraPlayingBuffer;
        ptsrc=ptGraMemory;
        

        Gra_Begin=0x10000;
        //Gra_Space=0;                  // Play the same grain
        //Gra_Space=GRA_MAX_SIZE;       // All the grains are contigue
        Gra_Space=GRA_MAX_SIZE;
        Gra_Density=1;
        Gra_Size            = GRA_MAX_SIZE;        // MAX GRA_MAX_SIZE
        Gra_OverlapPc        = 100;
        Gra_SizeAttack      = (5*Gra_Size)/100;
        Gra_SizeSustain     = (98*Gra_Size)/100;
        //Gra_SizeAttack      = 0;
        //Gra_SizeSustain     = Gra_Size;;
        Gra_OverlapSpl      = (Gra_Size*Gra_OverlapPc)/100;
        Gra_BufferSize      = Gra_Size+(Gra_Density-1)*Gra_OverlapSpl;
        Gra_NewBufferSize   = Gra_BufferSize;
        memset(ptGraPlayingBuffer,0,Gra_BufferSize*2);

        Gra_AttackCoeff = GRA_EG_FULLSCALE/(Gra_SizeAttack+1);
        Gra_ReleaseCoeff = GRA_EG_FULLSCALE/(Gra_Size-1-Gra_SizeSustain);

        ptWave=ptGraMemory;
        ptPlay=ptGraPlayingBuffer;
        ptGraGrain=ptGraPlayingBuffer;
        ptGrain=ptGraGrain;
        CptGrain=0;

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
void Granular_Process(void)
{
int16_t i16_spl;
float val1;
float coeff;    
static uint8_t cptstep=0;
static uint8_t step;
static uint8_t firstg;
static uint8_t lastg;
uint8_t stepnbgrain=1;

    // Can only compute x Grain at one time
    // 01-06 Fist step
    // 07-12 Second Step
    // 12-18 Third Step
    
    step=Gra_Density/stepnbgrain;
    firstg=stepnbgrain*cptstep;
    lastg=stepnbgrain*(cptstep+1);
    firstg=stepnbgrain*cptstep;
    lastg=stepnbgrain*(cptstep+1);
    if(lastg>Gra_Density)
        lastg=Gra_Density;
        
    //firstg=0;
    //lastg=Gra_Density;

    /* Refresh playing buffer*/
    // Gra_Ask_RefreshPlaying always true for now
    // CptGrain is increase for each time until its value is > Gra_Size
    if(Gra_Ask_RefreshPlaying)
    {
        //-------------------------------------------------
        // At the end of the grain paster we have build
        // stepnbgrain and we continue with the next one
        //-------------------------------------------------
        if(CptGrain>=Gra_Size)
        {
            CptGrain=0;
            //Gra_Ask_RefreshPlaying=0;
            ptGrain=ptGraGrain;
            cptstep++;
            if(cptstep>step)
            {
                Gra_BufferSize=Gra_NewBufferSize;
                cptstep=0;
            }
        }
        else
        {
            //-------------------------------------------------
            // Compute AR Enveloppe
            //-------------------------------------------------
            if(CptGrain<Gra_SizeAttack)
            {
                coeff=(float)CptGrain/(float)(Gra_SizeAttack+1);
            }
            else
            {
                if(CptGrain<Gra_SizeSustain)
                {
                    coeff=1;
                }
                else
                {
                    if(CptGrain<Gra_Size)
                    {
                        coeff=(float)(CptGrain-Gra_SizeSustain)/(float)(Gra_Size-Gra_SizeSustain);
                        coeff=1-coeff;
                    }
                }
            }
            uint8_t div=1;
            //-------------------------------------------------
            // Apply the EG and add only 2 grains
            //-------------------------------------------------
            for(uint8_t g1=firstg;g1<lastg;g1++)
            {
                // Left
                pt=ptWave+(str_tabgrain[g1].u32_beginpos+CptGrain);
                ptdst = ptGrain+g1*Gra_OverlapSpl;
                val1 = (float)(*pt)/2;
                val1 *=coeff;
                *ptdst=(int16_t)val1;
                // Overlap
                if(0)
                {
                    if(g1<(Gra_Density-1)/* && CptGrain>=Gra_OverlapSpl*/)
                    {
                        pt=ptWave+(str_tabgrain[g1+1].u32_beginpos+CptGrain);
                        val1 = (float)(*pt)/2;
                        val1 *=coeff;
                        *ptdst=*ptdst/2 + (int16_t)val1/2;
                    }
                }
                
                // Right
                pt=ptWave+1+(str_tabgrain[g1].u32_beginpos+CptGrain);
                ptdst++;
                val1 = (float)(*pt)/2;
                val1 *=coeff;
                *ptdst=(int16_t)val1;
                // Overlap
                if(0)
                {
                    if(g1<(Gra_Density-1)/*&& CptGrain>=Gra_OverlapSpl*/)
                    {
                        pt=ptWave+(str_tabgrain[g1+1].u32_beginpos+CptGrain);
                        val1 = (float)(*pt)/2;
                        val1 *=coeff;
                        *ptdst=*ptdst/2 + (int16_t)val1/2;
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
    for(uint8_t g=0;g<Gra_Density;g++)
    {
        str_tabgrain[g].u32_beginpos = Gra_Begin+g*(Gra_Space/*+Gra_Size*/);
        if(str_tabgrain[g].u32_beginpos>GRA_BUFFER_SIZE)
        {
            str_tabgrain[g].u32_beginpos -=GRA_BUFFER_SIZE;
        }

        str_tabgrain[g].u32_size = 441;  // 100ms - do notchange anything for now 24.05.2022
        str_tabgrain[g].u8_ident = g; 
    }    
    Gra_OverlapSpl      = (Gra_Size*Gra_OverlapPc)/100;
    Gra_NewBufferSize=Gra_Size+(Gra_Density-1)*Gra_OverlapSpl;

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