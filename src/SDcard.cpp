#include <SPI.h>
#include "FS.h"
#include "SD_MMC.h"

#include "typdedef.h"
#include "easysynth.h"
#include "ihm.h"
#include "simple_delay.h"

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
    
    tabname = (uint8_t*)ps_malloc(SDCARD_TAB_NAME);
    if (tabname == NULL)
    {
        Serial.printf("No more heap memory!\n");
        while(1);
    }
    Serial.printf("---- AFTER PS_MALLOC NAME SOUNDS----\n");
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());    

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
    file.close();
    Serial.printf("Save sound %d\n",snd);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadSound(uint8_t snd,uint8_t source)
{
char path[30];
uint16_t wr;
unsigned int sz=sizeof(WorkSound);
uint8_t range;
float factor; 
int val;   

    Delay_Reset();

    if(source==1)
    {
        sprintf(messnex,"page2.b%d.bco=0",oldCurrentSound);
        Nextion_Send(messnex);
        sprintf(messnex,"page2.b%d.pco=2024",oldCurrentSound);
        Nextion_Send(messnex);
    }

    LastSoundNumber = snd;
    SoundNameInc10 =   LastSoundNumber/10;
    CurrentSound = snd-SoundNameInc10*10;
    oldCurrentSound = CurrentSound; 

    // Set the page
    SDCard_Display10SndName();

    sprintf(path,"/sound/%d.snd",snd);
    File file = SD_MMC.open(path,"rb");
    wr=file.read((uint8_t*)&WS,sz);
    file.close();    
    Serial.printf("Load sound %d\n",snd);

    IsLoadSound = 1;
    for(uint8_t s=0;s<MAX_SECTION;s++)
    {
        for(uint8_t e=0;e<MAX_ENCODER;e++)
        {
            val = *Tab_Encoder[s][e].Data;
            if(Tab_Encoder[s][e].Type==TYPE_LIST)
            {
            }
            else
            {
                range = Tab_Encoder[s][e].MaxData-Tab_Encoder[s][e].MinData;
                factor = (float)range/127;
                val = Tab_Encoder[s][e].MinData + (int)((float)val*factor);
            }
            Tab_Encoder[s][e].ptrfunctValueChange(val);
            //Tab_Encoder[s][e].ptrfunctValueChange((int)*Tab_Encoder[s][e].Data);
            
        }
    }
    IsLoadSound = 0;
    // To overwrite the max poly if delay =0
    Fct_Ch_DlAmount(WS.DelayAmount);

    // Update sound name and number for the different pages
    SDCard_ReadSndName(snd);
    sprintf(messnex,"page0.Setup_Name.txt=%c%s%c",0x22,SndName,0x22);
    Nextion_Send(messnex);
    sprintf(messnex,"page0.Setup_Number.txt=%c%02d%c",0x22,snd,0x22);
    Nextion_Send(messnex);

    sprintf(messnex,"page2.Setup_Name.txt=%c%s%c",0x22,SndName,0x22);
    Nextion_Send(messnex);
    sprintf(messnex,"page2.Setup_Number.txt=%c%02d%c",0x22,snd,0x22);
    Nextion_Send(messnex);

    if(source==1)
    {
        sprintf(messnex,"page2.b%d.bco=65535",CurrentSound);
        Nextion_Send(messnex);
        sprintf(messnex,"page2.b%d.pco=0",CurrentSound);
        Nextion_Send(messnex);
    }

    // Write the sound number in a file
    sprintf(path,"/sound/last.lst");
    file = SD_MMC.open(path,"wb");
    wr=file.write((uint8_t*)&snd,1);
    file.close();    
   
       
}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadLastSound()
{
    char path[30];
    uint16_t wr;
    uint8_t snd;

    // Read the sound number in a file
    sprintf(path,"/sound/last.lst");
    File file = SD_MMC.open(path,"rb");
    wr=file.read((uint8_t*)&snd,1);
    file.close();   

    SDCard_LoadSound(snd,1);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_SaveMidiRx()
{
    char path[30];
    uint16_t wr;
    // Write the Midirx in a file
    sprintf(path,"/System/midirx.cfg");
    File file = SD_MMC.open(path,"wb+");
    wr=file.write((uint8_t*)&MidiRx,1);
    wr=file.write((uint8_t*)&MidiMode,1);
    wr=file.write((uint8_t*)&MidiRelCC,1);
    wr=file.write((uint8_t*)&MidiRelMin,1);        
    wr=file.write((uint8_t*)&MidiRelMax,1);
    Serial.printf("Save file midirx.cfg\n");
    file.close();   
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadMidiRx()
{
    char path[30];
    uint16_t rd;
    // Read the Midirx in a file
    sprintf(path,"/System/midirx.cfg");
    File file = SD_MMC.open(path,"rb");
    rd=file.read((uint8_t*)&MidiRx,1);
    rd=file.read((uint8_t*)&MidiMode,1);
    rd=file.read((uint8_t*)&MidiRelCC,1);
    rd=file.read((uint8_t*)&MidiRelMin,1);        
    rd=file.read((uint8_t*)&MidiRelMax,1);
    Serial.printf("Load file midirx.cfg\n");

    float temp;
    temp=((float)MidiMode/127)*MIDI_MODE_MAX;
    RealMidiMode=temp;    

   file.close();   
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadBackDelay()
{
    char path[30];
    uint16_t rd;
    // Read the Midirx in a file
    sprintf(path,"/System/BackDelay.cfg");
    File file = SD_MMC.open(path,"rb");
    if(file)
    {
        rd=file.read((uint8_t*)&BackDelay,1);
        file.close();   
    }
    else
    {
        BackDelay=10;
    }
}




/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_ReadSndName(uint8_t s)
{
uint8_t* ptname; 

    ptname = tabname+s*SDCARD_NAME_SIZE;
    memcpy(SndName,ptname,SDCARD_NAME_SIZE);
    SndName[SDCARD_NAME_SIZE-2]=0x00;
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_WriteSndName(uint8_t s)
{
uint8_t* ptname; 

    ptname = tabname+s*SDCARD_NAME_SIZE;
    memcpy(ptname,SndName,SDCARD_NAME_SIZE);
    Serial.printf("New Name %s",SndName);

}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_LoadSndName()
{
char path[30];
uint8_t ret;
uint16_t rd;

    // 38 37 36 35 34 33 32 31 0D 0A -> 87654321 + CR + LF
    // seek from the beginning of the file

    sprintf(path,"/sound/Names.txt");
    File file = SD_MMC.open(path,"rb");

    // read all the names and store them in the memory
    rd=file.read(tabname,SDCARD_TAB_NAME);
    
    for(uint8_t n=0;n<20;n++)
    {
        SDCard_ReadSndName(n);
        Serial.printf("File Name %03d %s\n",n,SndName);        

    }
    
    file.close();    
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_SaveSndName()
{
char path[30];
uint8_t ret;
uint16_t wr;
uint8_t* pt;

    // 38 37 36 35 34 33 32 31 0D 0A -> 87654321 + CR + LF
    // seek from the beginning of the file
    
    sprintf(path,"/sound/Names.txt");
    File file = SD_MMC.open(path,"wb");
    pt=tabname;
    for(uint16_t j=0;j<SDCARD_TAB_NAME;j++)
    {
        wr=file.printf("%c",*pt);
        pt++;
    }
    file.close();    
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void SDCard_Display10SndName()
{
    for(uint8_t i=0;i<10;i++)
    {
        SDCard_ReadSndName(i+SoundNameInc10*10);
        sprintf(messnex,"page2.b%d.txt=%c%s%c",i,0x22,SndName,0x22);
        Nextion_Send(messnex);
        Serial.printf("%s\n",SndName);
    }
}

/***************************************************/
/* bk bank                                         */
/* wa wavefrom                                     */
/*                                                 */
/***************************************************/
void SDCard_LoadWave(uint8_t bk,uint8_t wa)
{
char path[30];   
uint16_t rd,j; 
uint8_t tabspl[1024*2];
int16_t tmp16;
uint8_t *pt;

    Serial.printf("Load wave bk %d wa %d\n",bk,wa);
    pt = &tabspl[0];
    sprintf(path,"/AKWF/%03d/%03d.raw",bk,wa);
    File file = SD_MMC.open(path,"rb");
    rd=file.read(pt,1024*2);
    Serial.printf("File read %s data %d\n",path,rd);
    j=0;
    for (int i = 0; i < WAVEFORM_CNT; i++)
    {
        tmp16 = tabspl[j+1]<<8;
        tmp16 +=tabspl[j];
        waveAKWF[i] = (float)(tmp16)/32768.0f;
        waveAKWF[i+WAVEFORM_CNT/2] = waveAKWF[i];
        j+=2;
    }
    file.close();    
}




