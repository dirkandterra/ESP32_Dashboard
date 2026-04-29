//IntrVFD.h
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
//Routine to handle jacked up way Dodge decided to talk to vfd on
//the Intrepid Dashboard
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

#ifndef __WIFI_MQTT_H
#define __WIFI_MQTT_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "secret.h"

typedef struct _tempData{
  uint16_t garageTemp;
  uint16_t currentTemp; // temp times 10
  uint16_t currentWind; //wind mph times 10
  uint8_t precipPercent;
  uint8_t forecast[10]; //tmax1,tmin1,tmax2......
}NodeRedData;

extern NodeRedData ServerData;

void setup_MQTT(void);
void setup_WIFI(void);
void checkMQTTConnection(uint32_t now);

#endif
