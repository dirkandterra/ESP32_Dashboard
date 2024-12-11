//Guage and Lights Only
#include "RS232Decode.h"
#include "IntrVFD.h"

int gaugeData[25]={0,34,65,96,125,155,181,204,232,256,279,303,329,352,382,410,438,466,497,528,559,590,621,649,677};
int gaugeData2[25]={0,23,44,62,83,102,120,138,156,176,196};
char CBuffLen=0;
char t0_int;// = 0x30;
int t0_cyc=0;
char t0_disp;
char segrotate=1;
char digsel=2;
int rpmData=261;
int mphData=261;
int gasData=38;
int tempData=38;
int lightData=0;
char forecast[8]={0x64,0x3F,0x63,0x3E,0x62,0x3D,0x61,0x3C};
char mode=0;
int oldMPH=261;
int oldRPM=261;
uint8_t dectrip=0;
uint8_t vfd_count=0;
//261,5.25,22,41.1,61.8,80,96.5,113.5,936
int gypsyMath(int in);
int gypsyMath2(int in);
void vfdPrep(void);


int sendInfo(uint8_t gauge, uint16_t value){
	
	switch(gauge){
		
		case 0:
			rpmData=(int)(double(value)*12/7);
      rpmData=gypsyMath(rpmData);
			break;
	
		case 1:
			mphData=(int)value;
			mphData=gypsyMath(mphData);
			break;
		
		case 2:
			gasData=(int)value;
      gasData=gypsyMath2(gasData);
			break;
			
		case 3:
			tempData=(int)value;
      tempData=gypsyMath2(tempData);
			break;

    case 5:
      mphData=value;
      break;

		default:
		case 4:
			lightData=(int)value;
			break;
	}

}

//****************************************************
void updateGuages_Lights(){
  int diff;

  diff=rpmData-oldRPM;
  if (diff<100 && diff>-100) {oldRPM=rpmData;}
  else {oldRPM=oldRPM+diff/6;}

  diff=mphData-oldMPH;
  if (diff<100 && diff>-100) {oldMPH=mphData;}
  else {oldMPH=oldMPH+diff/6;}


  gaugeString[0]=oldMPH/256;
  gaugeString[1]=oldMPH%256;
  gaugeString[2]=oldRPM/256;
  gaugeString[3]=oldRPM%256;
  gaugeString[4]=tempData/256;
  gaugeString[5]=tempData%256;
  gaugeString[6]=gasData/256;
  gaugeString[7]=gasData%256;
  sendToGauges();

  lightString[0]=lightData/256;
  lightString[1]=lightData%256;
  sendToLights();
}
//####################################################
int sendVFD(uint8_t *c, uint8_t n){
	clearDisp();
    /*vfd[6]=c[7];
    vfd[7]=c[8];
    vfd[5]=c[1];
    vfd[4]=c[2];
    vfd[3]=c[3];
    vfd[2]=c[4];
    vfd[1]=c[5];
    vfd[0]=c[6];*/
    if(n){
	    smarterPopulateVFD();
    }else{
      populateVFD();
    }
	sendVFDDimming();
	senddispToVFD();
	vfd_count++;
	if(vfd_count>7)
		vfd_count=0;
}

//####################################################
int testVFD(uint16_t value){
	uint8_t carry=0, oldcarry=0;
	//clearDisp();
	
	if(vfdString[6]&0x80 || value)
		oldcarry=1;
	
	for(int ii=0; ii<8; ii++){
		carry=0;
		if(vfdString[ii]&0x80){
			carry=1;
		}
		vfdString[ii]<<=1;
		if(oldcarry)
			vfdString[ii]|=0x01;
		else
			vfdString[ii]&=0xFE;
		
		oldcarry=carry;
	}

	sendVFDDimming();
	senddispToVFD();

}

//####################################################
void vfdPrep()
{

	sendInfo(2,vfd_count*10);
	printNumToVFD(forecast[vfd_count],2,4,0,J_RIGHT,vfd);

	if(vfd_count%2==0)
	{
		 printTextToVFD("HI",0,2,J_LEFT,vfd);
	}

	else
	{
    printTextToVFD("LO",0,2,J_LEFT,vfd);
	}

		// Set Trans select to show day and light up "PRN D3L"
	if (vfd_count<2)
	{
		setVfdExtra(vfdPRN,1);
    setVfdExtra(vfdD3L,1);
    setVfdExtra(vfdD,0);
		setVfdExtra(vfdP,1);
	}
		else if (vfd_count<4)
	{
		setVfdExtra(vfdPRN,1);
    setVfdExtra(vfdD3L,1);
		setVfdExtra(vfdP,0);
    setVfdExtra(vfdR,1);
	}
	else if (vfd_count<6)
	{
		setVfdExtra(vfdPRN,1);
    setVfdExtra(vfdD3L,1);
		setVfdExtra(vfdR,0);
    setVfdExtra(vfdN,1);
	}
	else
	{
		setVfdExtra(vfdPRN,1);
    setVfdExtra(vfdD3L,1);
		setVfdExtra(vfdN,0);
    setVfdExtra(vfdD,1);
	}	
      Serial.print(vfd[5],HEX);
      Serial.print(vfd[4],HEX);
      Serial.print(vfd[3],HEX);
      Serial.print(vfd[2],HEX);
      Serial.print(vfd[1],HEX);
      Serial.println(vfd[0],HEX);	

}

int gypsyMath(int in)
{
if (in>=1200)
	{
		return 937;	//676+261
	}
	else
	{
		int i=in/50;
		i=((in%50)*(gaugeData[i+1]-gaugeData[i]))/50 +gaugeData[i];
		return (i+261);
	}
}

int gypsyMath2(int in)
{
if (in>=100)
  {
    return 232;  //38+192
  }
  else
  {
    int i=in/10;
    i=((in%10)*(gaugeData2[i+1]-gaugeData2[i]))/10 +gaugeData2[i];
    return (i+38);
  }
}
