
#include "RS232Decode.h"
#include <string.h>
#include <SPI.h>

#define DELAYTIME 5	//in uS

char z,k;
char temp=0;
unsigned char datachar;
uint16_t data16;
char DispString[20];
char bytecounter;
char dispcycle;
unsigned char gaugeString[8];
unsigned char lightString[2];
char vfdString[8];
//char vfdDimming=0xD0;
char vfdDimming=0x40;
SPISettings spiSettings(100000,MSBFIRST,SPI_MODE1);
void VFDData(uint8_t in);

//******************
 
//******************

void DecodeInit(){
  pinMode((uint8_t)L_LATCH,OUTPUT);
  pinMode((uint8_t)G_CHIPSEL,OUTPUT);
  pinMode((uint8_t)VFD_LOAD,OUTPUT);
  
  SPI.begin((uint8_t)HSPI_SCLK,-1, (uint8_t)HSPI_MOSI);
}


//-_-_-_-_-_-_-_-_ Send Dimming Info to VFD -_-_-_-_-_-_-
void sendVFDDimming()
{
	//gpio_set_level(VFD_LOAD,LOW);
  SPI.beginTransaction(spiSettings);
  data16=0x0F00+vfdDimming;
  SPI.transfer16(data16);
  SPI.endTransaction();
	pulseVFDLoad();
}
//############### Send Info to VFD##################
void senddispToVFD()
{
  gpio_set_level(VFD_LOAD,LOW);
  dashDelay(DELAYTIME);
  datachar = 2;
  SPI.beginTransaction(spiSettings);
  SPI.transfer(datachar);
  Serial.println("************************");
	for (z=0;z<8;z++)							//there will be (8) 8 bit xfers
	{	
    data16=0;
    //Serial.println(vfdString[z],HEX);
    data16 = (vfdString[z++]<<8);
    data16 +=vfdString[z];
		SPI.transfer16(data16);								
    //send byte
	}
  SPI.endTransaction();
	pulseVFDLoad();


}

void pulseVFDLoad()
{
  dashDelay(DELAYTIME);
	gpio_set_level(VFD_LOAD,HIGH);
	
}

void sendToGauges()
{
	gpio_set_level(G_CHIPSEL,HIGH);
  dashDelay(DELAYTIME);
	//pack these (4) 10 bit xfers into (2) 16 bit xfer and one 8
	data16 = ((gaugeString[0]&0x03)<<14)&0xC000;
  data16 += gaugeString[1]<<4;
  data16 +=(gaugeString[2]&0x03)<<2;
  data16 +=gaugeString[3]>>4;
  SPI.beginTransaction(spiSettings);
  SPI.transfer16(data16);

  data16 = (gaugeString[3]<<12)&0xFC00;
  data16 += (gaugeString[4]&0x3)<<10;
  data16 += gaugeString[5]<<2;
  data16 += gaugeString[6]&0x03;
  SPI.transfer16(data16);
  
  datachar = gaugeString[7];
  SPI.transfer(datachar);					
  SPI.endTransaction();	
	gpio_set_level(G_CHIPSEL,LOW);
}


void sendToLights()
{
  data16 = lightString[0]<<8;
  data16 += lightString[1];
  SPI.beginTransaction(spiSettings);
  SPI.transfer16(data16);
  SPI.endTransaction();	
	pulseLLatch();

}

void pulseLLatch()								//clock pulse duration and dashDelay
{	
	dashDelay(DELAYTIME);
	gpio_set_level(L_LATCH,1);
	dashDelay(DELAYTIME);
	gpio_set_level(L_LATCH,0);	
}

void dashDelay(char i)								//dashDelay routine
{
	delayMicroseconds(i);
}

/*
  GL  Clk   D4
  GL  Data  D5
  L   Latch D6
  G   CS    D7
  VFD Data  D8
  VFD CLK   D9
  VFD Load  D10
  
*/