#ifndef RS232Decode_H
#define RS232Decode_H

#include <inttypes.h>

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif

#define L_LATCH   GPIO_NUM_25
#define G_CHIPSEL GPIO_NUM_26
#define VFD_LOAD  GPIO_NUM_12
#define HSPI_MOSI GPIO_NUM_13
#define HSPI_SCLK GPIO_NUM_14

	void senddispToVFD(void);
	char storedispdata(char);
	char toprow(char);
	char botrow(char);
	char iconupdate(char);
	void pulseLCDClk(void);
	void pulseVFDLoad(void);
	void pulseLLatch(void);
	void pulseGLClk(void);
	void dashDelay(char);
  void DecodeInit(void);
  

	void sendToGauges(void);
	void sendToLights(void);
	void sendVFDDimming(void);
	extern unsigned char gaugeString[8];
	extern unsigned char lightString[2];
	extern char vfdString[8];




#endif


