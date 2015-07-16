//Header files

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

//Node ID definitions

#define DOOR 4
#define NET_ID 1
#define NODE_ID 3

int doorval;

//Structure to store the radio packets

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload door;

RF24 radio(9,10); //Radio initialization with CS and CE pins at arduino 9 and 10

const uint64_t pipe = 0xDEADBEEF03; //Pipe address of this node

void setup() {
  
  // Serial Initialization
  
  Serial.begin(57600);
  printf_begin();
  
  //Radio Initialization
  
  radio.begin();
  radio.setRetries(15,15);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(92);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);
  
  //Open Pipe for Writing
  
  radio.openWritingPipe(pipe);
  //radio.startListening();
  radio.printDetails();
  pinMode(DOOR,INPUT);
  pinMode(3,OUTPUT);
}

void loop() {
  
  //Power Up radio from sleep mode
  
  radio.powerUp();
  
  // Read Sensor Data
  
  doorval = digitalRead(DOOR);
  
  //Store data in structure
  
  door.from = NODE_ID;
  door.to = 0;
  door.type = 3;
  door.data1 = doorval;
  digitalWrite(3,doorval);
  bool ok = false;
  
  //Wait untill the data is sent
  
  while(!ok)
  ok = radio.write(&door,sizeof(payload));
  //if(ok)
  printf("ok\n");
  
  //Power Down the node for 1 second
  
  radio.powerDown();
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}
