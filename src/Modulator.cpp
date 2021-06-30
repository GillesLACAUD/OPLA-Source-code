#include "Arduino.h"
#include "typdedef.h"

#define __MODULATOR__
#include "Modulator.h"
#include "easysynth.h"


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void IRAM_ATTR ModWheel_Process()
{
float temp;    

    temp= ModWheelValue;
    temp *= ModWheelAmount;
   
    switch(ui8_ModWheelDest)
    {
        case MOD_CUTOFF:
        FiltCutoffMod += temp;
        //Serial.printf("Mod:Send %3.2f\n",FiltCutoffMod);      
        break;
        case MOD_NOISE:
        NoiseMod +=temp;
        break;
        case MOD_PAN:
        PanMod +=temp-0.5;
        //Serial.printf("Mod:Send %3.2f\n",PanMod);      
        break;
        case MOD_LAMT1:
        Lfo1AmtMod +=temp-0.5;
        break;
        case MOD_LAMT2:
        Lfo2AmtMod +=temp-0.5;
        break;
        case MOD_LSPEED1:
        Lfo1SpeedMod +=(0.5-temp)*2;
        break;
        case MOD_LSPEED2:
        Lfo2SpeedMod +=(0.5-temp)*2;
        break;
        case MOD_WS1:
        WaveShapping1Mod +=temp-0.5;
        break;
        // Route the Mod wheel to another CC
        case MOD_CC:
        break;

    }
}

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void IRAM_ATTR AfterTouch_Process()
{
   
    switch(ui8_AfterTouchDest)
    {
    }
    
    
}