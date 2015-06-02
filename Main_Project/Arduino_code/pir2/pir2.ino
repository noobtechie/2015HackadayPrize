#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

#define PIR 4
#define NET_ID 1
#define NODE_ID 2

int pirval;

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload pir2;

RF24 radio(9,10);

const uint64_t pipe = 0xDEADBEEF02;

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
  pinMode(PIR,INPUT);
}

void loop() {
  radio.powerUp();
  // put your main code here, to run repeatedly:
  pirval = digitalRead(PIR);
  pir2.from = NODE_ID;
  pir2.to = 0;
  pir2.type = 2;
  pir2.data1 = pirval;
  
  bool ok = false;
  while(!ok)
  ok = radio.write(&pir2,sizeof(payload));
  //if(ok)
  printf("ok\n");
  //else
  //printf("failed\n");
  radio.powerDown();
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); 
  LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
}
