#include <Wire.h>
#include <RTClib.h>
#include <LowPower.h>

void wakeup(); //turns SCL and SDA pins back on, as well as VCC back to clock
void VCCpowerdown(); // turns I2C communication off and vcc to clock off and optical sensor to ensure clock is only running on button battery power
void sleep(); // turns off all functionaility and sleeps 8 seconds
void wait1(); //waits for 20 minutes and flips bait check flag
void wait2(); //waits 20 minutes flips bait check flag, turns motor on
void motorForward(); //sets pins to make motor push the piston forward
void motorReverse(); // sets pins to make motor pull the piston backwards
void motorStop(); // sets pins to make motor stop
int baitcount;
bool previousWait = false;
int lastKnownHour;
int lastKnownMinute;
int lastKnownSecond;
int timeComp;



RTC_DS3231 rtc; // Create an RTC object using RTClib

void setup() {
  baitcount = 0; //sets bait counter
  previousWait = false; //always starts previous bait check as false after reset
   
   for (int pin = 4; pin <= 13; pin++) { //setting all digital pins to low to avoid internal pull ups drawing unintended power
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }

  // Set analog pins A0-A5 to LOW (they can act as digital pins) // sets all digital pins to low to avoid internal pullup pins from drawing power 
  for (int pin = A1; pin <= A3; pin++) {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
  }
  pinMode(A0, INPUT_PULLUP);//input for optical sensor
  pinMode(3, INPUT_PULLUP); //back reed switch, when circuit is open, pin will read HIGH 
  pinMode(2, INPUT_PULLUP); // front reed switch, when circuit is open, pin will read HIGH
  pinMode(A4, INPUT);
  pinMode(A5, INPUT);
  digitalWrite(11,LOW);//control pin for VCC circuit
  delay(100); //changed this from 500 to 100 5/27/2025 11:37AM
 
  Wire.begin();
  delay(100); //changed from 50 to 100 5/13/2025 9:11AM
  rtc.begin();
  
  //Run rtc.adjust once to update time to the current date and time the sketch is compiled, then comment out and re upload
  
  delay(5);
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% TIME ADJUST %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  delay(100);
 Serial.begin(9600);

 VCCpowerdown();
  delay(5);
 motorReverse();
 
 

}

void loop(){
  wakeup();
  delay(150); //delay in ms to allow IR sensor to stabilize and make a first reading.
  DateTime now = rtc.now();
  delay(100); //changed from 50 to 100 5/12/2025 8:52AM
  int hour;
  //delay(5);
  int minute;
  //delay(5);
  int BaitCheck = digitalRead(A0);
  delay(5);
  
  if (now.year() > 2020) {
  Serial.println("Clock normal");
  delay(100);
  lastKnownHour = now.hour();    
  lastKnownMinute = now.minute(); 
  lastKnownSecond = now.second();
  }

  else if (now.year() <= 2020) {
  Serial.println("Clock reset, updating times with last known values");
  delay(100);

  lastKnownMinute += timeComp;

  if (lastKnownMinute >= 60) {
    lastKnownMinute -= 60;
    lastKnownHour += 1;

    if (lastKnownHour >= 24) {
      lastKnownHour = 0;
    }
  }

  rtc.adjust(DateTime(2025, 5, 11, lastKnownHour, lastKnownMinute, lastKnownSecond));
  delay(100);
  now = rtc.now();
  delay(100);
 
}

  hour = now.hour();
  delay(5);
  minute = now.minute();
  delay(5);


Serial.print(hour,DEC);
Serial.print(":");
Serial.println(minute,DEC);
delay(100);
Serial.print("Bait count: ");
Serial.println(baitcount,DEC);
 //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% BAIT COUNT ADJUST %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  if(baitcount >=40 ){// checks bait counter, goes to sleep if no baits left
   Serial.println("Out of bait, going to sleep");
   delay(100);
    VCCpowerdown();
      sleep();
  }
 //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

 //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% OPERATION WINDOW ADJUST %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  else if(hour >= 6 && hour < 18){// if time is between 6Pm and 6Am, go to sleep. Using daytime hours 6Am to 6pm for testing.
    Serial.println("Outside hours of op");
    delay(100);
    VCCpowerdown();
      sleep();
  }
 //%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  else if (BaitCheck != LOW && previousWait == false) { //if no bait is detected and previous baitcheck = False, proceed to wait1 (20 minute sleep cycle), 

    Serial.println("No bait detected, go to wait 1");
    delay(100);
    VCCpowerdown();
    wait1();
  
  } 

  else if (BaitCheck != LOW && previousWait == true){// if no bait is detected and we previously waited 20 minutes (previousbait check = true), go to sleep for 20 minutes and start loading bait (motorReverse)
    
    Serial.println("No bait detected proceed to wait 2 and deploy");
    delay(100);
    VCCpowerdown();
    wait2();
  }    
  
  else {
    
      Serial.println("Bait present, going to sleep");
      delay(100);
      VCCpowerdown();
      sleep();
      
  }
}  
    
void VCCpowerdown(){//powersdown VCC circuit, IR sensor and RTC. writes A4 and A5 low which are I2C com lines. if left high, they will continue powering the RTC
    
  Wire.end();
  delay(100); //changed from 50 to 100 5/13/2025 9:16AM
  pinMode(A4, OUTPUT); 
 // delay(5);
  pinMode(A5, OUTPUT);
 // delay(5);  
  digitalWrite(A4, LOW);
  delay(5);
  digitalWrite(A5, LOW);
  delay(100);//changed this from 750 to 100 5/27/2025 11:37AM
  digitalWrite(11, HIGH);
    delay(5);
}

void sleep(){
    for(int i =0; i<36; i++){
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
    }
    
     timeComp = 5;

}

void wakeup(){
    pinMode(A4, INPUT);
    pinMode(A5, INPUT);
    digitalWrite(11,LOW); //turns VCC power circuit on
    delay(100); //changed this from 500 to 100 5/27/2025 11:40AM
    Wire.begin(); // Starts I2c communication
    delay(100); //changed this from 50 to 100 5/13/2025 9:17am
}

void motorReverse(){
  Serial.println("Piston reverse");
  delay(100);
  unsigned long milliStart = millis();//calls current milliseconds since arduino was powered on to use for time difference checks later
  delay(5);
  digitalWrite(9, LOW);
  delay(5);

  for(int i=0; i<255; i++){//ramps motor up to full speed
    analogWrite(10, i);
    delay(1);
  }

  while(millis() - milliStart <= 27000){//if the time since motor start is less than 50 seconds, motor will run and check for back reed switch to go low. if back reed switch goes low and stays low, while loop breaks and moves to next operation
    if(digitalRead(3)==LOW){
      delay(30);
      if(digitalRead(3)==LOW){
        break;
      }
    }
  }
  
  for(int i=255; i>=0; i--){//after while loop is broken, motor ramps down to stop
    analogWrite(10, i);
    delay(1);
  }

  digitalWrite(10, LOW); //ensures motor is stopped
  
  delay(75); //wait for steady state

  motorForward();

}

void motorForward(){
  Serial.println("Piston forward");
  delay(100);
  unsigned long milliStart = millis();
  delay(5);
  digitalWrite(10, LOW);
  delay(5);

  for(int i = 0; i<255; i++){//ramps motor up slowly to full speed
    analogWrite(9, i);
    delay(1);
  }
  
  while(millis() - milliStart <= 27000){//if the time since motor start is less than 50 seconds, motor will run and check for front reed switch to go low. if back front switch goes low and stays low, while loop breaks and moves to next operation
    if(digitalRead(2)==LOW){
      delay(30);
      if(digitalRead(2)==LOW){
        break;
      }
    }
  }

  for(int i = 255; i >= 0; i--){
    analogWrite(9, i);
    delay(1);
  }

  baitcount = baitcount + 1;

  motorStop();

}

void motorStop(){

  digitalWrite(10, LOW);
  delay(5);
  digitalWrite(9, LOW);
  delay(5);
  
}

void wait1(){ //wait X minutes and flip previous wait flag
  
    for( int i=0; i < 4; i++){
    sleep();
  }
  previousWait = !previousWait;
  timeComp = 20;
}

void wait2(){// wait X minutes reverse previous wait flag and start motor to begin loading bait
  
    for( int i=0; i < 4; i++){
    sleep();
  }
  previousWait = !previousWait;
  timeComp = 21; //add one additional minute to wait period for motor run time
  motorReverse();
}