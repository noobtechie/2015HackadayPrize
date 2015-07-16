//Include files

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

//Defines for network

#define PIR 4
#define NET_ID 1
#define NODE_ID 2

int pirval;

//Structure to packetize data

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload pir2;

RF24 radio(9,10); //Initialize radio class

const uint64_t pipe = 0xDEADBEEF02; //Pipe address for this node

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  printf_begin();
  
  //Radio settings
  
  radio.begin();
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(92);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);
  radio.openWritingPipe(pipe);
  //radio.startListening();
  radio.printDetails();
  pinMode(PIR,INPUT);
}

void loop() {
  
  //Power up radio from sleep
  
  radio.powerUp();
  // Save sensor data in structure
  
  pirval = digitalRead(PIR);
  pir2.from = NODE_ID;
  pir2.to = 0;
  pir2.type = 2;
  pir2.data1 = pirval;
  
  //Wait till data is transmitted
  
  bool ok = false;
  while(!ok)
  ok = radio.write(&pir2,sizeof(payload));
  //if(ok)
  printf("ok\n");
  //else
  //printf("failed\n");
  //Power down radio and node
  
  radio.powerDown();
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}
