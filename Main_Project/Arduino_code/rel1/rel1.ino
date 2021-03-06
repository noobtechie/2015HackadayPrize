//Include files

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

//defines for the network

#define RELAY_CH1 4
#define RELAY_CH2 5
#define NET_ID 1
#define NODE_ID 5

int relval;

//Structure to packetize sensor data

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload rel1;

RF24 radio(9,10); //Initialize radio class

const uint64_t pipe = 0xDEADBEEF05; //Pipe address for this node

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  printf_begin();
  
  //Set up radio for reception
  
  radio.begin();
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(92);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);
  radio.openReadingPipe(1,pipe);
  radio.startListening();
  radio.printDetails();
  pinMode(RELAY_CH1,OUTPUT);
  pinMode(RELAY_CH2,OUTPUT);
}

void loop() {
  // Wait till data is available
  
  if(radio.available())
  {
    bool done = false;
    while(!done)
      done = radio.read(&rel1,sizeof(payload));
  }
  
  //Write data to relays
  
  digitalWrite(RELAY_CH1,rel1.data1);
  digitalWrite(RELAY_CH2,rel1.data2);
}
