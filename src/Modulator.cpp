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