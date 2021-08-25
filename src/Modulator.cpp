#include "Arduino.h"
#include "typdedef.h"

#define __MODULATOR__
#include "Modulator.h"
#include "easysynth.h"

extern int Fct_Ch_Noise(int val);

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void ModWheel_Process()
{
float temp;    
    
    temp = ModWheelValue;
    temp *=ModWheelAmount;

    // Multiple if faster than a case ?????
    
    if(ui8_ModWheelDest==MOD_CUTOFF)
    {
        FiltCutoffMod += temp;
        //Serial.printf("Mod:Send %3.2f\n",FiltCutoffMod);      
        return;
    }
    
    
    if(ui8_ModWheelDest==MOD_NOISE)
    {
        NoiseMod +=temp;
        return;
    }
    
    
    if(ui8_ModWheelDest==MOD_PAN)
    {
        PanMod +=temp-0.5;
        //Serial.printf("Mod:Send %3.2f\n",PanMod);      
        return;
    }
        
    if(ui8_ModWheelDest==MOD_LAMT1)
    {
        Lfo1AmtMod +=temp;
        return;
    }
    
    // Route the Mod wheel to another CC
    if(ui8_ModWheelDest==MOD_CC)
    {
        return;
    }
    
    if(ui8_ModWheelDest==MOD_LSPEED1)
    {
        Lfo1SpeedMod +=(0.5-temp)*2;
        return;
    }

    if(ui8_ModWheelDest==MOD_WS1)
    {
        WaveShapping1Mod +=temp-0.5;
        return;
    }
    
    if(ui8_ModWheelDest==MOD_LAMT2)
    {
        Lfo2AmtMod +=temp;
        return;
    }
    
    if(ui8_ModWheelDest==MOD_LSPEED2)
    {
        Lfo2SpeedMod +=(0.5-temp)*2;
        return;
    }

}


/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void CaseModWheel_Process()
{
float temp;    
    
    temp = ModWheelValue*ModWheelAmount;
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
        Lfo1AmtMod +=temp;
        break;

        // Route the Mod wheel to another CC
        case MOD_CC:
        //Serial.printf("Mod:Send CC\n");      
        break;

        /*
        case MOD_LSPEED1:
        Lfo1SpeedMod +=(0.5-temp)*2;
        break;
        */
        

        /*        
        case MOD_WS1:
        WaveShapping1Mod +=temp-0.5;
        break;
        */
        

        
        case MOD_LAMT2:
        Lfo2AmtMod +=temp;
        break;

               
        /*
        case MOD_LSPEED2:
        Lfo2SpeedMod +=(0.5-temp)*2;
        break;
        */
        default:
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
float temp;    
    
    temp = AfterTouchValue;
    temp *=AfterTouchAmount;

    // Multiple if faster than a case ?????
    
    if(ui8_AfterTouchDest==MOD_CUTOFF)
    {
        FiltCutoffMod += temp;
        //Serial.printf("Mod:Send %3.2f\n",FiltCutoffMod);      
        return;
    }
    
    
    if(ui8_AfterTouchDest==MOD_NOISE)
    {
        NoiseMod +=temp;
        return;
    }
    
    
    if(ui8_AfterTouchDest==MOD_PAN)
    {
        PanMod +=temp-0.5;
        //Serial.printf("Mod:Send %3.2f\n",PanMod);      
        return;
    }
        
    if(ui8_AfterTouchDest==MOD_LAMT1)
    {
        Lfo1AmtMod +=temp;
        return;
    }
    
    // Route the Mod wheel to another CC
    if(ui8_AfterTouchDest==MOD_CC)
    {
        return;
    }
    
    if(ui8_AfterTouchDest==MOD_LSPEED1)
    {
        Lfo1SpeedMod +=(0.5-temp)*2;
        return;
    }

    if(ui8_AfterTouchDest==MOD_WS1)
    {
        WaveShapping1Mod +=temp-0.5;
        return;
    }
    
    if(ui8_AfterTouchDest==MOD_LAMT2)
    {
        Lfo2AmtMod +=temp;
        return;
    }
    
    if(ui8_AfterTouchDest==MOD_LSPEED2)
    {
        Lfo2SpeedMod +=(0.5-temp)*2;
        return;
    }   
}