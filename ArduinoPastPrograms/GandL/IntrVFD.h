//IntrVFD.h
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
//Routine to handle jacked up way Dodge decided to talk to vfd on
//the Intrepid Dashboard
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_

#ifndef __IntrVFD_H
#define __IntrVFD_H

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif
enum{J_LEFT,J_MID,J_RIGHT};
extern unsigned char vfd[8];
extern void populateVFD(void);
extern void smarterPopulateVFD(void);
extern void setVfdExtra(uint16_t symbol,uint8_t on);
extern void clearDisp(void);
extern uint8_t printNumToVFD(int32_t num,uint8_t start,uint8_t len,uint8_t dec,uint8_t justify,uint8_t *destArray);
extern void printTextToVFD(char *c,uint8_t start,uint8_t len,uint8_t justify,uint8_t *destArray);
int expand(char c);
extern uint8_t prepIt;

enum vfdExtras{
  vfdLow=0x02,
  vfd3=0x04,
  vfdD=0x08,
  vfdN=0x10,
  vfdR=0x20,
  vfdP=0x40,
  vfdODOff=0x80,
  vfdOD=0x200,
  vfdTracOff=0x400,
  vfdTrac=0x800,
  vfdD3L=0x1000,
  vfdPRN=0x2000,
  vfdTrip=0x4000,
  vfdDec=0x8000
};

#endif
