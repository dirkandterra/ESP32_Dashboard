//#include <avr/io.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>
#include "secret.h"
#include "GandL.h"
#include "IntrVFD.h"

#define VER "1.00"
#define INITVAL 0x5C

#define HBEAM   23
#define RTURN   22
#define LTURN   21
#define ABS     19
#define AIRBAG  18
#define OIL     2
#define BKLIGHT 15
#define PB      0

#define PB_DELAY 200
#define PB_LOCKOUT 500  //event triggered, delay before reading again
#define PB_LONGPUSH 1500/PB_DELAY   //Milliseconds divided by delay between reads
#define PB_PUSHED  0   //PB val is 0 when held

uint32_t retryTimeMQTT=0;
uint32_t timerGaugeAndLights=0;
uint32_t displayInfoUpdate;
uint32_t vfdLockoutTimer=0;
uint8_t vfdLockout=0;

uint32_t startupTest=1;
uint8_t GaugeMode = 0;
uint8_t backlight = 0;
typedef struct _pb{
  uint8_t currentRead;
  uint8_t lastRead;
  uint8_t holdCount;
  uint8_t pbEvent;  //0 is clear, 1 is short press, 2 is long press
  uint32_t pbTimer;
}pb_T;

typedef struct rxBuff{
  uint8_t data[10];
  uint8_t rxPtr;
}rxBuff;
rxBuff rx232;

pb_T pb;
boolean stringComplete=false;

void handle232(void);
void clearAll(uint8_t includeBacklight);
double del = 100000;

enum{
  PRESS_NONE,
  PRESS_SHORT,
  PRESS_LONG
};

enum{
  GMODE_SERIAL,
  GMODE_WEATHER,
  GMODE_TSTAT,
  GMODE_CAN,
  GMODE_END
};

WiFiClient IntrepidDash;
PubSubClient client(IntrepidDash);
WiFiServer TelnetServer(8266);
//*WIFI STUFF*
// Replace with your network credentials
const char* ssidAP     = WIFI_SSID;
const char* passwordAP = WIFI_PWD;

typedef struct{
  uint8_t initialized;
  char ssid[50];
  char pwd[30];
}EEVars;
EEVars EE;

uint8_t *EE_START=(uint8_t *)&EE.initialized;
char IPAddr[17]="0.0.0.0";
IPAddress IP;
String SSIDName="";

void setup_wifi(){ 
  // Connect to Wi-Fi
  int storedWifiRetry=20;
  WiFi.begin(EE.ssid, EE.pwd);
  while ((WiFi.status() != WL_CONNECTED) && (storedWifiRetry>0)) {
    delay(500);
    Serial.print(".");
    storedWifiRetry--;
  }
  if(storedWifiRetry){
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      SSIDName=EE.ssid;
      IP = WiFi.localIP();
      Serial.println(IP);
  }
  else{
    Serial.println("WiFi Failed on " + String(EE.ssid) + "!");
    WiFi.softAP(ssidAP, passwordAP);
    IP = WiFi.softAPIP();
    SSIDName=ssidAP;
    Serial.print("AP IP address: ");
    Serial.println(IP);
  } 
}
void EEPROM_Read(uint8_t *data, uint8_t bytes){
    int ii;
    int addr=data-EE_START;
    for(ii=0; ii<bytes; ii++){
      data[ii]=EEPROM.read(ii+addr);
    } 
}
void EEPROM_Write(uint8_t *data, uint8_t bytes){
    int ii;
    int addr=data-EE_START;
    for(ii=0; ii<bytes; ii++){
       EEPROM.write((ii+addr),data[ii]);
       EEPROM.commit();
    }
    
}
void backupWifiVars(){
    int zz=0;
    uint8_t sizeOfString=0;
    sizeOfString=strlen(EE.ssid)+1;
    Serial.print("Size of SSID: ");
    Serial.println(sizeOfString);
    EEPROM_Write((uint8_t *)&EE.ssid,sizeOfString);    
    sizeOfString=strlen(EE.pwd)+1;
    EEPROM_Write((uint8_t *)&EE.pwd,sizeOfString);
}
void factoryReset(){
  int ii=0;
  EE.initialized=INITVAL;
  EEPROM_Write(&EE.initialized,1);
  strcpy(EE.ssid,WIFI_SSID);
  strcpy(EE.pwd,WIFI_PWD);
  Serial.println("Factory Reset");
  backupWifiVars();
  
}

void setup() {
  int ii=0;
  pb.lastRead = !PB_PUSHED;
  pinMode(HBEAM,OUTPUT);
  pinMode(RTURN,OUTPUT);
  pinMode(LTURN,OUTPUT);
  pinMode(AIRBAG,OUTPUT);
  pinMode(ABS,OUTPUT);
  pinMode(OIL,OUTPUT);
  pinMode(BKLIGHT,OUTPUT);
  pinMode(PB,INPUT);

  DecodeInit();

  Serial.begin(115200);
  EEPROM.begin(sizeof(EE));
  rx232.rxPtr=0;
  Serial.println("Starting...");
  startupTest=millis()+5000;
  sendInfo(0, 700);
  sendInfo(1, 1200);
  sendInfo(2, 100);
  sendInfo(3, 100);
  sendInfo(4, 0xFFFF);
  digitalWrite(HBEAM,1);
  digitalWrite(RTURN,1);
  digitalWrite(LTURN,1);
  digitalWrite(ABS,1);
  digitalWrite(AIRBAG,1);
  digitalWrite(OIL,1);
  digitalWrite(BKLIGHT,1);
  printTextToVFD("888888",0,6,J_LEFT,vfd);
  sendVFD(vfd);

//initialize eevars
  EEPROM_Read(&EE.initialized,1);
  Serial.print("Initialized: ");
  Serial.println(EE.initialized);
  if(EE.initialized==INITVAL){
    for(ii=0;ii<49;ii++){
        EEPROM_Read((uint8_t*)&EE.ssid[ii],1);
        if(EE.ssid[ii]==0){ii=50;}
    }
    
    for(ii=0;ii<29;ii++){
        EEPROM_Read((uint8_t*)&EE.pwd[ii],1);
        if(EE.pwd[ii]==0){ii=30;}
    }    
  }
  else{
    factoryReset();
  }

  setup_wifi();

  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callbackMQTT);
}
// ################ MQTT ROUTINES ##################

// This functions connects your ESP8266 to your MQTT broker
void checkMQTTConnection(uint32_t now) {
  // Try to reconnect every 20 sec if not connected or timed out
  if (!client.connected() || !client.loop()){
    if ((now>retryTimeMQTT) || retryTimeMQTT==0){
      retryTimeMQTT=now+20000;
      Serial.print("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect("IntrepidEsp32",MOSQUITTO_USR,MOSQUITTO_PWD)) {
        Serial.println("connected");
        // Subscribe or resubscribe to a topic
        // You can subscribe to more topics (to control more LEDs in this example)
        client.subscribe("intrESP32/forecast");
        client.subscribe("esp32/GarageTemp");
      } else {
        Serial.print("failed, rc=");
        Serial.print(client.state());
        Serial.println(" try again in 20 seconds");
      }
    }
  }
}
// ################ MQTT ROUTINES ##################
// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// check the topic and handle the message
void callbackMQTT(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  char *token;
  char data[100];
  uint8_t *ptr8;
  int i=0;
  
  for (i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if(topic=="esp32/GarageTemp"){
    ServerData.garageTemp=(uint16_t)round(messageTemp.toFloat());
    Serial.println(ServerData.currentTemp);
  }
  if(topic=="intrESP32/forecast"){
    messageTemp.toCharArray(data, messageTemp.length()+1);
    token = strtok(data, ","); // Get the first token
    if (token != NULL) {
      ServerData.currentTemp = (uint16_t)round(atof(token)*10); // Convert the token to a float
    }
    token = strtok(NULL, ","); // Get the next
    if (token != NULL) {
      ServerData.currentWind = (uint16_t)round(atof(token)); // Convert the token to a float
    }
    ptr8=ServerData.forecast;
    for(i=0;i<10;i++){
      token = strtok(NULL, ","); // Get the next
      if (token != NULL) {
        *ptr8 = (uint8_t)round(atof(token)); // Convert the token to a float
      }
      ptr8++;
    }
    token = strtok(NULL, ","); // Get the next
    if (token != NULL) {
      ServerData.precipPercent = (uint8_t)round(atof(token)); // Convert the token to a float
    }

  }
}
void loop() {
  uint32_t loopMillis = millis();
  checkMQTTConnection(loopMillis);

  if(vfdLockoutTimer<loopMillis){
    vfdLockout=false;
  }
  if(loopMillis>pb.pbTimer){
    //time to read pb
    pb.pbTimer=loopMillis+PB_DELAY;
    pb.currentRead = digitalRead(PB);
    if(pb.currentRead == !PB_PUSHED){
      //button is up
      if(pb.currentRead != pb.lastRead){
        //pb has been released
        pb.holdCount=0;
        pb.pbTimer=loopMillis+PB_LOCKOUT;
        pb.pbEvent=PRESS_SHORT;
        Serial.println("Short");
      }  
    }else{
      //button is down
      pb.holdCount++;
      if(pb.holdCount>=PB_LONGPUSH){
        //Longpress event
        pb.holdCount=0;
        pb.currentRead = !PB_PUSHED;
        pb.pbTimer=loopMillis+PB_LOCKOUT;
        pb.pbEvent=PRESS_LONG;
        Serial.println("Long");
      }
    }
    pb.lastRead=pb.currentRead;
    if(pb.pbEvent==PRESS_LONG){
      backlight=!backlight;
      digitalWrite(BKLIGHT,backlight);
    }else if(pb.pbEvent==PRESS_SHORT){
      GaugeMode++;
      if(GaugeMode>=GMODE_END){
        GaugeMode=0;
      }
      clearAll(0);
      switch(GaugeMode){
        default:
        case GMODE_SERIAL:
          printTextToVFD("SERIAL",0,6,J_LEFT,vfd);
          break;
        case GMODE_WEATHER:
          printTextToVFD("WEATHR",0,6,J_LEFT,vfd);
          break;
        case GMODE_TSTAT:
          printTextToVFD("TSTAT ",0,6,J_LEFT,vfd);
          break;
        case GMODE_CAN:
          printTextToVFD("CAN   ",0,6,J_LEFT,vfd);
          break;
      }
      sendVFD(vfd);
      vfdLockoutTimer=loopMillis+3000;  //Let the display show the mode for 3 seconds before changing
      vfdLockout=true;
      Serial.println(GaugeMode);
      
    }
    pb.pbEvent=PRESS_NONE;
  }
  if(loopMillis>displayInfoUpdate){
    switch(GaugeMode){
      case GMODE_SERIAL:
      default:
        if(stringComplete){
          stringComplete=false;
          handle232();
        }
        displayInfoUpdate=loopMillis+200;
        break;
      case GMODE_WEATHER:
        vfdWeatherPrep();
        if(!vfdLockout){
          sendVFD(vfd);
        }
        displayInfoUpdate=loopMillis+2000;
        break;
      case GMODE_TSTAT:
        sendInfo(1, ServerData.garageTemp*10);
        displayInfoUpdate=loopMillis+2000;
        break;
      case GMODE_CAN:
        //todo
        break;
    }    
  }
  if(loopMillis>timerGaugeAndLights){
    timerGaugeAndLights=loopMillis+50;
    updateGuages_Lights();
  }
  //#################only runs once after bootup###############
  if((loopMillis>startupTest) && startupTest>0){
    startupTest=0;  //one time only
    clearAll(1);
    printTextToVFD("SERIAL",0,6,J_LEFT,vfd);
    sendVFD(vfd);
    vfdLockoutTimer=loopMillis+3000;  //Let the display show the mode for 3 seconds before changing
    vfdLockout=true;
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    if(GaugeMode==GMODE_SERIAL){
      if (inChar == '\r') {
        stringComplete = true;
      }
      else if(inChar=='\n'){
        
      }      
      else{
        rx232.data[rx232.rxPtr] = inChar;
        rx232.rxPtr++;
      }
    }
  }
}

void handle232(){
  uint8_t dataLength=rx232.rxPtr;
  uint16_t multiplier=1, temp16=0;
  if(dataLength==1){
    rx232.data[1]='0';
  }
  switch(rx232.data[0]){
    case '0':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      //Serial.println(temp16);  
      sendInfo(0, temp16);
      break;
    case '1':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      //Serial.println(temp16);  
      sendInfo(1, temp16);
      break;
    case '2':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      //Serial.println(temp16);  
      sendInfo(2, temp16);
      break;
    
  case '3':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      //Serial.println(temp16);  
      sendInfo(3, temp16);
      digitalWrite(HBEAM,0);
      digitalWrite(RTURN,0);
      digitalWrite(LTURN,1);
      digitalWrite(ABS,0);
      digitalWrite(AIRBAG,1);
      digitalWrite(OIL,0);
      digitalWrite(BKLIGHT,1);
      break;      
    case '4':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      //Serial.println(temp16);  
      sendInfo(4, temp16);
      digitalWrite(HBEAM,1);
      digitalWrite(RTURN,1);
      digitalWrite(LTURN,0);
      digitalWrite(ABS,1);
      digitalWrite(AIRBAG,0);
      digitalWrite(OIL,1);
      digitalWrite(BKLIGHT,0);
      break;
    case '5':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      testVFD(temp16);
      break;
    case '6':
      dataLength-=1;   //Don't count the first byte identifier 
      if(!vfdLockout){
        sendVFD(rx232.data);
      }
      break;

    case '7':
      dataLength-=1;   //Don't count the first byte identifier
      for(int ii=dataLength; ii>0; ii--){
          temp16+=(rx232.data[ii]-0x30)*multiplier;
          multiplier*=10;
      }
      rx232.data[8]|=0x80;
      rx232.data[5]=rx232.data[4];
      rx232.data[4]=rx232.data[3];
      rx232.data[3]=rx232.data[2];
      rx232.data[2]=rx232.data[1];
      rx232.data[0]=' ';  
      rx232.data[1]=' ';  
      //Serial.println(temp16);  
      sendInfo(1, temp16);
      printTextToVFD((char *)rx232.data,0,6,J_LEFT,vfd);
      if(!vfdLockout){
        sendVFD(rx232.data);
      }
      break;

    case '8':
      dataLength-=1;   //Don't count the first byte identifier 
      printTextToVFD("L",0,2,J_LEFT,vfd);
      printNumToVFD(1013,2,4,1,J_RIGHT,vfd);
      if(!vfdLockout){
        sendVFD(rx232.data);
      }
      break;
    case 'A':
      sendInfo(5,232);
      break;
    case 'B':
      sendInfo(5,590);
      break;
    case 'Z':
      clearAll(1);
      break;
    case 'v':
    case 'V':
      Serial.println(VER);
      break;
    default:
      break;
      
    
  }
  
  rx232.rxPtr=0;

}

void clearAll(uint8_t includeBacklight){
  sendInfo(0, 0);
  sendInfo(1, 0);
  sendInfo(2, 0);
  sendInfo(3, 0);
  sendInfo(4, 0x0);
  digitalWrite(HBEAM,0);
  digitalWrite(RTURN,0);
  digitalWrite(LTURN,0);
  digitalWrite(ABS,0);
  digitalWrite(AIRBAG,0);
  digitalWrite(OIL,0);
  if(includeBacklight){
    digitalWrite(BKLIGHT,0);
  }
  printTextToVFD("      ",0,6,J_LEFT,vfd);
  sendVFD(vfd);
}
