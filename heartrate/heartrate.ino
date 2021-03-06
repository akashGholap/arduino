#include <imframe.h>
#include <imatmega.h>
#include <SPI.h>
#include <EEPROM.h>
#include <Wire.h>
#include "heartRateLib.h"


#include "imdebug.h"
#include "imeprom.h"

/******************************** Configuration *************************************/

// Data wire is plugged into pin 2 on the Arduino

#define MMAC 0x240003  // My MAC
#define ServerMAC 0xA000  // Server  MAC
#define MDEVICE 24     //Type of device
#define MCHANNEL 2


/************************* Module specyfic functions **********************/


#include "imtrans.h"
#include "imtimer.h"
#include "imbufrfm69.h"

Transceiver trx;
IMBuffer    buffer;

#include "heartrate.h"

void PrepareData()
{
      if (trx.CycleData())
      {
   PrepareMAX30100();

      }
}  

void SendData()
{
      if (trx.CycleData()) {
     //   DBGPINHIGH();
        trx.Wakeup();
        static IMFrame frame;
        frame.Reset();
        DataMAX30100(frame);
    //    DBGPINLOW();
         trx.SendData(frame);
         trx.Transmit();
       }
 
}



void ReceiveData()
{
      while (trx.GetData())
      {
        if (trx.Parse())
        {
          DBGINFO(" rxGET ");
        }
      }
     DBGINFO("\r\n");
}



void stageloop(byte stage)
{
  switch (stage)
  {
    case STARTBROADCAST:  trx.Knock();     break;
    case STOPBROADCAST:  PrepareData();     break;
    case STARTDATA: SendData();  break;
    case STOPDATA:   trx.StopListen();      break;
    case LISTENDATA : ReceiveData();break;
    case LISTENBROADCAST : ReceiveData();break;
    case MEASUREDATA: MeasureMAX30100();break;
    case IMTimer::IDDLESTAGE : {

       DBGINFO("***IDDLE DATA");
       ReceiveData();break;
     }
    case IMTimer::PERIOD : 
  //      ERRFLASH();
  //     PrintStatus();
    break;
    default:
    break;
  }
}





void setup()
{
  resetPin();
  pinMode(10,OUTPUT);
  digitalWrite(10,HIGH);
  INITDBG();
  setupTimer2();
  power_timer0_enable();
  SetupADC();
  interrupts();
  delay(100);
  wdt_enable(WDTO_8S);
  SetupMAX30100();
 //  disableADCB();
  trx.startMAC=0;
  trx.myMAC=MMAC;
  trx.myChannel=MCHANNEL;
  
 
  trx.Init(buffer);
  trx.myDevice=MDEVICE;
 // power_timer0_disable();
  setupTimer2();
}

void loop()
{
  wdt_reset();
  byte xstage;
  do{
     xstage=trx.timer.WaitStage();
     stageloop(xstage);
  }while( xstage!=IMTimer::PERIOD);
}                                                                                    
