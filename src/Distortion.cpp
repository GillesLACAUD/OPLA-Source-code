#include "Arduino.h"
#include "typdedef.h"

#define __DISTO__
#include "Distortion.h"
#include "easysynth.h"

#define NORM127MUL2	0.0078740157

/***************************************************/
/*                                                 */
/*                                                 */
/*                                                 */
/***************************************************/
void IRAM_ATTR Distortion(float* sol,float* sor)
{
int32_t l1,r1;          // Wet signal
int32_t dl,dr;          // Disto
int32_t cl,cr;          // Decimator

uint8_t shift=0;        // Decrease volume if more decimator 
float ldec;             
float rdec;    

    if(WS.Distortion || WS.Decimator >1)
    {

        ldec=*sol;
        rdec=*sor;

        r1=(int16_t)(ldec*32768.0f);     // Dry signal
        l1=(int16_t)(rdec*32768.0f);     // Dry signal

        dl=l1;      // Disto signal    
        dr=r1;      // Disto signal

        cl=l1;      // Decimator signal
        cr=r1;      // Decimator signal

        if(WS.Distortion)
        {
            dr *= WS.Distortion;
            dl *= WS.Distortion;

            if(dr>32768)
            dr=32768;
            if(dr<-32768)
            dr=-32768;

            if(dl>32768)
            dl=32768;
            if(dl<-32768)
            dl=-32768;

            dl=(dl*2)/127;
            dr=(dr*2)/127;

            dl =(dl*(127-WS.PanDecimator))/64;  // Dist with pan
            dr =(dr*WS.PanDecimator)/64;        // Dist with pan
        }
        if(Decimator >1)
        {
            if(Decimator>11)
                shift = 2;
            else if(Decimator>10)
                shift=2; 
            else if(Decimator>8)
                shift=1; 

            cl = (cl>>Decimator)<<(Decimator-shift);             
            cr = (cr>>Decimator)<<(Decimator-shift); 

            cl =(cl*WS.PanDecimator)/32;            // Decim with pan
            cr =(cr*(127-WS.PanDecimator))/32;      // Decim with pan
        }

        l1 = (l1*(127-WS.WDDecimator))/127;
        l1 +=((dl+cl)*WS.WDDecimator)/127;

        r1 = (r1*(127-WS.WDDecimator))/127;
        r1 +=((dr+cr)*WS.WDDecimator)/127;

        ldec= (float)l1*0.00003051758125f;
        rdec= (float)r1*0.00003051758125f;

        *sol=ldec;
        *sor=rdec;
    }

}
