#include <Enrf24.h>
#include <nRF24L01.h>
#include <SPI.h>

#define P2_0 PA_5
#define P2_1 PA_6
#define P2_2 PA_7

typedef struct {
  byte red;
  byte green;
  byte blue;
} color_t;

Enrf24 radio(P2_0, P2_1, P2_2);
const uint8_t txaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };

color_t data;

void setup()
{
  Serial.begin(9600);
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(1);
  
  radio.begin(250000);
  radio.setChannel(92);
  radio.setCRC(1,1);
  
  dump_radio_status_to_serialport(radio.radioState());
  radio.setTXaddress((void*)txaddr);
  
  randomSeed(analogRead(1)); //A1
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  if(radio.radioState() != ENRF24_STATE_NOTPRESENT) {
    digitalWrite(GREEN_LED, HIGH);
  } else {
    analogWrite(GREEN_LED, LOW);
  }
}

void loop()
{
  dump_radio_status_to_serialport(radio.radioState());
  digitalWrite(RED_LED, HIGH);
  
  data.red = random(0, 256);
  data.green = random(0, 256);
  data.blue = random(0, 256);
  
  Serial.print("Red = ");
      Serial.println(data.red);
      Serial.print("Green = ");
      Serial.println(data.green);
      Serial.print("Blue = ");
      Serial.println(data.blue);
  radio.write((void*)&data, sizeof(color_t));
  radio.flush();
  
  delay(20);
  
  digitalWrite(RED_LED, LOW);
  
  delay(2000);
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
