// ============================================================
// ROBOT EYE + CLOCK + DISTANCE + WALL-E SOUND
// ESP32 DevKit V1
// OLED 0.96" (I2C), HC-SR04, Buzzer
// by sabar24 (versi alive: jeda komunikasi, stop buzzer >1m)
// ============================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include "time.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define TRIG_PIN 5
#define ECHO_PIN 18
#define BUZZER_PIN 13

int leftEyeX=40, rightEyeX=80, eyeY=18, eyeWidth=25, eyeHeight=30;
int targetLeftEyeX=leftEyeX, targetRightEyeX=rightEyeX, moveSpeed=2;
int blinkState=0, blinkDelay=2000, expression=0;
unsigned long lastBlinkTime=0, moveTime=0;

// mood sound control
unsigned long lastSoundTime=0;
int currentMood=-1;

const char* ssid1="HUAWEI-2.4G-Gaz6"; const char* pass1="58YnbSpT";
const char* ssid2="sabar24"; const char* pass2="123qwerty";
const char* ntpServer="pool.ntp.org"; const long gmtOffset_sec=8*3600; 
const int daylightOffset_sec=0; bool timeInitialized=false;

bool tryConnectWiFi(const char* ssid,const char* pass,int timeout){
  WiFi.begin(ssid,pass);
  display.clearDisplay(); display.setTextSize(1); display.setTextColor(WHITE);
  display.setCursor(0,20); display.print("Connecting "); display.print(ssid); display.display();
  unsigned long start=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-start<(unsigned long)timeout){delay(500);}
  if(WiFi.status()==WL_CONNECTED) return true; else return false;
}

void setup(){
  Serial.begin(115200);
  Wire.begin(21,22); pinMode(TRIG_PIN,OUTPUT); pinMode(ECHO_PIN,INPUT); pinMode(BUZZER_PIN,OUTPUT);
  digitalWrite(BUZZER_PIN,LOW);
  if(!display.begin(SSD1306_SWITCHCAPVCC,0x3C)){for(;;);}
  display.display(); delay(1000); display.clearDisplay();
  randomSeed(analogRead(0));

  bool connected=tryConnectWiFi(ssid1,pass1,10000);
  if(!connected) connected=tryConnectWiFi(ssid2,pass2,10000);
  if(connected){ configTime(gmtOffset_sec,daylightOffset_sec,ntpServer); struct tm t; if(getLocalTime(&t)) timeInitialized=true; WiFi.disconnect(true);}
}

// ===========================================================
// === Sound Functions with Natural Pauses ===
void soundExtremeClose(){
  unsigned long now=millis();
  if(now-lastSoundTime<2000) return;
  lastSoundTime=now;

  for(int f=400;f<=1500;f+=20){tone(BUZZER_PIN,f); delay(4);}
  noTone(BUZZER_PIN); delay(100);
  for(int f=1500;f>=400;f-=22){tone(BUZZER_PIN,f); delay(4);}
  noTone(BUZZER_PIN); delay(120);
  for(int i=0;i<3;i++){tone(BUZZER_PIN,random(600,1200)); delay(random(50,100)); noTone(BUZZER_PIN); delay(random(60,120));}
}

void soundHappy(){
  unsigned long now=millis();
  if(now-lastSoundTime<1200) return;
  lastSoundTime=now;

  int base[]={523,587,659,698,784,350,1000,950};
  for(int i=0;i<6;i++){
    int n=base[random(0,5)];
    tone(BUZZER_PIN,n); delay(random(60,100)); noTone(BUZZER_PIN); delay(random(80,550));
  }
}

void soundSad(){
  unsigned long now=millis();
  if(now-lastSoundTime<1000) return;
  lastSoundTime=now;

  for(int f=600;f>=400;f-=10){tone(BUZZER_PIN,f); delay(80);}
  noTone(BUZZER_PIN); delay(100);
  for(int i=0;i<2;i++){tone(BUZZER_PIN,random(220,350)); delay(random(50,80)); noTone(BUZZER_PIN); delay(random(40,80));}
}

void soundAngry(){
  unsigned long now=millis();
  if(now-lastSoundTime<1000) return;
  lastSoundTime=now;

  for(int i=0;i<4;i++){
    tone(BUZZER_PIN,1000+random(-100,400)); delay(random(40,200));
    noTone(BUZZER_PIN); delay(random(60,100));
  }
}

// ===========================================================
// === Distance Measurement ===
float getDistanceCM(){
  digitalWrite(TRIG_PIN,LOW); delayMicroseconds(2); digitalWrite(TRIG_PIN,HIGH); delayMicroseconds(10); digitalWrite(TRIG_PIN,LOW);
  long dur=pulseIn(ECHO_PIN,HIGH,25000); if(dur<=0) return -1; return dur*0.034/2;
}

// ===========================================================
// === Draw Clock & Eyes ===
void drawClock(){ struct tm t; display.clearDisplay(); 
  if(!getLocalTime(&t)){display.setTextSize(1); display.setCursor(10,25); display.print("No Time Data");} 
  else{display.setTextSize(2); display.setCursor(10,25); char s[9]; strftime(s,sizeof(s),"%H:%M:%S",&t); display.print(s);}
  display.display();
}

void drawEyes(int exp){
  display.clearDisplay();
  if(blinkState==0){drawExpression(leftEyeX,eyeY,eyeWidth,eyeHeight,exp); drawExpression(rightEyeX,eyeY,eyeWidth,eyeHeight,exp);}
  else{display.fillRect(leftEyeX,eyeY+eyeHeight/2-2,eyeWidth,4,WHITE); display.fillRect(rightEyeX,eyeY+eyeHeight/2-2,eyeWidth,4,WHITE);}
  drawMouth(exp); display.display(); delay(50);
}

void drawMouth(int exp){
  switch(exp){
    case 0: display.fillRect(45,55,40,3,WHITE); break;
    case 1: display.drawLine(45,55,65,60,WHITE); display.drawLine(65,60,85,55,WHITE); break;
    case 2: display.drawLine(45,60,65,55,WHITE); display.drawLine(65,55,85,60,WHITE); break;
    case 3: display.drawLine(45,52,85,52,WHITE); break;
  }
}

void drawExpression(int x,int y,int w,int h,int e){
  switch(e){
    case 0: display.fillRoundRect(x,y,w,h,5,WHITE); break;
    case 1: display.fillRoundRect(x,y,w,h,5,WHITE); display.fillRect(x+5,y+18,w-10,4,WHITE); break;
    case 2: display.fillRect(x,y+h/2-2,w,4,WHITE); break;
    case 3: 
      if(x<SCREEN_WIDTH/2) display.fillTriangle(x,y+h,x+w,y+h-5,x,y,WHITE); 
      else display.fillTriangle(x,y+h-5,x+w,y+h,x+w,y,WHITE); 
      break;
  }
}

void angryAnimation(){for(int i=0;i<4;i++){leftEyeX-=5;rightEyeX-=5; drawEyes(3); delay(60); leftEyeX+=10;rightEyeX+=10; drawEyes(3); delay(60); leftEyeX-=5;rightEyeX-=5;} drawEyes(3); delay(300);}

// ===========================================================
// === Mood Functions with single-call sound ===
void moodExtremeClose(){display.clearDisplay(); display.setTextSize(2); display.setTextColor(WHITE); display.setCursor(10,25); display.print("Sabar_24"); display.display(); soundExtremeClose();}
void moodClock(){drawClock();}
void moodHappy(){expression=1; drawEyes(expression); soundHappy();}
void moodSad(){expression=2; drawEyes(expression); soundSad();}
void moodAngry(){expression=3; angryAnimation(); soundAngry();}

// ===========================================================
// === Main Loop ===
void loop(){
  unsigned long t=millis();
  float d=getDistanceCM();

  // blinking
  if(t-lastBlinkTime>blinkDelay && blinkState==0){ blinkState=1; lastBlinkTime=t;}
  else if(t-lastBlinkTime>400 && blinkState==1){ blinkState=0; lastBlinkTime=t;}

  // random eye movement
  if(t-moveTime>random(2000,5000) && blinkState==0){int m=random(0,3); if(m==1){targetLeftEyeX=30;targetRightEyeX=70;} else if(m==2){targetLeftEyeX=45;targetRightEyeX=85;} else{targetLeftEyeX=40;targetRightEyeX=80;} moveTime=t;}
  leftEyeX+=(targetLeftEyeX-leftEyeX)/moveSpeed; rightEyeX+=(targetRightEyeX-rightEyeX)/moveSpeed;

  // mood selection based on distance
  int newMood=-1;
  if(d>0 && d<10) newMood=0;
  else if(d>=10 && d<45) newMood=1;
  else if(d>=45 && d<60) newMood=2;
  else if(d>=60 && d<80) newMood=3;
  else newMood=4;

  // jika jarak > 100 cm, matikan semua suara tapi tetap animasi
  if(d > 100) {
      currentMood = -1; // mood -1 = tidak ada suara
      drawEyes(expression); // tetap tampilkan mata
  } else {
      if(newMood != currentMood){ currentMood=newMood; }
      switch(currentMood){
          case 0: moodExtremeClose(); break;
          case 1: moodClock(); break;
          case 2: moodSad(); break;
          case 3: moodAngry(); break;
          case 4: moodHappy(); break;
      }
  }
}
 
