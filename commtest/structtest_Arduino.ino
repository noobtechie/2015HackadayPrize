#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

typedef struct{
  byte red;
  byte green;
  byte blue;
} color_t;

color_t data;
//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipe = 0xDEADBEEF01 ;

void setup(){
  Serial.begin(57600);
  printf_begin();
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(92);
  radio.enableDynamicPayloads();
  radio.setCRCLength(RF24_CRC_16);
  radio.openReadingPipe(1,pipe);
  radio.startListening();
  radio.printDetails();
}

void loop(void){
  //Serial.println("In loop");
  if(radio.available())
  {
    Serial.println("Radio available");
    bool done;
    done = radio.read(&data,sizeof(color_t));
    if(done){
      Serial.print("Red = ");
      Serial.println(data.red);
      Serial.print("Green = ");
      Serial.println(data.green);
      Serial.print("Blue = ");
      Serial.println(data.blue);
    }
    else{
      Serial.println("Fail");
    }
  }
}
