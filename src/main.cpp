#include <arduino.h>
#include "typdedef.h"

//#include "AudioFileSourcePROGMEM.h"
//#include "AudioGeneratorRTTTL.h"
//#include "AudioOutputI2S.h"
#include "AudioOutputI2S.h"
#include "AC101.h"
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

TaskHandle_t  Core0TaskHnd;

//********************************************************************************
// HARDWARE CONFIGURATION
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

void onTimer1()
{
static uint8_t scaler=0;

    if(Lfo1_Mutex)
        return;
    Lfo1_Mutex=1;
    scaler++;
    if(scaler==1)
    {
        scaler=0;
        Lfo_cnt1+=1;
    }
    Lfo1_Mutex=0;
}

void onTimer2()
{
static uint8_t scaler=0;

    if(Lfo2_Mutex)
        return;
    Lfo2_Mutex=1;
    scaler++;
    if(scaler==1)
    {
        scaler=0;
        Lfo_cnt2+=1;
    }
    Lfo2_Mutex=0;

}

void onTimer1ms()
{
    //portENTER_CRITICAL_ISR(&timerMux_1ms);
    Timer1ms_cnt++;
    //portEXIT_CRITICAL_ISR(&timerMux_1ms);    
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

  // put your setup code here, to run once:
    delay(500);

    Serial.begin(115200);

    Serial.println();

    Serial.printf("Initialize Synth Module\n");
    Synth_Init();
    
    out = new AudioOutputI2S();
    out->SetPinout(IIS_SCLK /*bclkPin*/, IIS_LCLK /*wclkPin*/, IIS_DSIN /*doutPin*/);
    out->begin();
    Serial.printf("AUDIO OUT I2C OK\n");

    Serial.printf("Connect to AC101 codec... ");
	while (not ac.begin(IIC_DATA, IIC_CLK))
	{
  	    Serial.printf("Failed!\n");
		delay(1000);
	}
	Serial.printf("OK\n");
    AC101_volume = 50;
    ac.SetVolumeSpeaker(AC101_volume);
	ac.SetVolumeHeadphone(AC101_volume);
 
    Serial.printf("Initialize Midi Module\n");
    Midi_Setup();

    Serial.printf("Turn off Wifi/Bluetooth\n");

#ifndef ESP8266
    btStop();
    //esp_wifi_deinit();
#endif

    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());

    Serial.printf("Firmware started successfully\n");

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


    // put your main code here, to run repeatedly:
    static uint32_t loop_cnt_1hz;
    static uint8_t loop_count_u8 = 0;

    loop_count_u8++;

    loop_cnt_1hz++;
    overcpt++;
    if (loop_cnt_1hz >= 44)
    {
        //Loop_1Hz();
        //sprintf(messnex,"page0.Setup_Name.txt=%c1 %04d 2 %04d%c",0x22,Lfo_cnt1,Lfo_cnt2,0x22);
        //Nextion_Send(messnex);
    }
    if(overon && overcpt > NEXTION_MAX_OVER_TIME)
    {
       overon=false;
       // Pre update the values
       Nextion_PrintLabel();
       Nextion_PrintValues();
       // Change page
       sprintf(messnex,"page 0");
       Nextion_Send(messnex);
    }

    /*
    if(Timer1ms_cnt>=2)
    {
        // Reset all the mod
        
        FiltCutoffMod = 0;
        PanMod=0;
        AmpMod=0;
        NoiseMod=0;
        WaveShapping1Mod=0;
        
        Lfo_Process(&Lfo1);
        Lfo_Process(&Lfo2);

        Timer1ms_cnt = 0;
    }
    */


    if (Lfo_cnt1 >= 1024)
    {   
        //sprintf(messnex,"page0.Setup_Name.txt=%cLFO1 %d%c",0x22,cpttimer1,0x22);
        //Nextion_Send(messnex);
        cpttimer1++;
        Lfo_cnt1=0;
    }
    if (Lfo_cnt2 >= 1024)
    {
        //sprintf(messnex,"page0.Setup_Name.txt=%cLFO2 %d%c",0x22,cpttimer2,0x22);
        //Nextion_Send(messnex);
        cpttimer2++;
        Lfo_cnt2=0;
    }

    if(0)
    {
        if (i2s_write_sample_32ch2(sampleDataU.sample))  /* function returns always true / it blocks until samples are written to buffer */
        {
            float fl_sample, fr_sample;
            Synth_Process(&fl_sample, &fr_sample);

            sampleDataU.ch[0] = int32_t(fl_sample * 536870911.0f);
            sampleDataU.ch[1] = int32_t(fr_sample * 536870911.0f);
        }
    }
    if(1)
    {
        if(i2s_write_sample_16ch2(sampleData32.sample32))
        {
            float fl_sample, fr_sample;
            Synth_Process(&fl_sample, &fr_sample);
            sampleData32.sample[0] = (int16_t)(fl_sample*32768.0f);
            sampleData32.sample[1] = (int16_t)(fr_sample*32768.0f);
        }
    }

    /*
     * Midi does not required to be checked after every processed sample
     * - we divide our operation by 8
     */
    if (loop_count_u8 % 4 == 0)
    {
        Midi_Process();
        //Nextion_Process(); // in the task0
    }
}