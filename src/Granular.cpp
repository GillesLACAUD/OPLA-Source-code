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


