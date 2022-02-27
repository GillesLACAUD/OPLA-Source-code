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
void Distortion(float* sol,float* sor)
{
int32_t l1,l2;
int32_t r1,r2;
uint8_t shift=0;        // Decrease volume if more decimator 
float ldec;             
float rdec;    
float div;
float panlrdec;

float tol,tor;          // Temp float left right

    tol=*sol;
    tor=*sor;

    div = (float)WS.Distortion/127;
    panlrdec = 2*WS.PanDecimator*NORM127MUL2;

    r1=(int16_t)(tol*32768.0f);
    l1=(int16_t)(tor*32768.0f);
    r2=r1;
    l2=l1;

    r1 *= WS.Distortion;
    l1 *= WS.Distortion;

    if(r1>32768)
       r1=32768;
    if(r1<-32768)
       r1=-32768;

    if(l1>32768)
       l1=32768;
    if(l1<-32768)
       l1=-32768;

    r1 /= WS.Distortion;
    l1 /= WS.Distortion;

    // Int decimator
    if(WS.Decimator >1)
    {
        if(WS.Decimator>11)
            shift = 2;
        else if(WS.Decimator>10)
            shift=2; 
        else if(WS.Decimator>8)
            shift=1; 
        
        r2 = (r2>>WS.Decimator)<<(WS.Decimator-shift); 
        l2 = (l2>>WS.Decimator)<<(WS.Decimator-shift); 

        //ldec= (float)l1*0.00003051758125f*(panlrdec);
        //rdec= (float)r1*0.00003051758125f*(2-panlrdec);

        //out_l= (1-wetdrydec)*(out_l) + (1*wetdrydec)*ldec;
        //out_r= (1-wetdrydec)*(out_r) + (1*wetdrydec)*rdec;
    }
    l1 +=l2,
    r1 +=r2,


    ldec= (float)l1*0.00003051758125f;
    rdec= (float)r1*0.00003051758125f;

    //out_l =out_l+ldec;
    //out_r =out_r+rdec;

    tol =(tol)*(1-div)+1*ldec*(div);
    tor =(tor)*(1-div)+1*rdec*(div);

    *sol=tol;
    *sor=tor;

}
