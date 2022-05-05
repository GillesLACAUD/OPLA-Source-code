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

    ptGraWorkingBuffer = (int16_t *)ps_malloc(GRA_BUFFER_SIZE*2);
    if (ptGraWorkingBuffer == NULL)
    {
        Serial.printf("No more heap memory for Gra Buffer working!\n");
        while(1);
    }

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
        return(wr);
    }
    else
    {
        Serial.printf("GRANULAR ERROR File not present %s\n",name);
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
int16_t*    pt;    

    uint32_t s=0;    
    // Init grains
    for(uint8_t g=0;g<Gra_Density;g++)
    {
        str_tabgrain[g].u32_beginpos = Gra_Begin+g*Gra_Space;
        str_tabgrain[g].u32_size = 441;  // 100ms
        str_tabgrain[g].u8_ident = g; 
    }    
    // Add Grains - fill the playing buffer
    int16_t* ptdst;
    int16_t* ptsrc;
    ptdst=ptGraPlayingBuffer;
    ptsrc=ptGraMemory;
    ptkeep=ptGraPlayingBuffer;

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

    Gra_Size            = GRA_MAX_SIZE;        // MAX GRA_MAX_SIZE
    Gra_OverlapPc        = 100;
    Gra_SizeAttack      = 1*Gra_Size/10;
    Gra_SizeSustain     = 9*Gra_Size/10;
    Gra_OverlapSpl      = (Gra_Size*Gra_OverlapPc)/100;
    Gra_BufferSize      = Gra_Size+(Gra_Density-1)*Gra_OverlapSpl;

    // Init the playing buffer
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