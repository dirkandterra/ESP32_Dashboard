//IntrVFD.cpp
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
//Routine to handle jacked up way Dodge decided to talk to vfd on
//the Intrepid Dashboard
//-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_
#include "IntrVFD.h"
#include "RS232Decode.h"

#define MAX_VFD_CHAR 6
int bottom3=0;
int top3=0;
uint8_t prepIt=0;
unsigned char vfd[8]={'3','2','1','C','B','A',0X00,0X00};
unsigned char vfdTemp[8];
unsigned char Font[64] = 
{0x0,0x2,0x6,0x24,0xb4,0x9c,0xf6,0x4, //space - '
0xd4,0x9a,0x36,0x64,0x88,0x20,0x08,0x22,  // ( - /
0xde,0x0a,0xf2,0xba,0x2e,0xbc,0xfc,0x1a,  //0-7
0xfe,0x3e,0xa0,0xa8,0x34,0x30,0x32,0x72, //8-?
0xfa,0x7e,0xec,0xd4,0xea,0xf4,0x74,0xdc, //@-G
0x6e,0x44,0xca,0x7c,0xc4,0x58,0x5e,0xde, //H-O
0x76,0x3e,0x60,0xbc,0xe4,0xce,0x8e,0x86, //P-W
0x6e,0xae,0xf2,0xd4,0x24,0x9a,0x16,0x80}; //X-'


uint8_t segAddr[8][7]={{21,18,14,11,8,4,1},{22,19,15,12,9,5,2},
                      {23,20,0,13,10,6,3},{53,50,46,43,40,36,33},
                      {54,51,47,44,41,37,34},{55,52,32,45,42,38,35},
                      {17,63,61,59,27,29,31},{7,39,48,16,57,49,25}};
                      //{31,29,27,59,61,63,17},{25,49,57,16,48,39,7}};

void populateVFD(void);
void smarterPopulateVFD(void);
void clearDisp(void);
void setVfdExtra(uint16_t symbol,uint8_t on);


void clearDisp()
{	
	vfdString[0]=0x00;
	vfdString[1]=0x00;
	vfdString[2]=0x00;
	vfdString[3]=0x00;
	vfdString[4]=0x00;
	vfdString[5]=0x00;
	vfdString[6]=0x00;
	vfdString[7]=0x00;
}

void printTextToVFD(char *c,uint8_t start,uint8_t len,uint8_t justify,uint8_t *destArray){
    uint8_t stringLen=strlen(c);
    int8_t diff=0;
    uint8_t leadingBlanks=0;
    int ii;
    if(MAX_VFD_CHAR<(len+start))
    {
        len=MAX_VFD_CHAR-start;
    }
    diff=len-stringLen;
    if(diff<0){
        stringLen=len;
        diff=0;
    }else{
        if(justify==J_RIGHT){
            leadingBlanks=diff;
        }else if(justify==J_MID){
            leadingBlanks=diff/2;
        }
        stringLen=leadingBlanks+stringLen;
    }
   
    for(ii=0;ii<len;ii++){
        if(leadingBlanks>0){
            leadingBlanks--;
            destArray[MAX_VFD_CHAR-1-(ii+start)]=0x20;           
        }else if(ii>=stringLen){
            destArray[MAX_VFD_CHAR-1-(ii+start)]=0x20;
        }else{
            destArray[MAX_VFD_CHAR-1-(ii+start)]=*c;
            c++;
        }
    }
}
uint8_t printNumToVFD(int32_t num,uint8_t start,uint8_t len,uint8_t dec,uint8_t justify,uint8_t *destArray){
    //decimal is right justify only
    //negative vals?
    uint8_t leadingBlanks=0;
    uint8_t neg=0;
    int8_t diff=0;
    uint8_t tempDigitArray[MAX_VFD_CHAR]={};
    int32_t tempi32=0;
    uint8_t digits=0;
    uint8_t spaceReq=0;
    uint32_t ii=0;
    uint8_t ret=0;
 
    if(MAX_VFD_CHAR<(len+start))
    {
        len=MAX_VFD_CHAR-start;
    }
    if(num<0){
        neg=1;
        tempi32=num/-1; //abs value
        spaceReq++;
    }else{
        tempi32=num;
    }
    for(ii=100000;ii>0 ;ii=ii/10){
        tempDigitArray[digits]=(uint8_t)(tempi32/ii);
        tempi32=tempi32%ii;
        if(!digits){
            if(tempDigitArray[digits]){
                digits++;
            }
           
        }else{
            digits++;
        }
        if(ii==1){
            ii=0;
        }
    }
    if(digits==0){
        digits=1;   //when the number is zero, still want to print 0;
    }
    if(dec){
        justify=J_RIGHT;
        start=MAX_VFD_CHAR-len;
        setVfdExtra(vfdDec,1);
        destArray[1]='0';   //leading 0
        //we need another digit for leading 0
        //leading zero
        if(digits==1){
            tempDigitArray[1]=tempDigitArray[0];
            tempDigitArray[0]=0;
            digits++;
        }     
    }else{
      setVfdExtra(vfdDec,0);
    }
    spaceReq=spaceReq+digits;
    diff=len-spaceReq;

    if(diff<0){
        spaceReq=len;
        diff=0;
        ret=1;
    }else{
        if(justify==J_RIGHT){
            leadingBlanks=diff;
        }else if(justify==J_MID){
            leadingBlanks=diff/2;
        }
        spaceReq=leadingBlanks+spaceReq;
    }
    digits=0;
    for(ii=0;ii<len;ii++){
        if(leadingBlanks>0){
            leadingBlanks--;
            destArray[MAX_VFD_CHAR-1-(ii+start)]=0x20;           
        }else if(ii>=spaceReq){
            destArray[MAX_VFD_CHAR-1-(ii+start)]=0x20;
        }else if(neg){
            neg=0;
            destArray[MAX_VFD_CHAR-1-(ii+start)]=0x2D; //minus
        }else{
            destArray[MAX_VFD_CHAR-1-(ii+start)]=tempDigitArray[digits]+0x30;
            digits++;
        }
    }
    return ret;
}

void setVfdExtra(uint16_t symbol,uint8_t on){
  uint8_t bit;
  if(symbol>0x80){
    bit=(uint8_t)(symbol/256);
    if(on){
      vfd[7]=vfd[7]|bit;
    }else{
      vfd[7]=vfd[7]&~bit;
    }
  }else{
    bit=(uint8_t)symbol;
    if(on){
      vfd[6]=vfd[6]|bit;
    }else{
      vfd[6]=vfd[6]&~bit;
    }
  }
}

void smarterPopulateVFD(void){
    uint8_t storeBit, storeByte;
    int ii=0,jj=0,segBit=0;
    uint8_t temp=0;
    for(ii=0;ii<6;ii++){
      //sanitize vfd data
      temp=vfd[ii]-0x20;
      if(temp>63){
        vfd[ii]=0x2A;
      }
    }
      vfdTemp[0]=Font[vfd[0]-0x20];
      vfdTemp[1]=Font[vfd[1]-0x20];
      vfdTemp[2]=Font[vfd[2]-0x20];
      vfdTemp[3]=Font[vfd[3]-0x20];
      vfdTemp[4]=Font[vfd[4]-0x20];
      vfdTemp[5]=Font[vfd[5]-0x20];
      vfdTemp[6]=vfd[6];
      vfdTemp[7]=vfd[7];

    for(ii=0;ii<8;ii++){
        for(jj=7;jj>0;jj--){
            storeByte=segAddr[ii][7-jj]/8;
            storeBit=segAddr[ii][7-jj]%8;
            storeBit=0x01<<storeBit;
            segBit=0x01<<jj;
            if(vfdTemp[ii]&segBit){
                vfdString[storeByte]|=storeBit;
                temp=storeBit;
            }else{
                vfdString[storeByte]&=~storeBit; 
                temp=~storeBit;
            }
        }

    }
}

//-_-_-_-_-_-_-_-_ Send Dimming Info to VFD -_-_-_-_-_-_-
void populateVFD(void)
{
  char dig=3;
  char digBase=3;
  char i;
  char segment_count=23;
  unsigned char d=0x80;
  char vf=2;
  char vfStop=0;
  vfdTemp[0]=Font[vfd[0]-0x20];
  vfdTemp[1]=Font[vfd[1]-0x20];
  vfdTemp[2]=Font[vfd[2]-0x20];
  vfdTemp[3]=Font[vfd[3]-0x20];
  vfdTemp[4]=Font[vfd[4]-0x20];
  vfdTemp[5]=Font[vfd[5]-0x20];

  clearDisp();
  
  //to-do decimal or no decimal first (vf
  while (vf>=vfStop)
  {
    for(i=segment_count;i>=0;i--)
    {
        vfdString[vf]=vfdString[vf]<<1;
        switch (i){
            case 17:       //Skip 17 and 16, not involved with 7 segment
                break;
            case 16:
                dig=digBase-1;
                vf--;
                vfdString[vf]=0;
                break;
            case 7:
                break;
            case 0:
                vf--;
                break;
            default:
              dig--;
                if (vfdTemp[dig]&d)
              {
                vfdString[vf]=vfdString[vf]+ 1;
              }
              if (dig==digBase-3)
              {
                dig=digBase;
                d=d>>1;
              }
              if(i==8){
                  vf--;
                  vfdString[vf]=0;
              }
                break;
        }
        
    }
    if(vf<0){
        vf=6;
        segment_count=23;
        vfStop=4;
        d=0x80;
        digBase=6;
        dig=6;
    }
    i=0;
  }
  
    // Middle Segment digit 2
  if (vfdTemp[2]&0x20) {vfdString[0]=vfdString[0]|0x01;}

  // Middle Segment digit 5
  if (vfdTemp[5]&0x20) {vfdString[4]=vfdString[4]|0x01;}
  
      //Decimal
  if (vfd[7]&0x80) {vfdString[0]=vfdString[0]|0x80;}
  
  //*********************Icons

  //Trip
  if (vfd[7]&0x40) {vfdString[4]=vfdString[4]|0x80;}

  //PRN
  if (vfd[7]&0x20) {vfdString[6]=vfdString[6]|0x01;}
  // D3L
  if (vfd[7]&0x10) {vfdString[2]=vfdString[2]|0x01;}
  
    //Trac
  if (vfd[7]&0x08) {vfdString[7]=vfdString[7]|0x02;}
  //OFF-Trac
  if (vfd[7]&0x04) {vfdString[6]=vfdString[6]|0x02;}
  //OD
  if (vfd[7]&0x02) {vfdString[3]=vfdString[3]|0x02;}
  //OFF-OD
  if (vfd[6]&0x80) {vfdString[2]=vfdString[2]|0x02;}
  
  //Gear Selection
  if (vfd[6]&0x40){vfdString[7]=vfdString[7]|0x80;}
  if (vfd[6]&0x20){vfdString[7]=vfdString[7]|0x20;}
  if (vfd[6]&0x10){vfdString[7]=vfdString[7]|0x08;}
  if (vfd[6]&0x08){vfdString[3]=vfdString[3]|0x08;}
  if (vfd[6]&0x04){vfdString[3]=vfdString[3]|0x20;}
  if (vfd[6]&0x02){vfdString[3]=vfdString[3]|0x80;}

}


