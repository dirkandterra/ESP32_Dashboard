#ifndef RS232Decode_H
#define RS232Decode_H

#include <inttypes.h>

#if ARDUINO >= 100
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc
#else
#include "WProgram.h"      // for delayMicroseconds
#include "pins_arduino.h"  // for digitalPinToBitMask, etc
#endif

#define L_LATCH   25
#define G_CHIPSEL 26
#define VFD_LOAD  12
#define HSPI_MOSI 13
#define HSPI_SCLK 14

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
  

	void sendToGauges(void);
	void sendToLights(void);
	void sendVFDDimming(void);
	extern unsigned char gaugeString[8];
	extern unsigned char lightString[2];
	extern char vfdString[8];




#endif


