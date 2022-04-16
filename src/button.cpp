#include <button.h>
#include <network.h>
#include <Arduino.h>
#include <OneButton.h>

extern Network *network;
extern OneButton button1;
extern OneButton button2;
extern OneButton button3;

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

bool btnClick3;
bool btnDouble3;
bool btnLongStart3;
bool btnLongPress3;
bool btnLongEnd3;
bool btnMulti3;

int nClicks1 = 0;
int nClicks2 = 0;
int nClicks3 = 0;

int pressDuration1 = 0; // Button 1 press duration in seconds
int pressDuration2 = 0; // Button 2 press duration in seconds
int pressDuration3 = 0; // Button 2 press duration in seconds

int pressStart1 = 0;
int pressStop1 = 0;
int pressStart2 = 0;
int pressStop2 = 0;
int pressStart3 = 0;
int pressStop3 = 0;

// Button 1 :

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
  Serial.print("Button 1 multiclick."); Serial.print(" | Clicks: "); Serial.println(button1.getNumberClicks());
  btnMulti1 = true; 
  nClicks1 = button1.getNumberClicks();
} // multiclick

// This function will be called once, when the button1 is pressed for a long time.
void longPressStart1() {
  Serial.println("Button 1 longPress start");
  btnLongStart1 = true;
  pressStart1 = millis();
} // longPressStart1


// This function will be called often, while the button1 is pressed for a long time.
void longPress1() {
  btnLongPress1 = true;
  pressStop1 = millis();
  pressDuration2 = floor((pressStop1 - pressStart1)/1000);
  
} // longPress1


// This function will be called once, when the button1 is released after beeing pressed for a long time.
void longPressStop1() {
  Serial.println("Button 1 longPress stop");
  btnLongEnd1 = true;
  pressStop1 = millis();
  pressDuration1 = floor((pressStop1 - pressStart1)/1000);
  Serial.printf("Button 1 Held for %d seconds\n", pressDuration1);
} // longPressStop1


// Button 2 :

void click2() {
  Serial.println("Button 2 click.");
  btnClick2 = true;
} // click2


void doubleclick2() {
  Serial.println("Button 2 doubleclick.");
  btnDouble2 = true;
} // doubleclick2

void multiclick2(){
  Serial.print("Button 2 multiclick."); Serial.print(" | Clicks: "); Serial.println(button2.getNumberClicks());
  btnMulti2 = true; 
  nClicks2 = button2.getNumberClicks();
} // multiclick


void longPressStart2() {
  Serial.println("Button 2 longPress start");
  btnLongStart2 = true;
  pressStart2 = millis();
} // longPressStart2


void longPress2() {
  btnLongPress2 = true;
  pressStop2 = millis();
  pressDuration2 = floor((pressStop2 - pressStart2)/1000);
} // longPress2

void longPressStop2() {
  Serial.println("Button 2 longPress stop");
  btnLongEnd2 = true;
  pressStop2 = millis();
  pressDuration2 = floor((pressStop2 - pressStart2)/1000);
  Serial.printf("Button 2 Held for %d seconds\n", pressDuration2);
} // longPressStop2

// Button 3 :

void click3() {
  Serial.println("Button 3 click.");
  btnClick3 = true;
} // click3

void doubleclick3() {
  Serial.println("Button 3 doubleclick.");
  btnDouble3 = true;
} // doubleclick3

void multiclick3(){
  Serial.print("Button 3 multiclick."); Serial.print(" | Clicks: "); Serial.println(button3.getNumberClicks());
  btnMulti3 = true; 
  nClicks3 = button3.getNumberClicks();
} // multiclick

void longPressStart3() {
  Serial.println("Button 3 longPress start");
  btnLongStart3 = true;
  pressStart3 = millis();
} // longPressStart3

void longPress3() {
  btnLongPress3 = true;
  pressStop3 = millis();
  pressDuration3 = floor((pressStop3 - pressStart3)/1000);
} // longPress3

void longPressStop3() {
  Serial.println("Button 3 longPress stop");
  btnLongEnd3 = true;
  pressStop3 = millis();
  pressDuration3 = floor((pressStop3 - pressStart3)/1000);
  Serial.printf("Button 3 Held for %d seconds\n", pressDuration3);
}