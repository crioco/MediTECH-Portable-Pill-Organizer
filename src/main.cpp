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
extern int nClicks3;

extern bool isWiFiEnabled;
extern bool isWiFiConnected;
extern bool isBluetoothEnabled;
extern bool bluetoothAuth;
extern bool isBTClientConnected;
extern bool isBTClientDisconnected;

int displayCountStart = 0;

extern int buzzStartMillis;
extern int alarmSoundCounter;
extern bool alarmSoundOn;
extern bool alarmOn;

extern int checkBatteryStart;
// extern U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2;

bool isDeviceOpen = false;
bool LedMatrixOn = false;
extern std::vector<std::vector<int>> LEDVec;

extern float temperature;
extern float humidity;

RTC_DATA_ATTR int bootCount = 0;

// -----------------------------------------------------------------------------------------------------------------------------
// std::vector<std::vector<int>> VEC = {{1, 3}, {2, 0}, {3, 1}, {4, 2}, {5, 4}}; // FOR TESTING

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
  pinMode(vib_pin, OUTPUT);

  // LiPo Battery Level
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
  loadConfigJSON();

  // // ---------------------------------------------------

  // If restarted after WiFi on while BLE on
  EEPROM.begin(1);
  if (EEPROM.read(0) == 1){
    EEPROM.write(0, 0);
    EEPROM.commit();
    initNetwork();
  }

  // Turn on Display
  display->PowerSaveOn(false);
}

int x = 1;

void loop() {
  // Checks Battery Level in Percentage (3.3 V - 4.11 V)
  checkBattery();

  // alarmSoundOn = true;

  display->displayTime();
  checkAlarm();
  checkHumTemp();

  if (isBluetoothEnabled){
    if (!bluetoothAuth){
      bluetooth->readAuth();
    } else {
      bluetooth->readBluetoothSerial();
    } 
  }

  if (isBTClientConnected){
    isBTClientConnected = false;
    display->BTClientConnect();
    delay(2000);
  } else if(isBTClientDisconnected){
    isBTClientDisconnected = false;
    display->BTClientDisconnect();
    delay(2000);
  }

  // TURN DISPLAY ON FROM POWER SAVE
  if (btnClick1 == true || btnClick2 == true){
    display->PowerSaveOn(false);
    btnClick1 = btnClick2 = false;
  }

  // DISPLAY POWER SAVE AFTER 30 SECONDS => Turn Display OFF
  if ((millis() - displayCountStart) > DisplaySleepDelay){
    display->PowerSaveOn(true);
  }

  // WiFi ON/OFF
  if (btnMulti1){
    // display->PowerSaveOn(false);
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
          delay(2000);
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
  if (btnMulti2){
    // display->PowerSaveOn(false);
    btnMulti2 = false;
    switch (nClicks2)
    {
    case 3:
      if (!isBluetoothEnabled){
        if(isWiFiConnected){
          network->WiFiDisconnect();
          Serial.println("BLUETOOTH ON");
          bluetooth->initBluetooth();
        } else{
           bluetooth->initBluetooth();
        }
      } 
      else {
        bluetooth->BTDisconnect();     
        ESP.restart();
      }
      break;
    case 5:
      Serial.println("Uploading Queue to Firestore");
      uploadFirestoreQueue();
      break;
    }
    nClicks2 = 0;
  }

  // RESTART
  if (btnMulti3){
    btnMulti3 = false;
    switch (nClicks3)
    {
    case 3:
      ESP.restart();
      break;
    }
    nClicks3 = 0;
  }

  // TURN DEEP SLEEP ON
  if(btnLongPress3){
    // display->PowerSaveOn(false);
    btnLongPress3 = false;
    if (pressDuration3 == 5){
      pressDuration3 = 0;
      display->PowerOFF();
      delay(2000);
      display->PowerSaveOn(true);
      esp_sleep_enable_timer_wakeup(30 * 1000000);
      esp_deep_sleep_start();
    }
  }

  // SHOW DEVICE HUMIDITY AND TEMPERATURE
  if (btnDouble1){
    btnDouble1 = false;
    // display->PowerSaveOn(false);
    display->HumTemp(humidity, temperature);
    delay(3000);
    display->PowerSaveOn(false);
  }

  // if (btnClick1 == true){
  //   x++;
  //   if (x > 9) x=1;
  // } btnClick1 = false;
  // switch (x)
  // {
  // case 1:
 
  //   break;
  // case 2:

  //   break;
  // case 3:

  //   break;
  // case 4:
  
  //   break;
  // case 5:
 
  //   break;
  // case 6:

  //   break;
  // case 7:

  //   break;
  // case 8:

  //   break;
  // case 9:

  //   break;
  // }
}


