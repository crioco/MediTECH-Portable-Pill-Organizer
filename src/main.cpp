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

extern bool AHTFound;
extern float temperature;
extern float humidity;
extern bool OLEDFound;

// RTC_DATA_ATTR int bootCount = 0;

// -----------------------------------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> VEC = {{1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}}; // FOR TESTING

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
      digitalWrite(vib_pin, LOW);
      alarmSoundCounter = 0;
    }

    // Display Led Matrix
    if (LedMatrixOn){
      displayLED(LEDVec);
    }

    // RESTART
    if (btnMulti3){
      btnMulti3 = false;
      switch (nClicks3)
      {
      case 4:
        ESP.restart();
        break;
      }
      nClicks3 = 0;
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

  pinMode(buzzerPin, OUTPUT);
  // pinMode(vib_pin, OUTPUT);

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
  checkBattery();
  display->initDisplay();
  getPillListfromJSON();
  loadConfigJSON();

  // ---------------------------------------------------

  // If restarted after WiFi on while BLE on
  EEPROM.begin(1);

  if (EEPROM.read(0) == 1){
    EEPROM.write(0, 0);
    EEPROM.commit();
    initNetwork();
  } else if (EEPROM.read(0) == 2){
    EEPROM.write(0, 0);
    EEPROM.commit();
    bluetooth->initBluetooth();
  }

  // Turn on Display
  if(OLEDFound) display->PowerSaveOn(false);
}
// int x = 1;
void loop() {

  if (OLEDFound) {
    // Checks Battery Level in Percentage (3.3 V - 4.11 V)
    checkBattery();

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
      btnMulti1 = false;
      switch (nClicks1)
      {
      case 3:
        if (isWiFiConnected){
          network->WiFiDisconnect();
        } else {
            initNetwork();   
          }
          break;
      }
        
      nClicks1 = 0;  
    }

    // BLUETOOTH ON/OFF
    if (btnMulti2){
      btnMulti2 = false;
      switch (nClicks2)
      {
      case 3:
        if (!isBluetoothEnabled){
          if(isWiFiConnected){
            EEPROM.write(0, 2);
            EEPROM.commit();
            ESP.restart();
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

    // SHOW DEVICE HUMIDITY AND TEMPERATURE
    if (btnDouble1 || btnDouble2){
      btnDouble1 = false; btnDouble2 =false;
      display->HumTemp(humidity, temperature);
      delay(3000);
      display->PowerSaveOn(false);
    }

  }

  // TURN DEEP SLEEP ON
  if(btnLongPress3){
    btnLongPress3 = false;
    if (pressDuration3 == 5){
      pressDuration3 = 0;
      display->PowerOFF();
      delay(2000);
      display->PowerSaveOn(true);
      delay(2000);
      esp_sleep_enable_timer_wakeup(30 * 1000000);
      esp_deep_sleep_start();
    }
  }

  // if (btnClick1 == true){
  //   btnClick1 = false;
  //   x++;
  //   if (x > 41) x=1;
  // } 
  // switch (x)
  // {
  //   case 1:
  //     display->AlarmSnooze();
  //     break;
  //   case 2:
  //     display->ComponentStatus();
  //     break;
  //   case 3:
  //     display->AlarmStopped();
  //     break;
  //   case 4:
  //     display->AlarmTakePills();
  //     break;
  //   case 5:
  //     display->BluetoothDisabled();
  //     break;
  //     case 6:
  //     display->BluetoothEnabled();
  //     break;
  //     case 8:
  //     display->BTAuthorized();
  //     break;
  //     case 9:
  //     display->BTClientConnect();
  //     break;
  //     case 10:
  //     display->BTClientDisconnect();
  //     break;
  //     case 11:
  //     display->BTDenied();
  //     break;
  //     case 12:
  //     display->DeviceSettingsUpdate(true);
  //     break;
  //     case 13:
  //     display->DeviceSettingsUpdate(true);
  //     break;
  //     case 14:
  //     display->FirebaseConnected();
  //     break;
  //     case 15:
  //     display->FirebaseConnecting();
  //     break;
  //     case 16:
  //     display->FirebaseNotConnected();
  //     break;
  //     case 17:
  //     display->FirebaseUpdated(true);
  //     break;
  //     case 18:
  //     display->FirebaseUpdated(false);
  //     break;
  //     case 19:
  //     display->FirebaseUploading();
  //     break;
  //     case 20:
  //     display->FromReset();
  //     break;
  //     case 21:
  //     display->LowBattery();
  //     break;
  //     case 22:
  //     display->NTPConnected();
  //     break;
  //     case 23:
  //     display->NTPConnecting();
  //     break;
  //     case 24:
  //     display->NTPNotConnected();
  //     break;
  //     case 25:
  //     display->PillSettingsUpdate(true);
  //     break;
  //     case 26:
  //     display->PillSettingsUpdate(false);
  //     break;
  //     case 27:
  //     display->PowerOFF();
  //     break;
  //     case 28:
  //     display->WiFiConnected();
  //     break;
  //     case 29:
  //     display->WiFiConnecting();
  //     break;
  //     case 30:
  //     display->WiFiDisabled();
  //     break;
  //     case 31:
  //     display->WiFiDisconnected();
  //     break;
  //     case 32:
  //     display->WiFiEnabled();
  //     break;
  //     case 33:
  //     display->WiFiNotConnected();
  //     break;
  //     case 34:
  //     display->WiFiReconnecting();
  //     break;
  //     case 35:
  //     display->WiFiSettingsUpdate(true);
  //     break;
  //     case 36:
  //     display->WiFiSettingsUpdate(false);
  //     break;
  //     case 37:
  //     display->AlarmStatus(1);
  //     break;
  //     case 38:
  //     display->AlarmStatus(2);
  //     break;
  //     case 39:
  //     display->AlarmStatus(3);
  //     break;
  //     case 40:
  //     display->HumTempWarning(60, 35, 2);
  //     break;
  //     case 41:
  //     display->NextAlarm("3:00 PM", false);
  //     break;
  // }
}


