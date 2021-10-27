#include <Arduino.h>

#ifdef __CODEC__
#define CODEC_EXTRN
#else
#define CODEC_EXTRN extern
#endif

#define Codec_ID_NOCODEC        0
#define Codec_ID_AC101          1
#define Codec_ID_ES8388         2

#define Codec_ES8388_ADDR       0x10
#define Codec_AC101_ADDR        0x1A

#define Codec_I2C_NOTDEFINE     99  // 
#define Codec_I2C_PORT0         0   // 33 and 32 pin
#define Codec_I2C_PORT1         1   // 18 and 23 pin

struct AudioKitCodec
{
    uint8_t Codec_Id;
    
    uint8_t i2c_port;
    uint8_t i2c_addr;
    uint8_t i2c_sda;
    uint8_t i2c_scl;

    uint8_t i2s_mclk;
    uint8_t i2s_blck;
    uint8_t i2s_wclk;
    uint8_t i2s_dout;
    uint8_t i2s_din;
};

CODEC_EXTRN AudioKitCodec   ESP32AudioCodec;
