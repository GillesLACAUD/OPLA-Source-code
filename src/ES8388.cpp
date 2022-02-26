#include "ES8388.h"



uint8_t ES8388_ReadReg(uint8_t reg)
{
    Wire.beginTransmission(ES8388_ADDR);
    Wire.write(reg);
    Wire.endTransmission(false);

    uint8_t val = 0u;
    if (1 == Wire.requestFrom(uint16_t(ES8388_ADDR), uint8_t(1), true))
    {
        val = Wire.read();
    }
    Wire.endTransmission(false);

    return val;
}

bool ES8388_WriteReg(uint8_t reg, uint8_t val)
{
    Wire.beginTransmission(ES8388_ADDR);
    Wire.write(reg);
    Wire.write(val);
    return 0 == Wire.endTransmission(true);
}

bool ES8388_begin(int sda, int scl, uint32_t frequency)
{
    bool ok = Wire.begin(sda, scl, frequency);

    // Reset all registers, readback default as sanity check
    //ok &= WriteReg(CHIP_AUDIO_RS, 0x123);
    delay(100);

    Serial.printf("0x00: 0x%02x\n", ES8388_ReadReg(ES8388_CONTROL1));
    Serial.printf("0x01: 0x%02x\n", ES8388_ReadReg(ES8388_CONTROL2));

    ES8388_WriteReg(ES8388_CONTROL1, 1 << 7); /* do reset! */
    ES8388_WriteReg(ES8388_CONTROL1, 0x06);
    ES8388_WriteReg(ES8388_CONTROL2, 0x50);

    ok &= (0x06 == ES8388_ReadReg(ES8388_CONTROL1));
    ok &= (0x50 == ES8388_ReadReg(ES8388_CONTROL2));
    return ok;
}

void es8388_read_all()
{
    for (int i = 0; i < 53; i++)
    {
        uint8_t reg = 0;
        reg = ES8388_ReadReg(i);
        if(i<9)
            Serial.printf("CTRL Reg %02d 0x%02x = 0x%02x\n",i, i, reg);
        else
        if(i<23)
            Serial.printf("ADC  Reg %02d 0x%02x = 0x%02x\n",i, i, reg);
        else
        if(i<=52)
        Serial.printf("DAC  Reg %02d 0x%02x = 0x%02x\n",i, i, reg);

    }
}


void ES8388_SetADCVOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("ADC Volume /db", (vol - 1) * 97 + 0.5);
#endif

    vol *= -192;
    vol += 192;

    uint8_t volu8 = vol;

    if (volu8 > 192)
    {
        volu8 = 192;
    }

    ES8388_WriteReg(0x10, volu8); // LADCVOL
    ES8388_WriteReg(0x11, volu8); // RADCVOL
}

void ES8388_SetDACVOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("DAC Volume /db", (vol - 1) * 97 + 0.5);
#endif

    vol *= -192;
    vol += 192;

    uint8_t volu8 = vol;

    if (volu8 > 192)
    {
        volu8 = 192;
    }

    ES8388_WriteReg(0x1A, volu8); // LDACVOL
    ES8388_WriteReg(0x1B, volu8); // RDACVOL
}

void ES8388_SetPGAGain(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("PGA Gain /db", vol * 24 + 0.25);
#endif

    vol *= 8;

    uint8_t volu8 = vol;

    if (volu8 > 8)
    {
        volu8 = 8;
    }
    // ES8388_ADCCONTROL1
    ES8388_WriteReg(0x09, volu8 + (volu8 << 4)); // MicAmpL, MicAmpR
}

void ES8388_SetInputCh(uint8_t ch, float var)
{
    if (var > 0)
    {
        uint8_t in;
        switch (ch)
        {
        case 0:
            in = 0;
            Serial.printf("AdcCh0!\n");
            break;

        case 1:
            in = 1;
            Serial.printf("AdcCh1!\n");
            break;

        default:
            Serial.printf("Illegal Input ch!\n");
            return;
        }
        // ES8388_ADCCONTROL2
        ES8388_WriteReg(0x0A, (in << 6) + (in << 4)); // LINSEL , RINSEL , DSSEL , DSR

#ifdef STATUS_ENABLED
        Status_ValueChangedInt("ADC Ch", in);
#endif
    }
}

void ES8388_SetMixInCh(uint8_t ch, float var)
{
    if (var > 0)
    {
        uint8_t in = 0;
        switch (ch)
        {
        case 0:
            in = 0;
            Serial.printf("MixCh0!\n");
            break;

        case 1:
            in = 1;
            Serial.printf("MixCh1!\n");
            break;

        case 2:
            in = 3;
            Serial.printf("MixChAMPL!\n");
            break;

        default:
            Serial.printf("Illegal Mix Input ch!\n");
            return;
        }
        // ES8388_DACCONTROL16
        ES8388_WriteReg(0x26, in + (in << 3)); // LMIXSEL, RMIXSEL
#ifdef STATUS_ENABLED
        Status_ValueChangedInt("Mix In Ch", in);
#endif
    }
}

void ES8388_SetIn2OoutVOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("In to out volume /db", (vol - 1) * 16 + 0.5);
#endif

    vol *= -5;
    vol += 7;

    uint8_t volu8 = vol;

    if (volu8 > 7)
    {
        volu8 = 7;
    }

    uint8_t var;

    var = ES8388_ReadReg(0x27) & 0xC0;
    if (volu8 == 7)
    {
        var &= ~ 0x40;
    }
    else
    {
        var |= 0x40;
    }
    ES8388_WriteReg(0x27, (volu8 << 3) + var); // LD2LO, LI2LO, LI2LOVOL

    var = ES8388_ReadReg(0x2A) & 0xC0;
    if (volu8 == 7)
    {
        var &= ~ 0x40;
    }
    else
    {
        var |= 0x40;
    }
    ES8388_WriteReg(0x2A, (volu8 << 3) + var); // RD2RO, RI2RO, RI2ROVOL
}

void ES8388_SetOUT1VOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("OUT1VOL /db", (vol - 1) * 31 + 0.5);
#endif

    vol *= 0x1E;

    uint8_t volu8 = vol;

    if (volu8 > 129)
    {
        volu8 = 129;
    }

    ES8388_WriteReg(0x2E, volu8); // LOUT1VOL
    ES8388_WriteReg(0x2F, volu8); // ROUT1VOL
}

void ES8388_SetOUT2VOL(uint8_t unused, float vol)
{
#ifdef STATUS_ENABLED
    Status_ValueChangedInt("OUT2VOL /db", (vol - 1) * 31 + 0.5);
#endif

    vol *= 0x1E;

    uint8_t volu8 = vol;

    if (volu8 > 129)
    {
        volu8 = 129;
    }

    ES8388_WriteReg(0x30, volu8); // LOUT2VOL
    ES8388_WriteReg(0x31, volu8); // ROUT2VOL
}


int ES8388_Start()
{
    int res = 0;
    uint8_t prev_data = 0, data = 0;
    res |= ES8388_WriteReg(ES8388_DACCONTROL16, 0x09);  // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2 by pass enable
    res |= ES8388_WriteReg(ES8388_DACCONTROL17, 0xC0);  // left DAC to left mixer enable  and  LIN signal to left mixer enable 0db  : bupass enable
    res |= ES8388_WriteReg(ES8388_DACCONTROL20, 0xC0);  // right DAC to right mixer enable  and  LIN signal to right mixer enable 0db : bupass enable
    res |= ES8388_WriteReg(ES8388_DACCONTROL21, 0xC0);  //enable adc
    res |= ES8388_WriteReg(ES8388_CHIPPOWER, 0xF0);     //start state machine
    res |= ES8388_WriteReg(ES8388_CHIPPOWER, 0x00);     //start state machine
    //res |= ES8388_WriteReg(ES8388_ADCPOWER, 0x00);      //power up adc and line in

    res |= ES8388_WriteReg(ES8388_DACCONTROL4, 0x00);
    res |= ES8388_WriteReg(ES8388_DACCONTROL5, 0x00);

    //res |= ES8388_WriteReg(ES8388_DACPOWER, 0x3C);
    res |= ES8388_WriteReg(ES8388_DACPOWER, 0x3C);
    res= ES8388_ReadReg(ES8388_DACPOWER);
    Serial.printf("Reg Power 0X%02x\n",res);

    res |= ES8388_WriteReg(ES8388_DACCONTROL24, 0x1E);
    res |= ES8388_WriteReg(ES8388_DACCONTROL25, 0x1E);

    return res;
}


void ES8388_Setup()
{
    int res = 0;

    while (not ES8388_begin(ES8388_PIN_SDA, ES8388_PIN_SCL, 100000))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }

    res |= ES8388_WriteReg(ES8388_DACCONTROL3, 0x04);       // 0x04 mute/0x00 unmute&ramp;DAC unmute and  disabled digital volume control soft ramp
    /* Chip Control and Power Management */
    res |= ES8388_WriteReg(ES8388_CONTROL2, 0x50);
    res |= ES8388_WriteReg(ES8388_CHIPPOWER, 0x00);         //normal all and power up all
    res |= ES8388_WriteReg(ES8388_MASTERMODE, 0x00);        //CODEC IN I2S SLAVE MODE

    /* dac */
    res |= ES8388_WriteReg(ES8388_DACPOWER, 0xC0);          //disable DAC and disable Lout/Rout/1/2
    res |= ES8388_WriteReg(ES8388_CONTROL1, 0x13);          //Enfr=0,Play&Record Mode,(0x17-both of mic&paly)
//    res |= ES8388_WriteReg(ES8388_ADDR, ES8388_CONTROL2, 0);  //LPVrefBuf=0,Pdn_ana=0
    res |= ES8388_WriteReg(ES8388_DACCONTROL1, 0x18);       //1a 0x18:16bit iis , 0x00:24
    res |= ES8388_WriteReg(ES8388_DACCONTROL2, 0x02);       //DACFsMode,SINGLE SPEED; DACFsRatio,256
    res |= ES8388_WriteReg(ES8388_DACCONTROL16, 0x00);      // 0x00 audio on LIN1&RIN1,  0x09 LIN2&RIN2
    res |= ES8388_WriteReg(ES8388_DACCONTROL17, 0x90);      // only left DAC to left mixer enable 0db
    res |= ES8388_WriteReg(ES8388_DACCONTROL20, 0x90);      // only right DAC to right mixer enable 0db
    res |= ES8388_WriteReg(ES8388_DACCONTROL21, 0x80);      //set internal ADC and DAC use the same LRCK clock, ADC LRCK as internal LRCK
    res |= ES8388_WriteReg(ES8388_DACCONTROL23, 0x00);      //vroi=0
    //res |= es8388_set_adc_dac_volume(ES_MODULE_DAC, 0, 0);          // 0db
    int tmp = 0;
    tmp = DAC_OUTPUT_LOUT1 | DAC_OUTPUT_LOUT2 | DAC_OUTPUT_ROUT1 | DAC_OUTPUT_ROUT2;
    res |= ES8388_WriteReg(ES8388_DACPOWER, tmp);  //0x3c Enable DAC and Enable Lout/Rout/1/2
    /* adc */
    res |= ES8388_WriteReg(ES8388_ADCPOWER, 0xFF);
    res |= ES8388_WriteReg(ES8388_ADCCONTROL1, 0x00);       // MIC Left and Right channel PGA gain
    tmp = 0;
    tmp = ADC_INPUT_LINPUT1_RINPUT1;
    res |= ES8388_WriteReg(ES8388_ADCCONTROL2, tmp);        //0x00 LINSEL & RINSEL, LIN1/RIN1 as ADC Input; DSSEL,use one DS Reg11; DSR, LINPUT1-RINPUT1
    res |= ES8388_WriteReg(ES8388_ADCCONTROL3, 0x02);
    res |= ES8388_WriteReg(ES8388_ADCCONTROL4, 0x0d);       // Left/Right data, Left/Right justified mode, Bits length, I2S format
    res |= ES8388_WriteReg(ES8388_ADCCONTROL5, 0x02);       //ADCFsMode,singel SPEED,RATIO=256
    //ALC for Microphone
    //res |= es8388_set_adc_dac_volume(ES_MODULE_ADC, 0, 0);      // 0db
    res |= ES8388_WriteReg(ES8388_ADCPOWER, 0x09);          //Power on ADC, Enable LIN&RIN, Power off MICBIAS, set int1lp to low power mode
    /* enable es8388 PA */
    //es8388_pa_power(true);
    //return res;
    // Add for audio input

}



void ES8388_rawSetup(uint8_t sda,uint8_t scl)
{
    while (not ES8388_begin(sda,scl, 400000))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }
    ES8388_WriteReg(ES8388_CONTROL1,18); 	    // 0x12	0x12	0x05	X	Reg 0x00	00	0001 0010: chip control settings
    ES8388_WriteReg(ES8388_CONTROL2,80); 	    // 0x50	0x50	0x40	X	Reg 0x01	01	0101 0000: chip control settings
    ES8388_WriteReg(ES8388_CHIPPOWER,0); 	    // 0x00	0x00	0x00	-	Reg 0x02	02 	0000 0000: chip power management
    ES8388_WriteReg(ES8388_ADCPOWER,0); 	    // 0x08	0x00	0x00	-	Reg 0x03	03	0000 0000: Activating ADC
    ES8388_WriteReg(ES8388_ADCPOWER,0xFF); 
    ES8388_WriteReg(ES8388_DACPOWER,60); 	    // 0x30	0x3C	0x3C	-	Reg 0x04	04	0011 1100: Activating DAC
    ES8388_WriteReg(ES8388_MASTERMODE,0);       // 0x00	0x00	0x00	-	Reg 0x08	08	0000 0000: slave serial port mode 

    // ADC CONFIG
    ES8388_WriteReg(ES8388_ADCCONTROL1,136);    // 0xBB	0x88	0x88 	-	Reg 0x09	09	1000 1000: mic preamps gains
    ES8388_WriteReg(ES8388_ADCCONTROL1,0);    // 0xBB	0x88	0x88 	-	Reg 0x09	09	1000 1000: mic preamps gains
    ES8388_WriteReg(ES8388_ADCCONTROL4,12); 	// 0x0C	0x0C	0x0C 	-	Reg 0x0C	12	0000 1100: settings i2s config (16 bits)
    ES8388_WriteReg(ES8388_ADCCONTROL5,2); 	    // 0x02	0x02	0x02	-	Reg 0x0D	13	0000 0010: ADC MCLK at 256 fo ADC
    ES8388_WriteReg(ES8388_ADCCONTROL8,0); 	    // 0x00	0x00	-	        Reg 0x10	16	0000 0000: Digital volume control attenuates the signal in 0.5 dB incremental at 0dB for ADC L
    ES8388_WriteReg(ES8388_ADCCONTROL9,0); 	    // 0x00	0x00	-	        Reg 0x11	17	0000 0000: Digital volume control attenuates the signal in 0.5 dB incremental at 0dB for ADC R
    ES8388_WriteReg(ES8388_ADCCONTROL10,12);    // 0xDA	0x0C	0x16	X	Reg 0x12	18	0000 1100: PGA gain

    // DAC CONFIG
    ES8388_WriteReg(ES8388_DACCONTROL1,24); 	// 0x18	0x18	0x18	-	Reg 0x17	23	0001 1000: i2s config (16 bits)
    ES8388_WriteReg(ES8388_DACCONTROL2,2); 	    // 0x02	0x02	0x02	-	Reg 0x18	24	0000 0010: DACFsRatio (Master mode DAC MCLK to sampling frequency ratio) is set to 256 
    ES8388_WriteReg(ES8388_DACCONTROL4,0); 	    // 0x00	0x00	0x00	-	Reg 0x1A	26	0000 0000: gain of output (L) is 0dB
    ES8388_WriteReg(ES8388_DACCONTROL5,0); 	    // 0x00	0x00	0x02	X	Reg 0x1B	27	0000 0000: gain of output (R) is 0dB  
    ES8388_WriteReg(ES8388_DACCONTROL17,0xE0);  // 0x80	0x50	0xB8	X	Reg 0x27	39	1001 0000: left DAC to left mixer enabled + LIN signal to left mixer gain is 0dB
    ES8388_WriteReg(ES8388_DACCONTROL20,0xE0);  // 0x80	0x50	0xB8	X	Reg 0x2A	42	1001 0000: same config as above but for right mixer
    ES8388_WriteReg(ES8388_DACCONTROL21,128);   // 0x80	0x80	0x80	-	Reg 0x2B	43	1000 0000: DACLRC and ADCLRC are same
    ES8388_WriteReg(ES8388_DACCONTROL24,29);    // 0x1F	0x1E	0x1E	-	Reg 0x2E	46	0001 1110: DAC volume is 0dB (L)
    ES8388_WriteReg(ES8388_DACCONTROL25,29);    // 0x1F	0x1E	0x1E	-	Reg 0x2F	47	0001 1110: DAC volume is 0dB (R)

    ES8388_SetOUT1VOL(0,0.95);
    ES8388_SetOUT2VOL(0,0.95);

    // Set input2 to output
    // Input line IN2 0101 0000 = 0x50
    ES8388_WriteReg(ES8388_ADCCONTROL2,0x50); 
    // Output Mix 0000 1001 L adn R IN2
    ES8388_WriteReg(ES8388_DACCONTROL16,0x09);
    // Output Mix 0000 0000 L adn R IN1
    ES8388_WriteReg(ES8388_DACCONTROL16,0x00);    // Mute input 

    REG_WRITE(PIN_CTRL, 0xFFFFFFF0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0_CLK_OUT1);

}

void oldES8388_Setup()
{
    Serial.printf("Connect to ES8388 codec... ");

    while (not ES8388_begin(ES8388_PIN_SDA, ES8388_PIN_SCL, 400000))
    {
        Serial.printf("Failed!\n");
        delay(1000);
    }

    ES8388_WriteReg(ES8388_CHIPPOWER, 0xFF);  //reset and stop es8388


    ES8388_WriteReg(0x00, 0x80); /* reset control port register to default */
    ES8388_WriteReg(0x00, 0x06); /* restore default value */



    /*
     * https://dl.radxa.com/rock2/docs/hw/ds/ES8388%20user%20Guide.pdf
     */

    /*
     * 10.5 Power Down Sequence (To Standby Mode)
     */
    ES8388_WriteReg(0x0F, 0x34); /* ADC Mute */
    ES8388_WriteReg(0x19, 0x36); /* DAC Mute */

    ES8388_WriteReg(0x02, 0xF3); /* Power down DEM and STM */

    /*
     * 10.4 The sequence for Start up bypass mode
     */
    /* Set Chip to Slave Mode */
    ES8388_WriteReg(0x08, 0x00);
    /* Power down DEM and STM */
    ES8388_WriteReg(0x02, 0x3F);
    /* Set same LRCK */
    ES8388_WriteReg(0x2B, 0x80);
    /* Set Chip to Play&Record Mode */
    ES8388_WriteReg(0x00, 0x05);
    /* Power Up Analog and Ibias */
    ES8388_WriteReg(0x01, 0x40);
#if 0
    /*
     * Power down ADC, Power up
     * Analog Input for Bypass
     */
    ES8388_WriteReg(0x03, 0x3F);
#else
    ES8388_WriteReg(0x03, 0x3F); /* adc also on but no bias */

    ES8388_WriteReg(0x03, 0x00); // PdnAINL, PdinAINR, PdnADCL, PdnADCR, PdnMICB, PdnADCBiasgen, flashLP, Int1LP
#endif

#if 0
    /*
     * Power Down DAC, Power up
     * Analog Output for Bypass
     */
    ES8388_WriteReg(0x04, 0xFC);
#else
    /*
     * Power up DAC / Analog Output
     * for Record
     */
    ES8388_WriteReg(0x04, 0x3C);


#if 1
    /*
     * Select Analog input channel for ADC
     */
    ES8388_WriteReg(0x0A, 0x80); // LINSEL , RINSEL , DSSEL , DSR

    /* Select PGA Gain for ADC analog input */
    ES8388_WriteReg(0x09, 0x00); // PGA gain?

    //ES8388_WriteReg(0x0C, 0x18); // DATSEL, ADCLRP, ADCWL, ADCFORMAT
    //ES8388_WriteReg(0x0C, 0x40); // DATSEL, ADCLRP, ADCWL, ADCFORMAT
    ES8388_WriteReg(0x0C, 0x0C); // DATSEL, ADCLRP, ADCWL, ADCFORMAT
    ES8388_WriteReg(0x0D, 0x02); // ADCFsMode , ADCFsRatio
    //ES8388_WriteReg(0x0D, (1<<5) + 0x03); // ADCFsMode , ADCFsRatio Hasan
    //ES8388_WriteReg(0x0D, 0); // ADCFsMode , ADCFsRatio Hasan

    /*
     * Set ADC Digital Volume
     */
#if 1
    ES8388_SetADCVOL(0, 1.0f);
#else
    ES8388_WriteReg(0x10, 0x00); // LADCVOL
    ES8388_WriteReg(0x11, 0x00); // RADCVOL
#endif

    /* UnMute ADC */
    ES8388_WriteReg(0x0F, 0x30); //

    ES8388_WriteReg(0x12, 0x16);

    ES8388_WriteReg(0x17, 0x18); // DACLRSWAP, DACLRP, DACWL, DACFORMAT
    ES8388_WriteReg(0x18, 0x02); // DACFsMode , DACFsRatio
#endif


    /*
     * Set ADC Digital Volume
     */
    ES8388_WriteReg(0x1A, 0x00);
    ES8388_WriteReg(0x1B, 0x02);
    //ES8388_WriteReg(0x1B, (1<<5) + 0x03); // ADCFsMode , ADCFsRatio Hasan
    //ES8388_WriteReg(0x1B, 0); // ADCFsMode , ADCFsRatio Hasan
    /* UnMute DAC */
    ES8388_WriteReg(0x19, 0x32);
#endif
    /*
     * Setup Mixer
     */
    ES8388_WriteReg(0x26, 0x09);//        ES8388_WriteReg(0x26, 0x00);
    ES8388_WriteReg(0x27, 0xD0); // ES8388_DACCONTROL17
    ES8388_WriteReg(0x28, 0x38);
    ES8388_WriteReg(0x29, 0x38);
    ES8388_WriteReg(0x2A, 0xD0);

    /* Set Lout/Rout Volume */
#if 1
    ES8388_SetOUT1VOL(0, 1);
    ES8388_SetOUT2VOL(0, 1);
#else
    ES8388_WriteReg(0x2E, 0x1E);
    ES8388_WriteReg(0x2F, 0x1E);
    ES8388_WriteReg(0x30, 0x1E);
    ES8388_WriteReg(0x31, 0x1E);
#endif

    /* Power up DEM and STM */
#if 0
    ES8388_WriteReg(0x02, 0xF0);
#else
    ES8388_WriteReg(0x02, 0x00);
#endif

    ES8388_SetInputCh(1, 1);
    ES8388_SetMixInCh(2, 1);
    ES8388_SetPGAGain(0, 1);
    ES8388_SetIn2OoutVOL(0, 0);

    Serial.printf("ES8388 setup finished!\n");
    es8388_read_all();
}