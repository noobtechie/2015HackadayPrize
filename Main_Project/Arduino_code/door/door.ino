#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

#define DOOR 4
#define NET_ID 1
#define NODE_ID 3

int doorval;

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload door;

RF24 radio(9,10);

const uint64_t pipe = 0xDEADBEEF03;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  printf_begin();
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
  pinMode(DOOR,INPUT);
  pinMode(3,OUTPUT);
}

void loop() {
  radio.powerUp();
  // put your main code here, to run repeatedly:
  doorval = digitalRead(DOOR);
  door.from = NODE_ID;
  door.to = 0;
  door.type = 3;
  door.data1 = doorval;
  digitalWrite(3,doorval);
  bool ok = false;
  while(!ok)
  ok = radio.write(&door,sizeof(payload));
  //if(ok)
  printf("ok\n");
  radio.powerDown();
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}
