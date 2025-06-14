//#include <avr/io.h>
#include "GandL.h"
#include "IntrVFD.h"

#define HBEAM   23
#define RTURN   22
#define LTURN   21
#define ABS     19
#define AIRBAG  18
#define OIL     2
#define BKLIGHT 15
#define PB      0

uint32_t timerGaugeAndLights=0;
uint32_t weatherInfoUpdate=0;
uint32_t startupTest=1;
uint8_t removeTest[8];
typedef struct rxBuff{
  uint8_t data[10];
  uint8_t rxPtr;
}rxBuff;

rxBuff rx232;
boolean stringComplete=false;

uint8_t timerCheck(uint32_t tmr, uint32_t setpoint){
  uint32_t now=millis();
  if(now>(tmr+setpoint)){
    return true;
  }
  else
  {
    return false;
  }
}

void handle232(void);
double del = 100000;

void setup() { 
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
  rx232.rxPtr=0;
  Serial.println("Starting...");
  startupTest=millis()+3000;
  sendInfo(0, 700);
  sendInfo(1, 1200);
  sendInfo(2, 100);
  sendInfo(3, 100);
  sendInfo(4, 0xFFFF);
  printTextToVFD("888888",0,6,J_LEFT,vfd);
  sendVFD(vfd,1);
  
}

void loop() {
  if(stringComplete){
    stringComplete=false;
    handle232();
  }
  if(timerCheck(timerGaugeAndLights,50)){
    updateGuages_Lights();
    timerGaugeAndLights=millis();
  }
  if(timerCheck(weatherInfoUpdate,2000) && prepIt){
    weatherInfoUpdate=millis();
    vfdPrep();
    Serial.println("Zip");
    sendVFD(vfd,1);
  }
  if(timerCheck(startupTest,3000) && startupTest>0){
    startupTest=0;  //one time only
    sendInfo(0, 0);
    sendInfo(1, 0);
    sendInfo(2, 0);
    sendInfo(3, 0);
    sendInfo(4, 0x0);
    printTextToVFD("      ",0,6,J_LEFT,vfd);
    sendVFD(vfd,1);
  }
}

void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    
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

void handle232(){
  uint8_t dataLength=rx232.rxPtr;
  uint16_t multiplier=1, temp16=0;
  if(dataLength>1){
    switch(rx232.data[0]){
      case '0':
        dataLength-=1;   //Don't count the first byte identifier
        for(int ii=dataLength; ii>0; ii--){
            temp16+=(rx232.data[ii]-0x30)*multiplier;
            multiplier*=10;
        }
        Serial.println(temp16);  
        sendInfo(0, temp16);
        break;
     case '1':
        dataLength-=1;   //Don't count the first byte identifier
        for(int ii=dataLength; ii>0; ii--){
            temp16+=(rx232.data[ii]-0x30)*multiplier;
            multiplier*=10;
        }
        Serial.println(temp16);  
        sendInfo(1, temp16);
        break;
     case '2':
        dataLength-=1;   //Don't count the first byte identifier
        for(int ii=dataLength; ii>0; ii--){
            temp16+=(rx232.data[ii]-0x30)*multiplier;
            multiplier*=10;
        }
        Serial.println(temp16);  
        sendInfo(2, temp16);
        break;
     
    case '3':
        dataLength-=1;   //Don't count the first byte identifier
        for(int ii=dataLength; ii>0; ii--){
            temp16+=(rx232.data[ii]-0x30)*multiplier;
            multiplier*=10;
        }
        Serial.println(temp16);  
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
        Serial.println(temp16);  
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
        sendVFD(rx232.data,0);
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
        Serial.println(temp16);  
        sendInfo(1, temp16);
        printTextToVFD((char *)rx232.data,0,6,J_LEFT,vfd);
        sendVFD(rx232.data,1);
        break;

     case '8':
        dataLength-=1;   //Don't count the first byte identifier 
        printTextToVFD("L",0,2,J_LEFT,vfd);
        printNumToVFD(1013,2,4,1,J_RIGHT,vfd);
        sendVFD(rx232.data,1);
        break;

      case '9':
        prepIt=!prepIt;
        break;
      case 'A':
        sendInfo(5,232);
        break;
      case 'B':
        sendInfo(5,590);
        break;
      case 'v':
        Serial.println("V0.99");
        break;
      default:
        break;
       
      
    }
  }
  rx232.rxPtr=0;

}

