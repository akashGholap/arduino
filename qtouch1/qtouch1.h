// 
//    FILE:
// VERSION: 0.1.00
// PURPOSE: qtouch sensor  for imwave
//
//
// HISTORY:
//

#ifndef imQtouch_h
#define imQtouch_h

//#include "Arduino.h"
#include "imframe.h"
#include "imdebug.h"

uint16_t cpuVin;
uint16_t cpuTemp;
uint16_t cpuVinCycle=0;


const uint16_t ledFadeTable[32] = {0, 1, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 9, 10, 12, 15, 17, 21, 25, 30, 36, 43, 51, 61, 73, 87, 104, 125, 149, 178, 213, 255}; // this is an exponential series to model the perception of the LED brightness by the human eye

//int ref2,ref1;
int idx1,idx2,idx3,mm;
int shift1,shift2,shift3;
byte idxbool=0,IDXBOOL=0,idxmm;
#define TPIN1 1
#define TPIN2 2
#define TPIN3 3
#define LEDB1 6
#define LEDB2 1
#define LEDB3 7


imTouch touch;





void SetupQtouch()
{
  pinMode(LEDB2, OUTPUT);
 pinMode(LEDB3, OUTPUT);
// analogWrite(9, 100);
//  analogWrite(LEDB1, 100);
//  digitalWrite(7, 100);
 // if (funtest()>0){
  digitalWrite(LEDB2, HIGH);
  delay(300);
   digitalWrite(LEDB2, LOW);
  delay(200);
   digitalWrite(LEDB2, HIGH);
  delay(300);
   digitalWrite(LEDB2, LOW);
 // }
  //  ref1=ADCTouchRead(A1,30);
 //   ref2=ADCTouchRead(A0,30);

  touch.setup();
  IDXBOOL=0;
    shift1=touch.check(TPIN1);
   shift2=touch.check(TPIN2);
   shift3=touch.check(TPIN3);
   mm=9;
   idxmm=0;
  cpuTemp=2;
}


void computeShift(){
    shift1=(shift1*11+idx1)/12;
    shift2=shift2*0.95+idx2*0.05;
    shift3=shift3*0.95+idx3*0.05;
    idxmm=0; 
}
void LoopQtouch() {
 power_adc_enable(); // ADC converter
     touch.setup();
   idx1=touch.check(TPIN1);
   idx2=touch.check(TPIN2);
   idx3=touch.check(TPIN3);
   
  // calculate the index to the LED fading table
//  if(idx1>31) idx1= 31; // limit the index!!!
//  if(idx2>31) idx2= 31; // limit the index!!!
  //if(idx3>31) idx3= 31; // limit the index!!!
  bool ib1=(idx1-shift1)>mm;
  bool ib2=(idx2-shift2)>mm;
  bool ib3=(idx3-shift3)>mm;
  
  // fade the LED
//  analogWrite(9, ledFadeTable[idx3]);
  //analogWrite(7, ledFadeTable[idx]);
 //  analogWrite(LEDB1, ledFadeTable[idx1]);
   bool change=false;
   idxbool=0;
   if (ib1) idxbool+=1;
   if (ib2) idxbool+=2;
   if (ib3) idxbool+=4;
   ShutOffADC();
    power_adc_disable();

   if (idxbool!=IDXBOOL){
      IMTimer::doneMeasure();
      idxmm=0;
   digitalWrite(LEDB1, ib1);
   digitalWrite(LEDB2,ib2);
   digitalWrite(LEDB3,ib3);
   };
   if (idxbool==0){
     idxmm++;
     if (idxmm>32) computeShift();  
   } else{
    idxmm=0;
   } 
}


void PrepareQtouch()
{
 // IMTimer::doneMeasure();
//   sensors.requestTemperatures();
}  


void MeasureVCC(){
    SetupADC();
    cpuVin=internalVcc();
    cpuVin=internalVcc();
    cpuTemp=internalTemp();
    cpuTemp=internalTemp();
    ShutOffADC();
    power_adc_disable();
}
void DataQtouch(IMFrame &frame)
{   
  if (cpuVinCycle % 8==0){
      MeasureVCC();
  }
  
   cpuVinCycle++;
  
   IMFrameData *data =frame.Data();
 //      data->w[2]=hh;
   data->w[1]=cpuTemp;
   data->w[0]=cpuVin;
   data->w[2]=idxbool;
   data->w[3]=idx1;
   data->w[4]=idx2;
   data->w[5]=idx3;
   data->w[6]=(uint16_t)shift1;
   data->w[7]=shift2;
   data->w[8]=shift3;
   data->w[10]=0xA11A;
   IDXBOOL=idxbool;
}





#endif
//
// END OF FILE
//
