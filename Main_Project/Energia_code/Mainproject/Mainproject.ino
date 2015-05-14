#include <Enrf24.h>
#include <nRF24L01.h>
#include <string.h>
#include <SPI.h>
#include <WiFi.h>

#define NET_ID 1
#define NODE_ID 0
#define REL1_ID 1
#define REL2_ID 2
#define RELAY 1
#define PIR 2
#define MAGSWITCH 3
#define OBSTACLE 4

Enrf24 radio(PA_5, PA_6, PA_7);

//Structure for data packets

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload rel1;
payload rel2;

typedef struct{
  boolean pir1;
  boolean pir2;
  boolean door;
  boolean wind;
}inputs;

inputs packet;

boolean entry = false;

//Pipes for different nodes

const uint8_t pir1addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
const uint8_t pir2addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x02 };
const uint8_t dooraddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x03 };
const uint8_t windaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x04 };
const uint8_t rel1addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x05 };
const uint8_t rel2addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x06 };

void setup()
{
  // Serial Port for Debugging
  
  Serial.begin(9600);
  
  //WiFi SmartConfig
  
  //wificonfig();
  
  //Radio Configuration
  
  radioconfig();
  
  pinMode(RED_LED,OUTPUT);
  
}

void loop()
{
  //radio.disableRX();
  //gettime();
  //checkrfid();
  //getpir1();
  //getpir2();
  //getdoor();
  getwind();
  //control();
  //alertonintrusion();
 if(packet.wind)
 Serial.println("Someone there");
  Serial.println("End of loop");
}

void control(){
 if(packet.pir1==HIGH){
   setrel1(HIGH,HIGH);
 }
 if(packet.pir2==HIGH){
   setrel2(HIGH,HIGH);
 }
}

void alertonintrusion(){
 if(packet.wind==HIGH){
  digitalWrite(RED_LED,HIGH);
 } 
 if(packet.door==HIGH){
   digitalWrite(RED_LED,HIGH);
 }
}

void setrel1(boolean ch1,boolean ch2){
  radio.setTXaddress((void*)&rel1addr);
  rel1.from = NODE_ID;
  rel1.to = REL1_ID;
  rel1.type = RELAY;
  rel1.data1 = ch1;
  rel1.data2 = ch2;
  radio.write((void*)&rel1,sizeof(payload));
  radio.flush();
  dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
  delay(1000);
}

void setrel2(uint8_t ch1,uint8_t ch2){
  radio.setTXaddress((void*)&rel2addr);
  rel1.from = NODE_ID;
  rel1.to = REL2_ID;
  rel1.type = RELAY;
  rel1.data1 = ch1;
  rel1.data2 = ch2;
  radio.write((void*)&rel2,sizeof(payload));
  radio.flush();
  dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
  delay(1000);
}

void getpir1(){
  payload pir1;
  radio.setRXaddress((void*)&pir1addr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
  if (radio.read(&pir1,sizeof(payload)))
    Serial.print("Received packet: pir1");
  radio.disableRX();  
  radio.deepsleep();
  packet.pir1 = pir1.data1;
}

void getpir2(){
  payload pir2;
  radio.setRXaddress((void*)pir2addr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
  if (radio.read(&pir2,sizeof(payload)))
    Serial.print("Received packet: pir2");
  radio.disableRX();  
  radio.deepsleep();
  packet.pir2 = pir2.data1;
}

void radioconfig(){
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(1);
  
  radio.begin(250000);  //  250Kbps
  radio.setChannel(92);  //Channel 92
  radio.setCRC(1,1);  //16bit CRC
  //radio.setRXaddress((void*)windaddr);
  //radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState()); 
}

void getdoor(){
  payload door;
  radio.setRXaddress((void*)dooraddr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
  if (radio.read(&door,sizeof(payload)))
    Serial.print("Received packet: door");
  radio.disableRX(); 
  radio.deepsleep(); 
  packet.door = door.data1;
}

void getwind(){
  payload wind;
  radio.setRXaddress((void*)windaddr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
    Serial.println("Received");
  if (radio.read(&wind,sizeof(payload)))
    Serial.println("Received packet: wind");
  radio.disableRX();  
  radio.deepsleep();
  packet.wind = wind.data1;
}

void wificonfig(){
  Serial.println("Starting WiFi SmartConfig");
  WiFi.startSmartConfig();

  Serial.print("Connected to SSID: ");
  Serial.println(WiFi.SSID());

  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); 
}

void dump_radio_status_to_serialport(uint8_t status)
{
  Serial.print("Enrf24 radio transceiver status: ");
  switch (status) {
    case ENRF24_STATE_NOTPRESENT:
      Serial.println("NO TRANSCEIVER PRESENT");
      break;

    case ENRF24_STATE_DEEPSLEEP:
      Serial.println("DEEP SLEEP <1uA power consumption");
      break;

    case ENRF24_STATE_IDLE:
      Serial.println("IDLE module powered up w/ oscillators running");
      break;

    case ENRF24_STATE_PTX:
      Serial.println("Actively Transmitting");
      break;

    case ENRF24_STATE_PRX:
      Serial.println("Receive Mode");
      break;

    default:
      Serial.println("UNKNOWN STATUS CODE");
  }
}
