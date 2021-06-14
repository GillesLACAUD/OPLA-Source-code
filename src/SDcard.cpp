#include <SPI.h>
#include "FS.h"
#include "SD_MMC.h"

#include "typdedef.h"
#include "easysynth.h"
#include "ihm.h"

#include "Nextion.h"

#define __SDCARD__
#include "SDCard.h"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels)
            {
                //listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
uint8_t SDCard_Init(void)
{
    // 1 bit mode OK
    /*
    pinMode(2, INPUT_PULLUP);
    if(!SD_MMC.begin("/sdcard",true)) 
    {
        Serial.println("Card Mount 1 Bit Failed");
    }
    else
    {
        Serial.println("Card Mount 1 Bit OK");
    }
    */
    // 4 bit mode 
    
    pinMode(2, INPUT_PULLUP);
    if(!SD_MMC.begin()) 
    {
        Serial.println("Card Mount 4 Bits Failed");
    }
    else
    {
        Serial.println("Card Mount 4 Bits OK");
    }   

uint8_t cardType = SD_MMC.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        return(1);
    }

    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
    Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

    createDir(SD_MMC, "/sound");
    createDir(SD_MMC, "/AKWF");
    createDir(SD_MMC, "/System");

    writeFile(SD_MMC, "/sound/hello.txt", "Hello ");
    appendFile(SD_MMC, "/sound/hello.txt", "World!\n");
    readFile(SD_MMC, "/sound/hello.txt");

    Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
    

    return(0);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_SaveSound(uint8_t snd)
{
char path[30];
uint16_t wr;
unsigned int sz=sizeof(WorkSound);
    sprintf(path,"/sound/%d.snd",snd);
    File file = SD_MMC.open(path,"wb");
    wr=file.write((uint8_t*)&WS,sz);
    Serial.printf("File write %s oct %d\n",path,wr);
    Serial.printf("Wave write %d\n",WS.OscWave);
    Serial.printf("Noise write %d\n",WS.NoiseLevel);
    file.close();
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadSound(uint8_t snd)
{
char path[30];
uint16_t wr;
unsigned int sz=sizeof(WorkSound);

    //sprintf(messnex,"v4.pco=62222");
    //Nextion_Send(messnex);

    sprintf(path,"/sound/%d.snd",snd);
    File file = SD_MMC.open(path,"rb");
    wr=file.read((uint8_t*)&WS,sz);
    Serial.printf("File read %s oct %d\n",path,wr);
    Serial.printf("Wave read %d\n",WS.OscWave);
    Serial.printf("Noise read %d\n",WS.NoiseLevel);
    file.close();    

    
    for(uint8_t s=0;s<MAX_SECTION;s++)
    {
        for(uint8_t e=0;e<MAX_ENCODER;e++)
        {
            Tab_Encoder[s][e].ptrfunctValueChange((int)*Tab_Encoder[s][e].Data);
        }
    }

    //delay(1000);
    //sprintf(messnex,"v4.pco=38815");
    //Nextion_Send(messnex);

   /*
    Fct_Ch_OscWave(WS.OscWave);
    Fct_Ch_SubWave(WS.SubWave);
    Fct_Ch_Noise(WS.NoiseLevel);
    Fct_Ch_Detune(WS.OscDetune);
    Fct_Ch_WS1(WS.WaveShapping1);   
    Fct_Ch_OscMix(WS.OscVolume);
    Fct_Ch_SubMix(WS.SubVolume);
    Fct_Ch_SubOct(WS.SubTranspose);
    
    Fct_Ch_Cutoff(WS.Cutoff);  
    Fct_Ch_Resonance(WS.Resonance);
    Fct_Ch_KbTrack(WS.KbTrack); 
    Fct_Ch_FVelo(WS.FVelo); 
    Fct_Ch_FType(WS.FType); 
    Fct_Ch_FlAttack(WS.FEgAttack);
    Fct_Ch_FlDecay(WS.FEgDecay);
    Fct_Ch_FlRelease(WS.FEgRelease);
    Fct_Ch_FlAmount(WS.FEgAmount);

    Fct_Ch_AmAttack(WS.AEgAttack);
    Fct_Ch_AmDecay(WS.AEgDecay);
    Fct_Ch_AmSustain(WS.AEgSustain);
    Fct_Ch_AmRelease(WS.AEgRelease);
    Fct_Ch_AmVelo(WS.AmpVelo);
    Fct_Ch_PiAttack(WS.PEgAttack);
    Fct_Ch_PiDecay(WS.PEgDecay);
    Fct_Ch_PiRelease(WS.PEgRelease);
    Fct_Ch_PiAmount(WS.PEgAmount);
    Fct_Ch_Portamento(WS.Portamento);

    Fct_Ch_L1Speed(WS.LFO1Speed);
    Fct_Ch_L1Shape(WS.LFO1Shape);
    Fct_Ch_L1Dest(WS.LFO1Dest);
    Fct_Ch_L1Amount(WS.LFO1Amount);
    Fct_Ch_L2Speed(WS.LFO2Speed);
    Fct_Ch_L2Shape(WS.LFO2Shape);
    Fct_Ch_L2Dest(WS.LFO2Dest);  
    Fct_Ch_L2Amount(WS.LFO2Amount);

    Fct_Ch_DlLen(WS.DelayLen);
    Fct_Ch_DlAmount(WS.DelayAmount);
    Fct_Ch_DlFeed(WS.DelayFeedback);
    
    Fct_Ch_SoundMode(WS.SoundMode);
    Fct_Ch_PBRange(WS.PBRange);
    Fct_Ch_MDDest(WS.MWDest);
    Fct_Ch_MDAmt(WS.MWAmt);
    Fct_Ch_ATDest(WS.ATDest);
    Fct_Ch_ATAmt(WS.ATAmt);
    */


    //Fct_Ch_MidiRx(WS.R);
    //Fct_Ch_Spread(WS.S);    

}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadSndName(void)
{
char path[30];
char nom[30];
uint8_t ret;

    sprintf(path,"/sound/Names.txt");
    File file = SD_MMC.open(path,"rb");
    ret=file.readBytesUntil(0x0D,nom,15);
    nom[ret+1]=0x00;
    Serial.printf("File Name %s\n",nom);
    file.readBytesUntil(0x0D,nom,15);
    nom[ret+1]=0x00;
    Serial.printf("File Name %s\n",nom);

    file.readBytesUntil(0x0D,nom,15);
    nom[ret+1]=0x00;
    Serial.printf("File Name %s\n",nom);

    file.readBytesUntil(0x0D,nom,15);
    nom[ret+1]=0x00;
    Serial.printf("File Name %s\n",nom);

    file.close();    
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_SaveSndName(uint8_t snd)
{
}


