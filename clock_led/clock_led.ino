#include "OneButton.h"
#include <SPI.h>
const int  cs=8; //chip select 
const int csc = 10;
const boolean serial = 1;
int onoff = 0;

int tenH;
int H;
int tenM;
int M;
int tenS;
int S;
// Setting the time:
//int flagled = 3;
int clickled = 6;
OneButton button(2,1);
bool holdFlag = 0;
int clickFlag = 0;


// End Time setting
void setup() {
  pinMode(csc,OUTPUT);
  RTC_init();
  if(serial){
    Serial.begin(9600);
    Serial.println("Ready...");
  }
  ledInit(); // Must be called AFTER RTC_init //
  //day(1-31), month(1-12), year(0-99), hour(0-23), minute(0-59), second(0-59)
  //SetTimeDate(11,12,13,23,59,55);
  // Setting Time
  //pinMode(flagled,OUTPUT);
  pinMode(clickled,OUTPUT);
  button.attachClick(oneClick);
  button.attachPress(longHold);
  button.attachDoubleClick(doubleClick);

}

void loop() {
  button.tick();
 
  getTime();
  
  if(holdFlag){
    flashMode(clickFlag);
  }
  else{
   sendit(0x1,tenH+128);
   sendit(0x2,H);
   sendit(0x3,tenM);
   sendit(0x4,M);
  }
  if(serial)
    debug();
  
  //delay(10);
}

//=====================================
void flashMode(int digit){
  int col = getDigit(digit);  
   sendit(0x1,tenH+128);
   sendit(0x2,H);
   sendit(0x3,tenM);
   sendit(0x4,M);
    sendit(digit,B00001111);
    delay(50);
    sendit(digit,col);
    delay(50);
}
int RTC_init(){ 
  
	  pinMode(cs,OUTPUT); // chip select
          
	  // start the SPI library:

	  SPI.begin();
	  SPI.setBitOrder(MSBFIRST); 
	  SPI.setDataMode(SPI_MODE3); // both mode 1 & 3 should work 
	  //set control register 

	  digitalWrite(cs, LOW);  
	  SPI.transfer(0x8E);
	  SPI.transfer(0x60); //60= disable Osciallator and Battery SQ wave @1hz, temp compensation, Alarms disabled
	  digitalWrite(cs, HIGH);
	  delay(10);

}
//=====================================
int SetTime(int place, int num, int pair){ 
  int addr;
  Serial.print(place,BIN);
  Serial.print(",  ");
  Serial.println(num,BIN);
  switch(place){
    case 1:
      addr = 0x82;
      num = (num<<4)+pair;
      break;		  
    case 2:
      addr = 0x82;
      num = (pair<<4)+num;
      break;
    case 3:
      addr = 0x81;
      num = (num<<4)+pair;
      break;
    case 4:
      addr = 0x81;
      num = (pair<<4)+num;
      break;

  }
  digitalWrite(cs, LOW);
  SPI.transfer(addr); 
  SPI.transfer(num);        
  digitalWrite(cs, HIGH);
  // Set seconds to 0
  digitalWrite(cs, LOW);
  SPI.transfer(0x80); 
  SPI.transfer(B00000000);        
  digitalWrite(cs, HIGH);

}



int SetTimeDate(int d, int mo, int y, int h, int mi, int s){ 
 
	int TimeDate [7]={s,mi,h,0,d,mo,y};
	for(int i=0; i<=6;i++){
		if(i==3)
			i++;
		int b= TimeDate[i]/10;
		int a= TimeDate[i]-b*10;
		if(i==2){
			if (b==2)
				b=B00000010;
			else if (b==1)
				b=B00000001;
		}	
		TimeDate[i]= a+(b<<4);
		  
		digitalWrite(cs, LOW);
		SPI.transfer(i+0x80); 
		SPI.transfer(TimeDate[i]);        
		digitalWrite(cs, HIGH);
  }
  
}
/////////////////////////////////////
void getTime(){	
  for(int i=0; i<=2;i++){
    digitalWrite(cs, LOW);
    SPI.transfer(i+0x00); 
    unsigned int n = SPI.transfer(0x00); 
    digitalWrite(cs, HIGH);
          
   if(i == 0){
      S = (n & B00001111) ;
      tenS = (n & B11110000) >>4;
    } 
    if(i == 1){
      M = (n & B00001111) ;
      tenM = (n & B11110000) >>4;
    }   
    if(i == 2){
      H = (n & B00001111) ;
      tenH = (n & B11110000) >>4;
    }   
  }

}


int readFlag(){
  uint8_t ans;
  digitalWrite(cs,LOW);
  SPI.transfer(0x0f);
  ans = SPI.transfer(-1);
  digitalWrite(cs,HIGH);
  Serial.println(ans,BIN);
  return (ans >> 7);
}
void clearFlag(){
  digitalWrite(cs,LOW);
  SPI.transfer(0x8f);
  SPI.transfer(B01001000);
  digitalWrite(cs,HIGH);
}
void debug(){
  Serial.print(tenH);
  Serial.print(H);
  Serial.print(":");
  Serial.print(tenM);
  Serial.print(M);
  Serial.print(":");
  Serial.print(tenS);
  Serial.println(S);
}
void ledInit(){
  sendit(0x9,0xff); // Decode Mode
  sendit(0xa,0x2); // Intensity
  sendit(0xB,0x3); // Scan limit
  //sendit(0xc,0x1); //shutdown
  disp();

}
void sendit(byte addr,byte val){ 
  digitalWrite(csc,LOW);
  SPI.transfer(addr);
  SPI.transfer(val);
  digitalWrite(csc,HIGH);
//delay(100);
}

void longHold() {
  if( holdFlag ){
    holdFlag = 0;
    //digitalWrite(flagled,LOW);
    digitalWrite(clickled,LOW);
      // Set seconds to 0
    digitalWrite(cs, LOW);
    SPI.transfer(0x80); 
    SPI.transfer(B00000000);        
    digitalWrite(cs, HIGH);
    clickFlag = 0;
  }
  else{
    holdFlag = 1;
    //digitalWrite(flagled,HIGH);
    clickFlag++;
  } 
} // longHold

void oneClick() {
  if(holdFlag){
    int pair;
    int limit;
    int existing = getDigit(clickFlag);
    switch(clickFlag){
      case 1:
        limit = 2;
        pair = getDigit(clickFlag+1);
        break;
      case 2:
        limit = 9;
        pair = getDigit(clickFlag-1);
        break;
      case 3:
        limit = 6;
        pair = getDigit(clickFlag+1);
        break;
      case 4:
        limit = 9;
        pair = getDigit(clickFlag-1);
        break;
    }
    if( existing >= limit ){
      existing = 0;
      SetTime(clickFlag,existing,pair);
    }
    else{
      SetTime(clickFlag,existing+1,pair);
    }
  }
  else{
      disp();
    return;
  }
} 


int getDigit(int place){
  int col;
  switch (place){
    case 1:
      col = tenH;break;
    case 2:
      col = H;break;
    case 3:
      col = tenM;break;
    case 4:
      col = M;break;
  }
  return col;
}


void doubleClick() {
  if(holdFlag){
    clickFlag++;
    if(clickFlag >4){
      clickFlag = 1;
    }
    digitalWrite(clickled,HIGH);
  }
  else{
    digitalWrite(clickled,LOW);
  }
} // doubleclick

void disp(){
  if(onoff == 1){
      sendit(0xC,0);
      onoff = 0;
    }
    else{
      sendit(0xC,1);
      onoff = 1;
    }
    
}

