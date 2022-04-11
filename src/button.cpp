#include <button.h>
#include <network.h>
#include <Arduino.h>
#include <OneButton.h>

extern Network *network;
extern OneButton button1;
extern OneButton button2;

bool btnClick1;
bool btnDouble1;
bool btnLongStart1;
bool btnLongPress1;
bool btnLongEnd1;
bool btnMulti1;

bool btnClick2;
bool btnDouble2;
bool btnLongStart2;
bool btnLongPress2;
bool btnLongEnd2;
bool btnMulti2;

int nClicks1 = 0;
int nClicks2 = 0;

int pressDuration1 = 0; // Button 1 press duration in seconds
int pressDuration2 = 0; // Button 2 press duration in seconds

int pressStart;
int pressStop;

void click1() {
  Serial.println("Button 1 click.");
  btnClick1 = true;
} // click1

// This function will be called when the button1 was pressed 2 times in a short timeframe.
void doubleclick1() {
  Serial.println("Button 1 doubleclick.");
  btnDouble1 = true;
} // doubleclick1

void multiclick1(){
  Serial.println("Button 1 multiclick."); Serial.print("Clicks: "); Serial.println(button1.getNumberClicks());
  btnMulti1 = true; 
  nClicks1 = button1.getNumberClicks();
} // multiclick

// This function will be called once, when the button1 is pressed for a long time.
void longPressStart1() {
  Serial.println("Button 1 longPress start");
  btnLongStart1 = true;
  pressStart = millis();
} // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
  btnLongPress1 = true;
  
} // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  Serial.println("Button 1 longPress stop");
  btnLongEnd1 = true;
  pressStop = millis();
  int time = pressStop - pressStart;
  if (time > 4000 && time < 5000){
    pressDuration1 = 4;
    Serial.print("Button 1 Held for 4 seconds");
  }
} // longPressStop1


// ... and the same for button 2:

void click2() {
  Serial.println("Button 2 click.");
  btnClick2 = true;
} // click2


void doubleclick2() {
  Serial.println("Button 2 doubleclick.");
  btnDouble2 = true;
} // doubleclick2

void multiclick2(){
  Serial.println("Button 2 multiclick."); Serial.print("Clicks: "); Serial.println(button2.getNumberClicks());
  btnMulti2 = true; 
  nClicks2 = button2.getNumberClicks();
} // multiclick


void longPressStart2() {
  Serial.println("Button 2 longPress start");
  btnLongStart2 = true;
  pressStart = millis();
} // longPressStart2


void longPress2() {
  btnLongPress2 = true;
} // longPress2

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
  btnLongEnd2 = true;
  pressStop = millis();
  int time = pressStop - pressStart;
  if (time > 4000 && time < 5000){
    pressDuration2 = 4;
    Serial.print("Button 2 Held for 4 seconds");
  }
} // longPressStop2