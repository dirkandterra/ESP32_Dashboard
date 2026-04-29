//EE.h
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
//Non-Volatile mem
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

#ifndef __EE_H
#define __EE_H

#include <EEPROM.h>
#include "secret.h"
#include "CANDash.h"

typedef struct{
  uint8_t initialized;
  char ssid[50];
  char pwd[30];
  ChanConfig_t chanCfg[NUM_CHANNELS];
  uint8_t gaugeMode;
}EEVars;
EEVars EE;

extern EEVars EE;
void init_EEVars(void);

#endif
