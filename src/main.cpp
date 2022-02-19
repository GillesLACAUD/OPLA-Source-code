//--------------------------------------------------------------------------
// OPLA HANSY SYNTH 
// V1.0     15.10.21 
//
//
//
//
//
//
//--------------------------------------------------------------------------


#include <arduino.h>
#include "typdedef.h"

//#include "AudioFileSourcePROGMEM.h"
//#include "AudioGeneratorRTTTL.h"
//#include "AudioOutputI2S.h"
#include "AudioOutputI2S.h"
#include "AC101.h"
#include "ES8388.h"
#include <WiFi.h>
#include <driver/i2s.h>

#include <SPI.h>
#include "FS.h"
#include "SD_MMC.h"


#define __GLOBAL__
#include "Global.h"
#include "easysynth.h"
#include "midi_interface.h"
#include "AC101.h"
#include "i2s_interface.h"
#include "Nextion.h"
#include "Lfo.h"
#include "SDCard.h"
#include "Simple_Delay.h"
#include "Reverb.h"
#include "Ihm.h"
#include "ArpSeq.h"

#define __CODEC__
#include "Codec.h"

TaskHandle_t  Core0TaskHnd;

float fl_sample, fr_sample;

//********************************************************************************
// HARDWARE CONFIGURATION WITH AC101
//
//                  00  DO NOT USE    
//
//  RXD0            37  DOWNLOAD SERIAL PORT
//  TXD0            36  DOWNLOAD SERIAL PORT
//
//  RXD2            22  ESP32 Audio KIT MIDI IN
//  TXD2            19  ESP32 Audio KIT MIDI OUT
//
//  IIS_SCLK        27  CODEC AC101 BCLOCK
//  IIS_LCLK        26  CODEC AC101 LEFT RIGHT
//  IIS_DSIN        25  CODEC AC101 DATA IN
//
//  IIC_CLK         32  CODEC I2C
//  IIC_DATA        33  CODEC I2C
//
//  GPIO_PA_EN      21  POWER AMPLIFIER
//
//  NEXTION TX      23
//  NEXTION RX      18
//
//                  02  SD CARD DATA 0
//                  04  SD CARD DATA 1
//                  12  SD CARD DATA 2      JTAG CONNECTOR
//                  13  SD CARD DATA 3
//                  15  SD CARD CMD 
//                  14  SD CARD CLK         JTAG CONNECTOR
//                  34  SD DETECT
//
//********************************************************************************

//********************************************************************************
// CODEC AC101
// ESP32AudioCodec.i2c_sda=     33;
// ESP32AudioCodec.i2c_scl=     32;
// ESP32AudioCodec.i2s_blck=    27;
// ESP32AudioCodec.i2s_wclk=    26;
// ESP32AudioCodec.i2s_dout=    25;
// ESP32AudioCodec.i2s_din=     35;
// ESP32AudioCodec.i2s_mclk=    0;
//
// CODEC ES8388 OLD VERSION
// ESP32AudioCodec.i2c_sda=     18;         Same as Nextion RX
// ESP32AudioCodec.i2c_scl=     23;         Same as Nextion TX
// ESP32AudioCodec.i2s_blck=    5;
// ESP32AudioCodec.i2s_wclk=    25;
// ESP32AudioCodec.i2s_dout=    26;
// ESP32AudioCodec.i2s_din=     35;
// ESP32AudioCodec.i2s_mclk=    0;
//
// CODEC ES8388 NEW VERSION
// ESP32AudioCodec.i2c_sda=     33;
// ESP32AudioCodec.i2c_scl=     32;
// ESP32AudioCodec.i2s_blck=    27;
// ESP32AudioCodec.i2s_wclk=    25;
// ESP32AudioCodec.i2s_dout=    26;     
// ESP32AudioCodec.i2s_din=     35;     
// ESP32AudioCodec.i2s_mclk=    0;
//********************************************************************************


//********************************************************************************
// INTERFACE
//
//  WAVEFORM 1
//  WAVEFORM 2
//
//  CUTOFF
//  RESONANCE
//
//  A  AMP 
//  D  AMP 
//  S  AMP 
//  R  AMP 
//
//  A  FILTER 
//  D  FILTER
//  S  FILTER
//  R  FILTER 
//
//********************************************************************************


AudioOutputI2S *out;
/*
 * You can modify the sample rate as you want
 */
AC101 ac;
uint8_t AC101_volume;

/*
 * this is more an experiment required for other data formats
 */
union sampleTUNT
{
    uint64_t sample;
    int32_t ch[2];
} sampleDataU;

union sampleTUNT32
{
    int32_t sample32;
    int16_t sample[2];
}sampleData32;


portMUX_TYPE timer1Mux_xms = portMUX_INITIALIZER_UNLOCKED;
portMUX_TYPE timer2Mux_xms = portMUX_INITIALIZER_UNLOCKED;

portMUX_TYPE timerMux_1ms = portMUX_INITIALIZER_UNLOCKED;


    // LFO
    // 1024 point for the LFO
    // Frequency min 0.5Hertz - 2 secondes
    // Frequency max 100Hertz - 0.001 secondes
    //Lfo1.ui16_Cpt = Lfo_cnt1;
    //Serial.printf("CPT %04d sine %f CUT LFO %f\n",Lfo1.ui16_Cpt,sine[Lfo1.ui16_Cpt],filtCutoffLfo);

void IRAM_ATTR onTimer1()
{
    if(Lfo1_Mutex)
        return;
    Lfo1_Mutex=1;
    Lfo_cnt1+=1;
    Lfo1_Mutex=0;
}

void IRAM_ATTR onTimer2()
{
    if(Lfo2_Mutex)
        return;
    Lfo2_Mutex=1;
    Lfo_cnt2+=1;
    Lfo2_Mutex=0;

}

void IRAM_ATTR onTimer1ms()
{
    //portENTER_CRITICAL_ISR(&timerMux_1ms);
    Timer1ms_cnt++;
    //portEXIT_CRITICAL_ISR(&timerMux_1ms);    
    if(!u8_ArpTrig)
    {
        u8_ArpCptHitKey++;
    }
}

void CoreTask0( void *parameter )
{
    while (true)
    {
     
        for(uint8_t m=0;m<MAX_MESS_TASK0;m++)
        {
            if(messnexfree[m]==1)
            {
                Serial1.printf(messnextask[m]);
                Serial1.write(0xff);
                Serial1.write(0xff);
                Serial1.write(0xff);
                //Serial.printf("TASK0-------%s\n",messnextask[m]);
                messnexfree[m]=0;
            }
        }
        Nextion_Process();
        /* this seems necessary to trigger the watchdog */
        delay(5);
        yield();
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void setup()
 {
     
char AffCodec[15]="Not Define";
char AffVersion[30]="V15 130222";
                    


  // put your setup code here, to run once:
    delay(500);

    Serial.begin(115200);

    
    Serial.println();

    
    Serial.printf("Initialize Synth Module\n");
    Synth_Init();
 
    
    //---------------------------------------
    // Scan I2C Codec Test
    //---------------------------------------
    byte error, address;
    int nDevices;

    ESP32AudioCodec.i2c_port = Codec_I2C_NOTDEFINE;
    
    
    Serial.println("----------------------------------------");
    Serial.println("Scanning I2C bus 33 32");
    Serial.println("----------------------------------------");

    ESP32AudioCodec.i2c_sda=33;
    ESP32AudioCodec.i2c_scl=32;

    Wire.begin(ESP32AudioCodec.i2c_sda,ESP32AudioCodec.i2c_scl); 

    // TRY I2C Pin 33 32
    nDevices = 0;
    for(address = 1; address < 127; address++ )
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
    
        if (error == 0)
        {
            Serial.print("33 32 I2C device found at address 0x");
            Serial.print(address, HEX);
            Serial.println("\n");
            ESP32AudioCodec.i2c_addr = address;
            ESP32AudioCodec.i2c_port = Codec_I2C_PORT0;
        }
        else if (error==4)
        {
            Serial.print("33 32Unknown error at address 0x");
            if (address<16)
                Serial.print("0");
            Serial.println(address,HEX);
        }    
    }
    delay (250);

    if(ESP32AudioCodec.i2c_port == Codec_I2C_NOTDEFINE)
    {
        // TRY I2C Pin 18 23
        Serial.println("----------------------------------------");
        Serial.println("Scanning  I2C bus 18 23...");
        Serial.println("----------------------------------------");

        ESP32AudioCodec.i2c_sda=18;
        ESP32AudioCodec.i2c_scl=23;

        Wire.begin(ESP32AudioCodec.i2c_sda,ESP32AudioCodec.i2c_scl);         
       
        nDevices = 0;
        for(address = 1; address < 127; address++ )
        {
            Wire.beginTransmission(address);
            error = Wire.endTransmission();
        
            if (error == 0)
            {
                Serial.print("18 23 I2C device found at address 0x");
                if (address<16)
                    Serial.print("0");
                Serial.print(address, HEX);
                Serial.println("\n");
                nDevices++;
                ESP32AudioCodec.i2c_addr = address;
                ESP32AudioCodec.i2c_port = Codec_I2C_PORT1;
            }
            else if (error==4)
            {
                Serial.print("18 23 Unknown error at address 0x");
                if (address<16)
                    Serial.print("0");
                Serial.println(address,HEX);
            }    
        }
    }
    
    // BIG ISSUE CODEC NOT FOUND
    if(ESP32AudioCodec.i2c_port == Codec_I2C_NOTDEFINE)
    {
        while(1);
    }

    if(ESP32AudioCodec.i2c_addr==Codec_AC101_ADDR)
    {
        ESP32AudioCodec.Codec_Id = Codec_ID_AC101;
        Serial.printf("Try Connect to AC101 codec...\n");
    	while (not ac.begin(IIC_DATA, IIC_CLK))
    	{
  	        Serial.printf("Failed!\n");
		    delay(1000);
    	}
        Serial.printf("AC101 OK\n");
        sprintf(AffCodec,"AC101");
        AC101_volume = 99;
        ac.SetVolumeSpeaker(AC101_volume);
        ac.SetVolumeHeadphone(AC101_volume);

        ESP32AudioCodec.i2s_blck=   27;
        ESP32AudioCodec.i2s_wclk=   26;
        ESP32AudioCodec.i2s_dout=   25;
        ESP32AudioCodec.i2s_din=    35;
        ESP32AudioCodec.i2s_mclk=   0;
    }


    if(ESP32AudioCodec.i2c_addr==Codec_ES8388_ADDR)
    {
        delay (250);
        Serial.printf("Try Connect to ES8388 codec...\n");
        sprintf(AffCodec,"ES8388");
        if(ESP32AudioCodec.i2c_sda==33)
        {
            Serial.printf("Try Connect to ES8388 codec...33 32\n");
            sprintf(AffCodec,"ES8388V2");
            Serial.printf("ES8388V2 OK\n");
            ESP32AudioCodec.i2s_blck=   27;
            ESP32AudioCodec.i2s_wclk=   25;
            ESP32AudioCodec.i2s_dout=   26;     
            ESP32AudioCodec.i2s_din=    35;     
            ESP32AudioCodec.i2s_mclk=   0;
        }
        if(ESP32AudioCodec.i2c_sda==18)
        {
            Serial.printf("Try Connect to ES8388 codec...18 23\n");
            sprintf(AffCodec,"ES8388V1");
            Serial.printf("ES8388V1 BAD\n");
            ESP32AudioCodec.i2s_blck=    5;
            ESP32AudioCodec.i2s_wclk=    25;
            ESP32AudioCodec.i2s_dout=    26;
            ESP32AudioCodec.i2s_din=     35;
            ESP32AudioCodec.i2s_mclk=    0;
        }
        ES8388_rawSetup(ESP32AudioCodec.i2c_sda,ESP32AudioCodec.i2c_scl);
        
    }

    Serial.printf("-------------------BEGIN I2S PORT\n");

    out = new AudioOutputI2S();
    out->SetPinout(ESP32AudioCodec.i2s_blck,ESP32AudioCodec.i2s_wclk,ESP32AudioCodec.i2s_dout);
    out->begin();
    Serial.printf("-------------------END INIT I2S PORT\n");
    
    if(0)
    {
        int16_t cpt=0;
        int16_t cpt2=0;
        while(1)
        {
            if(cpt<100)    
            {
                fl_sample=1; 
                fr_sample=1; 
                i2s_write_stereo_samples(&fl_sample, &fr_sample);
            }
            else
            {
                fl_sample=0; 
                fr_sample=0; 
                i2s_write_stereo_samples(&fl_sample, &fr_sample);
            }
            cpt++;
            if(cpt==200)
                cpt=0;

            if(cpt2==5000)
            {
                //Serial.printf("Read Key3 %d\n",digitalRead(19));
                cpt2=0;
            }
            cpt2++;
        }
    }

    Serial.printf("Initialize Midi Module\n");
    Midi_Setup();

    Serial.printf("Turn off Wifi/Bluetooth\n");

#ifndef ESP8266
    btStop();
    //esp_wifi_deinit();
#endif

    uint8_t sdcard=SDCard_Init();

    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Delay_Init();
    Reverb_Setup();

    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    /* PSRAM will be fully used by the looper */
    Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());

    uint32_t Freq = 0;
    Freq = getCpuFrequencyMhz();
    Serial.print("CPU Freq = ");
    Serial.print(Freq);
    Serial.println(" MHz");
    Freq = getXtalFrequencyMhz();
    Serial.print("XTAL Freq = ");
    Serial.print(Freq);
    Serial.println(" MHz");
    Freq = getApbFrequency();
    Serial.print("APB Freq = ");
    Serial.print(Freq);
    Serial.println(" Hz");


    Serial.printf("Firmware started successfully\n");
    Serial.printf("\n");

    
    Lfo_timer1 = timerBegin(LFO_ID1, 80, true);             // 80 Prescaler = 1MHertz 800 100KHertz
    timerAlarmWrite(Lfo_timer1,LFO_MAX_TIME,true);          // 1024 -> T=1s 2048 T=2s 16=2048/127 T=16ms = 32 Hertz
    timerAttachInterrupt(Lfo_timer1, &onTimer1, true);
    timerAlarmEnable(Lfo_timer1);

    Lfo_timer2 = timerBegin(LFO_ID2, 80, true);              // 80 Prescaler = 1MHertz 800 100KHertz
    timerAlarmWrite(Lfo_timer2,LFO_MAX_TIME,true);           // 1024 -> T=1s 2048 T=2s 16=2048/127 T=16ms = 32 Hertz
    timerAttachInterrupt(Lfo_timer2, &onTimer2, true);
    timerAlarmEnable(Lfo_timer2);

    timer_1ms = timerBegin(TIMER_1MS, 80, true);             // 80 Prescaler = 1MHertz 800 100KHertz
    timerAlarmWrite(timer_1ms,1000,true);                    // 
    timerAttachInterrupt(timer_1ms, &onTimer1ms, true);
    timerAlarmEnable(timer_1ms);
    

    xTaskCreatePinnedToCore(CoreTask0, "terminalTask", 8000, NULL,0, &Core0TaskHnd, 0);

    Nextion_Init();

        
    // SHOW SD CARD AND FIRMWARE VERSION
    sprintf(messnex,"page0.b2.txt_maxl=80");
    Nextion_Send(messnex);

    if(sdcard)
    {
        sprintf(messnex,"page0.b2.txt=%c>SDCARD HS %s%c%c>%s%c",0x22,AffCodec,0x0D,0x0A,AffVersion,0x22);
        Nextion_Send(messnex);
    }
    else
    {
        sprintf(messnex,"page0.b2.txt=%c>SDCARD OK %s%c%c>%s%c",0x22,AffCodec,0x0D,0x0A,AffVersion,0x22);
        Nextion_Send(messnex);
        // Load all the names in the memory
        SDCard_LoadSndName();
        // Send 10 first names to the Nextion screen
        SoundNameInc10=0;
        SDCard_Display10SndName();
    }

    SDCard_LoadLastSound();
    SDCard_LoadMidiRx();

    Serial.printf("Midi Rx is      %d\n",MidiRx);
    Serial.printf("Midi Mode is    %d\n",MidiMode);
    Serial.printf("Midi Rel CC is  %d\n",MidiRelCC);
    Serial.printf("Midi Rel Min is %d\n",MidiRelMin);
    Serial.printf("Midi Rel Max is %d\n",MidiRelMax);

    SDCard_LoadBackDelay();
    Serial.printf("BackDelay is %d\n",BackDelay);
    Nextion_PrintLabel();
    Nextion_PrintValues();
    //Synth_NoteOn(64-12);
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
bool i2s_write_sample_16ch2(uint32_t samplebis)
{
    static size_t bytes_written = 0;
    i2s_write((i2s_port_t)0, (const char *)&samplebis, 4, &bytes_written, portMAX_DELAY);
    if (bytes_written > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void loop()
{
static uint16_t cpttimer1;    
static uint16_t cpttimer2;    
static uint8_t onetime;    
    // put your main code here, to run repeatedly:

    static uint8_t loop_count_u8 = 0;

    loop_count_u8++;
    overcpt++;
    Cptloadwave++;

    // ARP Wait xms to all key on 
    if(u8_ArpCptHitKey>MAX_ARP_DELAY_HITKEYS && !u8_ArpTrig && u8_ArpNbKeyOn)
    {
        u8_ArpCptHitKey=0;
        u8_ArpTrig=1;
        switch(u8_ArpMode)
	    {
            case ARP_MODE_UP:  u8_ArpCptStep=255;u8_ArpUpDwn=ARP_UP;i8_ArpWay=1;break;
            case ARP_MODE_ORDER:  u8_ArpCptStep=255;u8_ArpUpDwn=ARP_UP;i8_ArpWay=1;break;
            case ARP_MODE_UP2:  u8_ArpCptStep=0;u8_ArpUpDwn=ARP_UP;i8_ArpWay=1;break;
            case ARP_MODE_DWN: u8_ArpCptStep=u8_ArpNbKeyOn;u8_ArpUpDwn=ARP_DOWN;i8_ArpWay=-1;break;
            case ARP_MODE_DWN2: u8_ArpCptStep=u8_ArpNbKeyOn-1;u8_ArpUpDwn=ARP_DOWN;i8_ArpWay=-1;break;
            case ARP_MODE_INC: u8_ArpCptStep=255;u8_ArpUpDwn=ARP_UP;i8_ArpWay=1;break;
            case ARP_MODE_EXC: u8_ArpCptStep=255;u8_ArpUpDwn=ARP_UP;i8_ArpWay=1;break;
        }
        u8_ArpRepeat=0;
        Serial.printf("START ARP Send %d\n",u8_ArpUpDwn);
        Arp_Filter_Note();
    }


    // Arpegiator timer
    static float toogle_swing=0.5;
    if(u8_ArpOn && u8_ArpTrig)
    {
        if(Timer1ms_cnt > u32_ArpTimeOff*(1+toogle_swing*ArpSwing) && onetime==0)
        {
            Arp_Stop_Note();
            onetime=1;
        }
        if(Timer1ms_cnt > u32_ArpTime*(1+toogle_swing*ArpSwing))
        {
            Arp_Play_Note();
            Timer1ms_cnt=0;
            onetime=0;
            if(toogle_swing==0.5)
            toogle_swing=-0.5;
            else
            toogle_swing=0.5;
        }
    }

    if(overon && overcpt > NEXTION_MAX_OVER_TIME)
    {
       overon=false;
       // Pre update the values
       Nextion_PrintLabel();
       Nextion_PrintValues();
       // Change page
       sprintf(messnex,"page 1");
       Nextion_Send(messnex);
    }

    if (Lfo_cnt1 >= 1024)
    {   
        cpttimer1++;
        if(Lfo1.ui8_Sync !=LFO_ONE)
            Lfo_cnt1=0;
        else
            Lfo_cnt1=1024;
    }
    if (Lfo_cnt2 >= 1024)
    {
        cpttimer2++;
        if(Lfo2.ui8_Sync !=LFO_ONE)
            Lfo_cnt2=0;
        else
            Lfo_cnt2=1024;
    }

    // Reset all the mod
    if (loop_count_u8 % 2 == 0)
    {

        FiltCutoffMod = 0;
        PanMod=0;
        AmpMod=0;
        NoiseMod=0;
        WaveShapping1Mod=0;
        WaveShapping2Mod=0;

        Lfo1AmtMod=0;
        Lfo2AmtMod=0;

        Lfo1SpeedMod = 0;
        Lfo2SpeedMod = 0;
        
        RevAmtMod=0;
        DelayAmtMod=0;
    

        ModWheel_Process();
        AfterTouch_Process();
        if(!Lfo1_Mutex)
        {
            Lfo1_Mutex=1;
            Lfo_Process(&Lfo1);
            Lfo1_Mutex=0;
        }
        if(!Lfo2_Mutex)
        {
            Lfo2_Mutex=1;
            Lfo_Process(&Lfo2);
            Lfo2_Mutex=0;
        }
    }

    //if (i2s_write_stereo_samples(&fl_sample, &fr_sample))
    /*
    i2s_read_stereo_samples_buff(&fl_sample,&fr_sample,SAMPLE_BUFFER_SIZE);
    sampleData32.sample[0] = (int16_t)(fl_sample*32768.0f);
    sampleData32.sample[1] = (int16_t)(fr_sample*32768.0f);
    */

    if(i2s_write_sample_16ch2(sampleData32.sample32))
    {
        Synth_Process(&fl_sample, &fr_sample);
        
        if(SoundMode!=SND_MODE_POLY)
        {
            if(WS.DelayAmount !=0)
                Delay_Process(&fl_sample, &fr_sample);
        }
        if(WS.ReverbLevel !=0)
        {
            Reverb_Process( &fl_sample, &fr_sample, SAMPLE_BUFFER_SIZE );       
        }
       
        sampleData32.sample[0] = (int16_t)(fl_sample*32768.0f);
        sampleData32.sample[1] = (int16_t)(fr_sample*32768.0f);
    }

    /*
     * Midi does not required to be checked after every processed sample
     * - we divide our operation by 8
     */
    if (loop_count_u8 % 8 == 0)
    {
        Midi_Process();
        if(trigloadwave && Cptloadwave > LOADWAVE_MAX_OVER_TIME)
        {
            trigloadwave=false;
            SDCard_LoadWave(WS.OscBank+1,WS.AKWFWave+1);
            Nextion_Plot();
            Fct_Ch_WS1(WS.WaveShapping1);
        }
        //Nextion_Process(); // in the task0
    }
}