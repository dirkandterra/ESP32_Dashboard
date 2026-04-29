//EE.h
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
//Non-Volatile mem
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

#ifndef __EE_H
#define __EE_H

#include <EEPROM.h>
#include "secret.h"

typedef struct{
  uint8_t initialized;
  char ssid[50];
  char pwd[30];
}EEVars;

extern EEVars EE;
void init_EEVars(void);

#endif
