#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

#define WIND 4
#define NET_ID 1
#define NODE_ID 4

int winval;

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload wind;

RF24 radio(9,10);

const uint64_t pipe = 0xDEADBEEF04;

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
  pinMode(WIND,INPUT);
}

void loop() {
  radio.powerUp();
  //delay(500);
  // put your main code here, to run repeatedly:
  winval = digitalRead(WIND);
  wind.from = NODE_ID;
  wind.to = 0;
  wind.type = 4;
  wind.data1 = winval;
  bool ok = false;
  while(!ok)
  ok = radio.write(&wind,sizeof(payload));
  //if(ok)
  printf("ok\n");
  //else
  //printf("failed\n");
  radio.powerDown();
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); 
  //LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  //delay(16000);
}
