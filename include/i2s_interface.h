#ifndef __I2S_INTER__
#define __I2S_INTER__

#include "Arduino.h"
#include "Global.h"
#include <driver/i2s.h>

/*
 * Define and connect your PINS to DAC here
 */
#define I2S_BCLK_PIN	27
#define I2S_WCLK_PIN	26
#define I2S_DOUT_PIN	25          // D IN AC101


const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number

bool i2s_write_sample_32ch2(uint64_t sample);
bool i2s_write_stereo_samples(float *fl_sample, float *fr_sample);
void setup_i2s();


#endif

