
#include "RS232Decode.h"
#include <string.h>

#define DELAYTIME 5	//in uS

char z,k;
char temp=0;
unsigned char datachar;
char DispString[20];
char bytecounter;
char dispcycle;
unsigned char gaugeString[8];
unsigned char lightString[2];
unsigned char vfdString[8];
//char vfdDimming=0xD0;
char vfdDimming=0x40;

void VFDData(uint8_t in);
void GLData(uint8_t in);

//******************
 
//******************

//-_-_-_-_-_-_-_-_ Send Dimming Info to VFD -_-_-_-_-_-_-
void sendVFDDimming()
{
  PORTB &= B11111011;
	VFDData(1);
	pulseLCDClk();			//send 11 first for dimming data
	VFDData(1);
	pulseLCDClk();

	VFDData(1);
	pulseLCDClk();			//not using last 2 bits of dimming data
	VFDData(1);
	pulseLCDClk();
	datachar=vfdDimming;
	for (k=0;k<8;k++)
	{

		temp = 0X01 & datachar;				//feeds in 8 bits, msb first and clocks
		VFDData(0);
		if (temp != 0)
		{
			VFDData(1);
		}
		datachar = datachar>>1;
		pulseLCDClk();
				
	}

	pulseVFDLoad();
}
//############### Send Info to VFD##################
void senddispToVFD()
{	
  PORTB &= B11111011;
	VFDData(1);
	pulseLCDClk();
	VFDData(0);				// send '10' first for digit data
	pulseLCDClk();
  Serial.println("************************");
	for (z=0;z<8;z++)							//there will be (8) 8 bit xfers
	{	
    Serial.println(vfdString[z],HEX);
		datachar=vfdString[z];								
		for (k=0;k<8;k++)
		{

			temp = 0X80 & datachar;				//feeds in 8 bits, msb first and clocks
			VFDData(0);
			if (temp != 0)
			{
				VFDData(1);
			}
			datachar = datachar<<1;
			pulseLCDClk();				
		}
	}
	pulseVFDLoad();


}

void pulseLCDClk()								//clock pulse duration and dashDelay
{		
	PORTB &= B11111101;
	dashDelay(DELAYTIME);
	PORTB |= B00000010;
	dashDelay(DELAYTIME);	
}

void pulseVFDLoad()
{
	PORTB |= B00000100;
	dashDelay(DELAYTIME);
	//PORTB &= B11111011;
}

void sendToGauges()
{
	PORTD |= B10000000;							//Chip select High
	for (z=0;z<8;z++)							//there will be (4) 10 bit xfers
	{	
		temp=gaugeString[z]&0x02;
		GLData(0);
		if (temp != 0)
		{
			GLData(1);
		}
		pulseGLClk();	
		temp=gaugeString[z]&0x01;
		GLData(0);
		if (temp != 0)
		{
			GLData(1);
		}
		pulseGLClk();
		z++;
		datachar=gaugeString[z];						
		for (k=0;k<8;k++)
		{
			temp = 0X80 & datachar;				//feeds in 8 bits, msb first and clocks
			GLData(0);
			if (temp != 0)
			{
				GLData(1);
			}
			datachar = datachar<<1;
			pulseGLClk();				
		}
	}
	PORTD &= B11101111;						//Ensure clock is low
	PORTD &= B01111111;						//Chip select low

}


void sendToLights()
{
		temp=lightString[0]&0x08;
		GLData(0);
		if (temp != 0)
		{
			GLData(1);
		}
		pulseGLClk();
		temp=lightString[0]&0x04;
		GLData(0);
		if (temp != 0)
		{
			GLData(1);
		}
		pulseGLClk();	
		temp=lightString[0]&0x02;
		GLData(0);
		if (temp != 0)
		{
			GLData(1);
		}
		pulseGLClk();	
		temp=lightString[0]&0x01;
		GLData(0);
		if (temp != 0)
		{
			GLData(1);
		}
		pulseGLClk();
		datachar=lightString[1];						
		for (k=0;k<8;k++)
		{
			temp = 0X80 & datachar;				//feeds in 8 bits, msb first and clocks
			GLData(0);
			if (temp != 0)
			{
				GLData(1);
			}
			datachar = datachar<<1;
			pulseGLClk();				
		}
	
	pulseLLatch();

}

void pulseGLClk()								//clock pulse duration and dashDelay
{	
	dashDelay(DELAYTIME);
	PORTD |= B00010000;							//clock high
	dashDelay(DELAYTIME);
	PORTD &= B11101111;							//clock low
}

void pulseLLatch()								//clock pulse duration and dashDelay
{	
	PORTD &= B10111111;	
	dashDelay(DELAYTIME);
	PORTD |= B01000000;
	dashDelay(DELAYTIME);
	PORTD &= B10111111;		
}

void dashDelay(char i)								//dashDelay routine
{
	delayMicroseconds(i);
}

void VFDData(uint8_t in){
	if(in)
		PORTB |= B00000001;
	else
		PORTB &= B11111110;
}
void GLData(uint8_t in){
	if(in)
		PORTD |= B00100000;
	else
		PORTD &= B11011111;
}

/*
  G   CS    D7
  L   Latch D6
  GL  Data  D5
  GL  Clk   D4
  VFD Load  D10
  VFD CLK   D9
  VFD Data  D8
*/