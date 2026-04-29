//EE.cpp
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
//Non-volatile
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
#include "EE.h"

#define INITVAL 0x5C

EEVars EE;

uint8_t *EE_START=(uint8_t *)&EE.initialized;

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
void init_EEVars(void){
  int ii=0;
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
    EEPROM_Read((uint8_t*)&EE.chanCfg, sizeof(EE.chanCfg));
    EEPROM_Read(&EE.gaugeMode, 1);
    if(EE.gaugeMode < GMODE_END) GaugeMode = EE.gaugeMode;
  }
  else{
    factoryReset();
  }
}