#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

#define PIR 4
#define NET_ID 1
#define NODE_ID 1

int pirval;

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload pir1;

RF24 radio(9,10);

const uint64_t pipe = 0xDEADBEEF01;

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
  // put your main code here, to run repeatedly:
  pirval = digitalRead(PIR);
  pir1.from = NODE_ID;
  pir1.to = 0;
  pir1.type = 2;
  pir1.data1 = pirval;
  
  bool ok = radio.write(&pir1,sizeof(payload));
  if(ok)
  printf("ok\n");
  else
  printf("failed\n");
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
}
