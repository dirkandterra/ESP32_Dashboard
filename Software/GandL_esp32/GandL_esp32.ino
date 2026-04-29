//#include <avr/io.h>
#include "WIFI_MQTT.h"
#include "EE.h"
#include "GandL.h"
#include "IntrVFD.h"

#define VER "1.00"

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
  init_EEVars();
  setup_WIFI();
  setup_MQTT();
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
