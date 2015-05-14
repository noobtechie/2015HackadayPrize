#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "LowPower.h"

#define RELAY_CH1 4
#define RELAY_CH2 5
#define NET_ID 1
#define NODE_ID 6

int relval;

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload rel2;

RF24 radio(9,10);

const uint64_t pipe = 0xDEADBEEF06;

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
  radio.openReadingPipe(1,pipe);
  //radio.startListening();
  radio.printDetails();
  pinMode(RELAY_CH1,OUTPUT);
  pinMode(RELAY_CH1,OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(radio.available())
  {
    bool done = false;
    while(!done)
      done = radio.read(&rel2,sizeof(payload));
  }
  digitalWrite(RELAY_CH1,rel2.data1);
  digitalWrite(RELAY_CH2,rel2.data2);
}
