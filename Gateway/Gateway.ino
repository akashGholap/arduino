// Sample RFM69 receiver/gateway sketch, with ACK and optional encryption, and Automatic Transmission Control
// Passes through any wireless received messages to the serial port & responds to ACKs
// It also looks for an onboard FLASH chip, if present
// RFM69 library and sample code by Felix Rusu - http://LowPowerLab.com/contact
// Copyright Felix Rusu (2015)

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
//#include <RFM69_ATC.h>//get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>      //comes with Arduino IDE (www.arduino.cc)
//#include <SPIFlash.h> //get it here: https://www.github.com/lowpowerlab/spiflash

//*********************************************************************************************
//************ IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
#define NODEID        1    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
#define GATEWAYID   1
 
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY     RF69_433MHZ
#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW    //uncomment only for RFM69HW! Leave out if you have RFM69W!
//#define ENABLE_ATC    //comment out this line to disable AUTO TRANSMISSION CONTROL
//*********************************************************************************************

#define SERIAL_BAUD   57600

  #define LED           13 // Moteinos have LEDs on D9

#ifdef ENABLE_ATC
  RFM69_ATC radio;
#else
  RFM69 radio;
#endif


typedef struct {
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  float         temp;   //temperature maybe?
  int           len;
  byte          len2; 
  uint16_t      data[8];  
} Payload;
Payload theData;
Payload recData;
 

void SendPayload(){
      //fill in the struct with new values
    theData.nodeId = NODEID;
    theData.uptime = millis();
    theData.temp = 91.23; //it's hot!
    
    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ... ");
    if (radio.send(GATEWAYID, (const void*)(&theData), sizeof(theData)),false)
      Serial.print(" ok!");
    else Serial.print(" nothing...");
    Serial.println();
 
}

void ReceivePayload(){
  
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);

    memcpy(&recData,(void *)&radio.DATA,sizeof(Payload));
       
    Serial.print("   [RX_RSSI:");Serial.print(radio.readRSSI());
    Serial.print("]");
    Serial.print(recData.uptime);
    Serial.print(":");
    Serial.print(recData.temp);
    Serial.print("]");
    

    if (radio.ACKRequested())
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
      delay(10);
    }
} 

//SPIFlash flash(FLASH_SS, 0xEF30); //EF30 for 4mbit  Windbond chip (W25X40CL)
bool promiscuousMode = true; //set to 'true' to sniff all packets on the same network

void setup() {
  delay(2000);
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
//  radio.setHighPower(); //only for RFM69HW!
#endif
//  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  //radio.setFrequency(919000000); //set frequency to some custom frequency
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
//  pinMode(LED, OUTPUT);
//      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
//      byte temperature = 100;
//      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
//      Serial.print( "Radio Temp is ");
//      Serial.print(temperature);
//      Serial.print("C, \r\n");
 radio.readAllRegs(); 
    
}


void Blink(byte PIN, int DELAY_MS)
{
  digitalWrite(PIN,HIGH);
  delay(600);
  digitalWrite(PIN,LOW);
  delay(1000);
}

byte inloop =0;
byte ackCount=0;
uint32_t packetCount = 0;
void loop() {
  //process any serial input
    
//      byte temperature =  radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
//      byte temperature = 100;
//      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
//      Serial.print( "Radio Temp is ");
//      Serial.print(temperature);
//      Serial.print("C, \r\n");

  if (radio.receiveDone()){
     Serial.print( "Radio receive ");
     ReceivePayload();
  }  
  inloop++;
  if (inloop >10){
  SendPayload();
   inloop=0;
  }
 
//    Blink(LED,3);
  delay(1000);
  
}


