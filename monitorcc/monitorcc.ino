/*
        monitor CC1101 tool
        for more information: www.imwave

	Copyright (c) 2015 Dariusz Mazur
        
        v0.1 - 2015.09.01
	    Basic implementation - functions and comments are subject to change
        
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software
	and associated documentation files (the "Software"), to deal in the Software without restriction,
	including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
	and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
	LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

//#.define DBGLVL 2
#include "imdebug.h"


#include "imtrans.h"



/******************************** Configuration *************************************/

#define MID  0x00  //My ID
#define TID  0x00  //Default target ID



#define RADIO_BUFFSIZE 725  //Buffer for radio
#define RADIO_SENDTHRES 710  //Start writing when this full

#define RESEND_TIMEOUT 75  //Wait this time in ms for an radio ack

//Used for error signaling (ON when restransmitting, OFF on receive success)
#define ERRLEDON() digitalWrite(13,HIGH)
#define ERRLEDOFF() digitalWrite(13,LOW)
#define ERRLEDINIT() pinMode(13, OUTPUT)



/******************************* Module specific settings **********************/




  
  #define bridgeSpeed 9600
  #define bridgeBurst 10  //Chars written out per call
  #define radioDelay 3000  //Time between calls - Let hardware serial write out async


#define listenBroadcast 100
#define listenData 10000
#define channelBroadcast 0
#define channelData 0


#define INDEX_NWID 0  //Network ID
#define INDEX_SRC  1  //Source ID
#define INDEX_DEST 2  //Target ID
#define INDEX_SEQ  3  //Senders current sequence number


#define listenDelay 2000

/****************** Vars *******************************/

#define UART_BUFFSIZE 125  
//Buffer for UART
 

char radioBuf[RADIO_BUFFSIZE] = {0,};
unsigned short radioBufLen = 0;
//unsigned short radio_writeout = 0xFFFF;
unsigned long radioOut_delay = 0;
unsigned long listenTimeOut = 0;

char uartBuf[UART_BUFFSIZE] = {0,};
unsigned short uartBufLen = 0;
unsigned short seqnr = 0;
byte MonitorChannel =channelBroadcast;



IMCC1101  cc1101;
Transceiver trx;
volatile byte ReadState = 0;
volatile byte WriteState= 0;

/************************* Functions **********************/



void printRadio()
{
//    DBGINFO("Radio:");
//    DBGINFO(radioBufLen);
    Serial.write("\xfa\xfb");
    for (byte i=0 ; i<radioBufLen ;  i++ )
    {
      Serial.write(radioBuf[i]);
    }
    radioBufLen = 0;

    Serial.write("\r\n");

}

void ShiftUartBuffer(unsigned short x)
{
    DBGINFO("ShiftUart");
    DBGINFO(uartBufLen);
    DBGINFO(" ");
//    DBGINFO(x);

     uartBufLen -= x;
      for (unsigned short i=0 ; i<uartBufLen ; i++ )
      {
        uartBuf[i] = uartBuf[x+i];
      }
}



void pciSetup(byte pin)
{
    *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
    PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
    PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

/*
ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
 {
   if (trx.state==TransceiverRead)
   {
     ReadState++;
   } else {
     WriteState++;
   }

 }
 */

 bool CheckReadState()
 {
   if (ReadState>1){
//     DBGINFO("<");
//     DBGINFO(ReadState);
//     DBGINFO("+");
//     DBGINFO(WriteState);
//     DBGINFO(">");
      ReadState=0;

      return true;
   }
   return false;
 }

 bool CheckWriteState()
 {
   if (WriteState>0){
      WriteState=0;
      return true;
   }
   return false;
 }

 


void OnRead(byte state)
{
 DBGINFO(state);
  if (state==2)
  {
    return ;
  }
  ReadState++;
//  Timer.doneListen();      
}

//Initialize the system
void setup()
{
  INITDBG();
  interrupts ();
//  attachInterrupt(4, OnRead, FALLING );
  ERRLEDINIT(); ERRLEDOFF();
  trx.Init(cc1101);
  trx.myId= MID;
  trx.onEvent=OnRead;
 
//  pciSetup(7);
  pciSetup(9);
//  DBGINFO("classtest");  DBGINFO(Transceiver::ClassTest());
}

//Main loop is called over and over again



byte GetData()
{
//  static IMFrame rxFrame;
  if (trx.GetData()  )
  {
//      trx.printReceive();
 
//      if (trx.GetFrame(rxFrame))
//      {
        memcpy(radioBuf,&(trx.RX_buffer.packet),sizeof(IMFrame));
        
        radioBufLen=sizeof(IMFrame);
//        radioBufLen+=rxFrame.Get((uint8_t*)&(radioBuf[radioBufLen]));
        printRadio();
//      } else{
//        Serial.write("\xfa\xfbERRORGETFRAME");
//      }  

  }
  return 0;

}


void loop()
{
  static IMFrame frame;
  if (millis()>listenTimeOut) {
    MonitorChannel=channelData;
    listenTimeOut=millis()+10000;
    trx.setChannel(MonitorChannel);
    trx.StartReceive();


  }

//  trx.StartReceive();
  /************** radio to UART ************************/
//  listenTimeOut=millis()+listenDelay;
//   do{
     if (ReadState>0)
     {
        ERRLEDON();         delay(50);         ERRLEDOFF();
        ReadState=0;

        if (GetData()){
          //trx.StartReceive();
//          break;
          
        }
        trx.StartReceive();
     };
//  }while (listenTimeOut>millis());
//  DBGINFO(millis());

  // prepare data

   byte  cnt;
   byte crc=Serial.available();
   while ((uartBufLen<UART_BUFFSIZE) && crc)
   {
     cnt =Serial.read();
     crc--;
     uartBuf[uartBufLen++]=cnt;
     if (!crc)
     { 
       delay(1);
       crc=Serial.available();
     }

   }
  
   if (uartBufLen>=sizeof(IMFrame))
   {
      memcpy(&frame,&uartBuf,sizeof(IMFrame));
      ShiftUartBuffer(sizeof(IMFrame));
      if (frame.Knock())
      {
         ERRLEDON();         delay(20);         ERRLEDOFF();
//         delay(20);
//         ERRLEDON();         delay(20);         ERRLEDOFF();
           MonitorChannel=channelBroadcast;
          listenTimeOut=millis()+listenBroadcast;
          trx.setChannel(MonitorChannel);
      } ;
//          MonitorChannel=channelData;
//          listenTimeOut=millis()+listenData;
//      }
//      trx.setChannel(MonitorChannel);
      trx.TX_buffer.packet=frame;
      trx.TX_buffer.len=sizeof(IMFrame);

 
      trx.Send();
      DBGINFO("PUSH ");
//      trx.setChannel(MonitorChannel);
      trx.StartReceive();

   }
//   delay(100);
//   if (Serial.available()==0)
//      uartBufLen=0;

   
  DBGINFO(")\r\n");

}

