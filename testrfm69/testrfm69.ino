
#include <imframe.h>
#include <imatmega.h>
//#include <avr/wdt.h>
#include <SPI.h>  
// Data wire is plugged into pin 2 on the Arduino







//#.define DBGLVL 2
#include "imdebug.h"





/******************************** Configuration *************************************/

#define MMAC 0x200001  // My MAC
#define ServerMAC 0xA000  // Server  MAC
#define MDEVICE 7     //Type of device






/************************* Module specyfic functions **********************/


#include "imtrans.h"
#include "imbuffer.h"
#include "imbufrfm69.h"
//#include "imbuftest.h"


IMBuffer    buf3;
Transceiver trx;
















void SendDataFlood()
{
     static IMFrame frame;
     frame.Reset();
     IMFrameData *data =frame.Data();
     for(byte i = 0; i < 1; i++){
        data->w[0]=100;
        data->w[1]=i;

        trx.SendData(frame);
        trx.Transmit();

     }
     
      ERRFLASH();
     
}


void SendData()
{
/*   if (trx.Connected())
   {
      if (trx.CycleData())
      {
        static IMFrame frame;
        frame.Reset();
        long mm=millis();
//        DataDS18B20(frame);
        DBGINFO(" :");
        DBGINFO(millis()-mm);
        

        DBGINFO("SendData ");
        trx.SendData(frame);
        trx.Transmit();
        ERRFLASH();
      } else{
         trx.printCycle();


      }

   } else {
     trx.ListenBroadcast();
   }

*/
}



void ReceiveData()
{
  static IMFrame rxFrame;
ERRLEDON();
  DBGINFO(" Receive ");
      if (trx.GetFrame(rxFrame))
      {
        if (!trx.ParseFrame(rxFrame))
        {
          DBGINFO(" rxGET ");
        }
      }
      DBGINFO("\r\n");

ERRLEDOFF();

}

void PrintStatus()
{
  DBGINFO("\r\n");
  DBGINFO(" Status ");
//  trx.printStatus();
  DBGINFO("\r\n");

}


void stageloop(byte stage)
{
//   if (stage== STARTBROADCAST){
    DBGINFO("stageloop=");  DBGINFO(millis());
    DBGINFO(":");  DBGINFO(stage);
//  }
  switch (stage)
  {
    case STARTBROADCAST:  trx.ListenBroadcast();    break;
    case STOPBROADCAST:  trx.Knock();      break;
//   case STARTDATA: SendDataFlood();  break;
//    case STOPDATA:   trx.StopListen();      break;
    case LISTENDATA : ReceiveData();break;
    case LISTENBROADCAST : ReceiveData();break;
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

   DBGINFO("@@\r\n");

}





void setup()
{
   
  INITDBG();
  DBGINFO("SETUP");
  DBGINFO(freeRam());
  DBGINFO("*");
  DBGINFO(sizeof(trx));
  DBGINFO("*");
//  DBGINFO(buf3._IM);
  
  DBGINFO("_");
 
  ERRLEDINIT();   ERRLEDOFF();
//  wdt_enable(WDTO_8S);

 // interrupts ();
//  randomSeed(analogRead(0)+internalrandom());

  trx.myMAC=MMAC;
      DBGINFO2(trx.myMAC,HEX);
  trx.Init(buf3);
 
 
    DBGINFO(" MMAC ");  DBGINFO2(trx.myMAC,HEX);
  //      DBGINFO("  ");
  trx.myDevice=MDEVICE;
  trx.timer.onStage=stageloop;
  trx.DisableWatchdog();
  
//  pciSetup(9);
    ERRLEDON();
 //   delay(1000);
    delay(200);
    ERRLEDOFF();
//  trx.TimerSetup();
//   DBGINFO("classtest Timer");  DBGINFO(IMTimer::ClassTest());
}

void loop()
{
//  wdt_reset();
//  PrintStatus();
//  delay(300);
   DBGINFO("\r\n");
  DBGINFO("LOOP");
    
  byte xstage;
  do{

     xstage=trx.timer.WaitStage();
     stageloop(xstage);
  }while( xstage!=IMTimer::PERIOD);


}
