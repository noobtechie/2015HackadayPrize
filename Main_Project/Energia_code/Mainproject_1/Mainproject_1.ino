#include <Enrf24.h>
#include <nRF24L01.h>
#include <string.h>
#include <SPI.h>
#include <WiFi.h>

// your network name also called SSID
char ssid[] = "sounak";
// your network password
char password[] = "12345678";

#define NET_ID 1
#define NODE_ID 0
#define REL1_ID 1
#define REL2_ID 2
#define RELAY 1
#define PIR 2
#define MAGSWITCH 3
#define OBSTACLE 4

Enrf24 radio(PA_5, PA_6, PA_7);

//Structure for data packets

typedef struct{
  uint8_t from;
  uint8_t to;
  uint8_t type;
  boolean data1;
  boolean data2;
}payload;

payload rel1;
payload rel2;

typedef struct{
  boolean pir1;
  boolean pir2;
  boolean door;
  boolean wind;
}inputs;

inputs packet;

boolean entry = false;

//Pipes for different nodes

const uint8_t pir1addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x01 };
const uint8_t pir2addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x02 };
const uint8_t dooraddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x03 };
const uint8_t windaddr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x04 };
const uint8_t rel1addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x05 };
const uint8_t rel2addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x06 };

// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "OIR8LLHNYCHMZ0CA";
String talkbackAPIKey = "TITL993H7V54V098";
String talkbackID = "784";
String modeCommand = "98931";
String R11Command = "98932";
String R12Command = "98937";
String R21Command = "98938";
String R22Command = "98939";
String timeCommand = "99648";
const int updateThingSpeakInterval = 16 * 1000;      // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)
long lastConnectionTime = 0;
int failedCounter = 0;

//RFID settings
const int rfidInterval = 8*1000;
long lastRfidAccess;
boolean accessState;
const int tagLen = 10;
const int idLen = 9;
const int kTags = 4;
int state;
int tagIndx = -1;

char knownTags[kTags][idLen] ;
char tempTags[kTags][idLen];
char addTag[idLen] = "07454582";
char delTag[idLen] = "07502485";
char newTag[idLen];

// Initialize Launchpad WiFi Client
WiFiClient client;

void setup()
{
  // Serial Port for Debugging
  
  Serial.begin(9600);
  Serial1.begin(9600);
  
  //Get Configurations
  getConfigSMS();
  
  //WiFi Setup
  
  startWiFi();
  
  //Radio Configuration
  
  radioconfig();
  
  pinMode(RED_LED,OUTPUT);
  digitalWrite(RED_LED,HIGH);
  delay(50);
  digitalWrite(RED_LED,LOW);
  delay(50);
  digitalWrite(RED_LED,HIGH);
  delay(50);
  digitalWrite(RED_LED,LOW);
  delay(50);
  
}
String response;
String mode;

void loop()
{
  Serial.println("Inside Loop");
  //radio.disableRX();
  //gettime();
  accessState = checkrfid();
  getdoor();
  getwind();
  getpir1();
  getpir2();
  if((millis() - lastConnectionTime > updateThingSpeakInterval)||(failedCounter>0)){
    failedCounter = 0;
    
    updateThingspeak();
    lastConnectionTime = millis();
    Serial.println("Updating Thingspeak");
  }
  mode = checktalkback(modeCommand);
  Serial.println("Mode: "+mode);
  if(mode.compareTo("MANUAL")==0)
  setManual();
  else if(mode.compareTo("AUTOMATIC")==0)
  setAutomatic();
  //control();
  //alertonintrusion();
  showStatus();
  
}

boolean checkrfid(){
  if((readTag()==0)&&((millis()-lastRfidAccess)>rfidInterval))
  return false;
  else{
    state = access();
    if(state==1){
    lastRfidAccess = millis();
    return true;
    }
  }
  if((millis()-lastRfidAccess)<rfidInterval)
  return true;
}

void getConfigSMS(){
  
}

void setAutomatic(){
  boolean lightmode;
  response = checktalkback(timeCommand);
  if(response.compareTo("OFF")==0)
  lightmode = false;
  else if(response.compareTo("ON")==0)
  lightmode = true;
  if(intrusion()){
    sendAlert();
    return;
  }
  else{
    if(packet.pir1==HIGH)
    setrel1(HIGH,lightmode);
    else
    setrel1(LOW,LOW);
    if(packet.pir2==HIGH)
    setrel2(HIGH,lightmode);
    else
    setrel2(LOW,LOW);
  }
}

boolean intrusion(){
  if(packet.wind == HIGH)
  return true;
  else if((checkrfid==false)&&(packet.door==true))
  return true;
  else
  return false;
}

void sendAlert(){
  sendMessage("There has been an Intrusion");
}

void sendMessage(String alertMessage){
  Serial.println("Alarm Raised");
}

void updateThingspeak(){
  String fieldData = "&field1="+String(packet.pir1,DEC)+"&field2="+String(packet.pir2,DEC)+"&field3="+String(packet.door,DEC)+"&field4="+String(packet.wind,DEC);
  if (client.connect(thingSpeakAddress, 80))
  {         
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(fieldData.length());
    client.print("\n\n");

    client.print(fieldData);
  }
  else
  failedCounter++;
}

void setManual(){
  boolean R1data1,R1data2,R2data1,R2data2;
  response = checktalkback(R11Command);
  if(response.compareTo("OFF")==0)
  R1data1 = false;
  else if(response.compareTo("ON")==0)
  R1data1 = true;
  response = checktalkback(R12Command);
  if(response.compareTo("OFF")==0)
  R1data2 = false;
  else if(response.compareTo("ON")==0)
  R1data2 = true;
  response = checktalkback(R21Command);
  if(response.compareTo("OFF")==0)
  R2data1 = false;
  else if(response.compareTo("ON")==0)
  R2data1 = true;
  response = checktalkback(R22Command);
  if(response.compareTo("OFF")==0)
  R2data2 = false;
  else if(response.compareTo("ON")==0)
  R2data2 = true;
  
  setrel1(R1data1,R1data2);
  setrel1(R2data1,R2data2);
}

String checktalkback(String commandID){
  String talkbackURL = "GET http://api.thingspeak.com/talkbacks/"+talkbackID+"/commands/"+commandID+"?api_key="+talkbackAPIKey;
  String talkBackCommand;
  char charIn;
  if (client.connect(thingSpeakAddress, 80)){
    Serial.println("Connected to thingspeak");
    client.print(talkbackURL); 
    
    }
    delay(100);
 while (client.available()) {
    charIn = char(client.read());
    talkBackCommand += charIn;
  }
  return talkBackCommand;
}

void showStatus(){
  Serial.print("PIR1:");
  Serial.println(packet.pir1);
  Serial.print("PIR2:");
  Serial.println(packet.pir2);
  Serial.print("DOOR:");
  Serial.println(packet.door);
  Serial.print("WINDOW:");
  Serial.println(packet.wind);
}

/*void alertonintrusion(){
 if(packet.wind==HIGH){
  digitalWrite(RED_LED,HIGH);
 } 
 if(packet.door==HIGH){
   digitalWrite(RED_LED,HIGH);
 }
}*/

void setrel1(boolean ch1,boolean ch2){
  payload rel1;
  radio.setTXaddress((void*)&rel1addr);
  rel1.from = NODE_ID;
  rel1.to = REL1_ID;
  rel1.type = RELAY;
  rel1.data1 = ch1;
  rel1.data2 = ch2;
  radio.write((void*)&rel1,sizeof(payload));
  radio.flush();
  dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
  //delay(1000);
}

void setrel2(uint8_t ch1,uint8_t ch2){
  payload rel2;
  radio.setTXaddress((void*)&rel2addr);
  rel1.from = NODE_ID;
  rel1.to = REL2_ID;
  rel1.type = RELAY;
  rel1.data1 = ch1;
  rel1.data2 = ch2;
  radio.write((void*)&rel2,sizeof(payload));
  radio.flush();
  dump_radio_status_to_serialport(radio.radioState());  // Should report IDLE
  //delay(1000);
}

void getpir1(){
  payload pir1;
  radio.setRXaddress((void*)&pir1addr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
  if (radio.read(&pir1,sizeof(payload)))
    Serial.println("Received packet: pir1");
  radio.disableRX();  
  radio.deepsleep();
  packet.pir1 = pir1.data1;
}

void getpir2(){
  payload pir2;
  radio.setRXaddress((void*)pir2addr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
  if (radio.read(&pir2,sizeof(payload)))
    Serial.println("Received packet: pir2");
  radio.disableRX();  
  radio.deepsleep();
  packet.pir2 = pir2.data1;
}

void radioconfig(){
  Serial.println("Inside radio config");
  SPI.begin();
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(1);
  
  radio.begin(250000);  //  250Kbps
  radio.setChannel(92);  //Channel 92
  radio.setCRC(1,1);  //16bit CRC
  //radio.setRXaddress((void*)windaddr);
  //radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState()); 
}

void getdoor(){
  payload door;
  radio.setRXaddress((void*)dooraddr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
  if (radio.read(&door,sizeof(payload)))
    Serial.println("Received packet: door");
  radio.disableRX(); 
  radio.deepsleep(); 
  packet.door = door.data1;
}

void getwind(){
  payload wind;
  radio.setRXaddress((void*)windaddr);
  radio.enableRX();
  dump_radio_status_to_serialport(radio.radioState());
  while (!radio.available(true))
    ;
    //Serial.println("Received");
  if (radio.read(&wind,sizeof(payload)))
    Serial.println("Received packet: wind");
  radio.disableRX();  
  radio.deepsleep();
  packet.wind = wind.data1;
}

void wificonfig(){
  Serial.println("Starting WiFi SmartConfig");
  WiFi.startSmartConfig();

  Serial.print("Connected to SSID: ");
  Serial.println(WiFi.SSID());

  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }

  Serial.println("\nIP Address obtained");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); 
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


void startWiFi()
{
  
  client.stop();

  Serial.println("Connecting Launchpad to network...");
  Serial.println();  

  delay(1000);
  
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }
  
  Serial.println("\nIP Address obtained");
  printWifiStatus();
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

int checkTag(char nTag[], char oTag[]){
  //Serial.print("Checking for: ");
  //Serial.print(oTag);
  for(int i=0;i<idLen;i++){
    if(nTag[i]!=oTag[i]){
      //Serial.println(" False");
      return 0;
    }
  }
  //Serial.println(" True");
  return 1;
}

int readTag(){
  for (int c=0 ; c<idLen ; c++){
    newTag[c] = 0;
  }
  int i = 0;
  int readByte;
  boolean tag = false;
  if(Serial1.available() == tagLen){
    tag = true;
  }
  
  if(tag == true){
    while(Serial1.available()){
      readByte = Serial1.read();
      if(readByte != 13 && readByte != 10){
        newTag[i] = readByte;
        i++;
      }
      if(readByte == 10){
        tag = false;
      }
    }
  }
  if(strlen(newTag)==0)
  return 0;
  else{
    Serial.print("Tag read: ");
    Serial.println(newTag);
    return 1;
  }
}

int access(){
  /*readTag();
  if(strlen(newTag)==0){
    return;
  }
  else{*/
  if(checkTag(newTag,addTag)>0){
    Serial.println("Swipe Card to give access");
    while(readTag()==0);
    for(int i=0;i<=tagIndx;i++)
    if(checkTag(newTag,knownTags[i])){
      Serial.println("Card already present");
      return 0;
    }
    ++tagIndx;
    Serial.println(tagIndx);
    for(int ind=0;ind<idLen;ind++)
    knownTags[tagIndx][ind]=newTag[ind];
    return 0;
  }
  else if(checkTag(newTag,delTag)>0){
    if(tagIndx==-1){
      Serial.println("No card saved");
      return 0;
    }
    Serial.println("Swipe Card to delete");
    while(readTag()==0);
    boolean match = false;
    int pos;
    for(int ct=0;ct<=tagIndx;ct++)
    if(checkTag(newTag,knownTags[ct])>0){
      match = true;
      pos = ct;
    }
    if(match == false){
      Serial.println("No card found");
      return 0;
    }
    else{
      for(int i=0;i<pos;i++)
      for(int j=0;j<idLen;j++)
      tempTags[i][j]=knownTags[i][j];
      for(int i=pos+1;i<tagIndx;i++)
      for(int j=0;j<idLen;j++)
      tempTags[i][j]=knownTags[i][j];
      --tagIndx;
      for(int i=0;i<tagIndx;i++)
      for(int j=0;j<idLen;j++)
      knownTags[i][j]=tempTags[i][j];
    }
    return 0;
  }
  else{
    //Serial.println(newTag);
    int total = 0;
    for(int ct=0 ; ct<kTags ; ct++){
      total += checkTag(newTag, knownTags[ct]);
    }
    if(total>0){
      Serial.println("Welcome");
      return 1;
    }
    else{
      Serial.println("access Denied");
      return 0;
    }
  }
}
