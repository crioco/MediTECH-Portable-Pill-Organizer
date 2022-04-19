#include <definition_libraries.h>
#include <functions.h>
#include "driver/rtc_io.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <EEPROM.h>

TaskHandle_t loop_2_t;

OneButton button1 (button1_pin, true);
OneButton button2 (button2_pin, true);
OneButton button3 (button3_pin, true);

extern Display *display;
extern Network *network;
extern Bluetooth *bluetooth;

extern const int LED1;
extern const int LED2;
extern const int LED3;
extern const int LED4;

extern const int LEDa;
extern const int LEDb;
extern const int LEDc;
extern const int LEDd;
extern const int LEDe;

extern bool btnClick1;
extern bool btnDouble1;
extern bool btnLongStart1;
extern bool bLongPress1;
extern bool btnLongEnd1;
extern bool btnMulti1;

extern bool btnClick2;
extern bool btnDouble2;
extern bool btnLongStart2;
extern bool btnLongPress2;
extern bool btnLongEnd2;
extern bool btnMulti2;

extern bool btnClick3;
extern bool btnDouble3;
extern bool btnLongStart3;
extern bool btnLongPress3;
extern bool btnLongEnd3;
extern bool btnMulti3;

extern int pressDuration1;
extern int pressDuration2;
extern int pressDuration3;

extern int nClicks1;
extern int nClicks2;

extern bool isWiFiEnabled;
extern bool isWiFiConnected;
extern bool isBluetoothEnabled;
extern bool bluetoothAuth;

int displayCountStart = 0;

extern int buzzStartMillis;
extern int alarmSoundCounter;
extern bool alarmSoundOn;
extern bool alarmOn;


bool isDeviceOpen = false;
bool LedMatrixOn = false;
extern std::vector<std::vector<int>> LEDVec;

extern float temperature;
extern float humidity;
extern int snoozeAmount;

RTC_DATA_ATTR int bootCount = 0;

// -----------------------------------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> VEC = {{1, 3}, {2, 0}, {3, 1}, {4, 2}, {5, 4}}; // FOR TESTING

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
    case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case 3  : Serial.println("Wakeup caused by timer"); break;
    case 4  : Serial.println("Wakeup caused by touchpad"); break;
    case 5  : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.println("Wakeup was not caused by deep sleep"); break;
  }
}

void DisplayOn(){
  display->PowerSaveOn(false);
  displayCountStart = millis();
}

void loop_2(void * pvParameters){

  //Loop
  for(;;){
    // Checks button 1, 2, & 3 Status
    button1.tick();
    button2.tick();
    button3.tick();

    // Alarm Sound
    if (alarmSoundOn){
      buzzStartMillis = millis();
      alarmSoundCounter = 0;
      while(alarmSoundOn){
        alarmSound();
        button1.tick();
        button2.tick();
      } 
      digitalWrite(buzzerPin, LOW);
      alarmSoundCounter = 0;
    }

    // Display Led Matrix
    if (LedMatrixOn){
      displayLED(LEDVec);
    }

    // Serial.print("Core: "); Serial.println(xPortGetCoreID());
  }
}

void setup() {

  // Loop() Task running on Core 0
  xTaskCreatePinnedToCore(
    loop_2,                               // Task Function
    "Loop_2",                             // Task Name
    10000,                                // Task Stack Size (bytes)
    NULL,                                 // Parameters
    0,                                    // Priority
    &loop_2_t,                            // Task Handler
    0);                                   // CPU Core 0/1 to run on

  delay(500); // Delay

  // ---------------------------------------------------
  
  Serial.begin(115200);

  Wire.begin(); // Initialize default I2C pins (GPIO 21 & 22) 
  Wire1.begin(SDA_2, SCL_2); // Initialize second I2C pins (GPIO 19 & 18)

  esp_sleep_enable_ext0_wakeup((gpio_num_t)button3_pin, 0);
  rtc_gpio_pullup_en((gpio_num_t)button3_pin);
  ++bootCount;
  Serial.printf("Booted %d times\n", bootCount);
  print_wakeup_reason();

  pinMode(buzzerPin, OUTPUT);
  // pinMode(vib_pin, OUTPUT);

  // ledcSetup(1, 2000, 8);
  // ledcAttachPin(buzzerPin, 1);

  analogReadResolution(11);
  analogSetAttenuation(ADC_6db);
  pinMode(LiPoLevelPin, INPUT);

  // LED Pins ------------------------------------------

  // // LED Columns
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  
  // LED Rows
  pinMode(LEDa, OUTPUT);
  pinMode(LEDb, OUTPUT);
  pinMode(LEDc, OUTPUT);
  pinMode(LEDd, OUTPUT);
  pinMode(LEDe, OUTPUT);

  // BUTTONS -------------------------------------------

  pinMode(button1_pin, INPUT_PULLUP);
  pinMode(button2_pin, INPUT_PULLUP);
  pinMode(button3_pin, INPUT_PULLUP);
  pinMode(REEDSwitchPin, INPUT_PULLUP);

  button1.attachClick(click1);
  button1.attachDoubleClick(doubleclick1);
  button1.attachMultiClick(multiclick1);
  button1.attachLongPressStart(longPressStart1);
  button1.attachLongPressStop(longPressStop1);
  button1.attachDuringLongPress(longPress1);

  button2.attachClick(click2);
  button2.attachDoubleClick(doubleclick2);
  button2.attachMultiClick(multiclick2);
  button2.attachLongPressStart(longPressStart2);
  button2.attachLongPressStop(longPressStop2);
  button2.attachDuringLongPress(longPress2);

  button3.attachClick(click3);
  button3.attachDoubleClick(doubleclick3);
  button3.attachMultiClick(multiclick3);
  button3.attachLongPressStart(longPressStart3);
  button3.attachLongPressStop(longPressStop3);
  button3.attachDuringLongPress(longPress3);

  // ---------------------------------------------------
  // INITIALIZE COMPONENTS

  network->initWiFi();
  initRTC();
  initAHT();
  initSPIFFS();
  display->initDisplay();
  getPillListfromJSON();
  Serial.print("Before: ");Serial.println(snoozeAmount);
  loadConfigJSON();
  Serial.print("After: ");Serial.println(snoozeAmount);

  // ---------------------------------------------------

  EEPROM.begin(1);
  if (EEPROM.read(0) == 1){
    EEPROM.write(0, 0);
    EEPROM.commit();
    initNetwork();
  }

  displayCountStart = millis();
}

int x = 1;

void loop() {
  // Checks Battery Level in Percentage (3.1 V - 4.2 V)
  // checkBattery();

  // alarmSoundOn = true;

  display->displayTime();
  checkAlarm();
  checkHumTemp();

  if (isBluetoothEnabled){
    if (!bluetoothAuth){
      bluetooth->readAuth();
    } else {
      bluetooth->readJSON();
    } 
  }

  // TURN DISPLAY ON FROM POWER SAVE
  if (btnClick1 == true || btnClick2 == true){
    DisplayOn();
    btnClick1 = btnClick2 = false;
  }

  // DISPLAY POWER SAVE AFTER 30 SECONDS => Turn Display OFF
  if ((millis() - displayCountStart) > DisplaySleepDelay){
    display->PowerSaveOn(true);
  }

  // WiFi ON/OFF
  if (btnMulti1 == true){
    DisplayOn();
    btnMulti1 = false;
    switch (nClicks1)
    {
    case 3:
      if (isWiFiConnected){
        network->WiFiDisconnect();
      } else {
        if (isBluetoothEnabled){
          EEPROM.write(0, 1);
          EEPROM.commit();

          Serial.println("BLUETOOTH OFF");
          bluetooth->BTDisconnect();
          Serial.println("RESETING DEVICE");
          ESP.restart();
        } else initNetwork();
        
      }
      break;

    case 4:
      displayPillList();
      break;
    case 5:
      readFirestoreQueue();  
      break;
    case 6:
      readDataStorageJSON();
      break;
    } 
    nClicks1 = 0;  
  }

  // BLUETOOTH ON/OFF
  if (btnMulti2 == true){
    DisplayOn();
    btnMulti2 = false;
    switch (nClicks2)
    {
    case 3:
      if (!isBluetoothEnabled){
        Serial.println("BLUETOOTH ON");
        bluetooth->initBluetooth();
      } else {
        Serial.println("BLUETOOTH OFF");
        bluetooth->BTDisconnect();
        Serial.println("RESETING DEIVCE");
        ESP.restart();
      }
      break;
    }
    nClicks2 = 0;
  }

  // TURN DEEP SLEEP ON
  if(btnLongPress3){
    DisplayOn();
    btnLongPress3 = false;
    if (pressDuration3 == 5){
      pressDuration3 = 0;
      Serial.println("Going to Sleep...");
      delay(2000);
      Serial.println("[Sleep]");
      delay(500);
      display->PowerSaveOn(true);
      esp_sleep_enable_timer_wakeup(30 * 1000000);
      esp_deep_sleep_start();
    }
  }

  // SHOW DEVICE HUMIDITY AND TEMPERATURE
  if (btnDouble1){
    btnDouble1 = false;
    DisplayOn();
    display->HumTemp(humidity, temperature);
    delay(3000);
    DisplayOn();
  }

  // if (btnClick1 == true){
  //   x++;
  //   if (x > 5) x=1;
  // } btnClick1 = false;

  // switch (x)
  // {
  // case 1:
  //   display->WiFiEnabled();
  //   break;
  // case 2:
  //   display->WiFiDisabled();
  //   break;
  // case 3:
  //   display->WiFiReconnecting();
  //   break;
  // case 4:
  //   display->WiFiDisconnected();
  //   break;
  // case 5:
  //   display->ComponentStatus();
  //   break;
  // }
}


