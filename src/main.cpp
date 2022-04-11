#include <definition_libraries.h>
#include <functions.h>

// REMINDER: Troubleshooting indicators, ex. Display message Wi-Fi not available
  // RTC not working
  // Wi-Fi & Bluetooth not connected/unavailable

TaskHandle_t loop_2_t;

OneButton button1 (button1_pin, true);
OneButton button2 (button2_pin, true);

extern Display *display;
extern Network *network;

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

extern int nClicks1;
extern int nClicks2;

extern bool isWiFiEnabled;
extern bool isWiFiConnected;
extern bool isBluetoothEnabled;

extern int buzzStartMillis;
extern bool alarmSoundOn;
extern bool alarmOn;

bool isDeviceOpen = false;
bool LedMatrixOn = false;
extern std::vector<std::vector<int>> LEDVec;

// -----------------------------------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> VEC = {{1, 3}, {2, 0}, {3, 1}, {4, 2}, {5, 4}}; // FOR TESTING

void loop_2(void * pvParameters){

  //Loop
  for(;;){
    // Checks button 1 & 2 Status
    button1.tick();
    button2.tick();

    // Alarm Sound
    // if (alarmSoundOn){
    //   buzzStartMillis = millis();
    //   while(alarmSoundOn){
    //     alarmSound();
    //     button1.tick();
    //     button2.tick();
    //   } 
    //   digitalWrite(buzzerPin, LOW);
    // }

    // Display Led Matrix
    if (LedMatrixOn){
      displayLED(LEDVec);
    }

    // Serial.print("Core: "); Serial.println(xPortGetCoreID());
  }
}

void setup() {
  
  Serial.begin(115200);

  Wire.begin(); // Initialize default I2C pins (GPIO 21 & 22) 
  Wire1.begin(SDA_2, SCL_2); // Initialize second I2C pins (GPIO 19 & 18)

  pinMode(buzzerPin, OUTPUT);
  // pinMode(vib_pin, OUTPUT);

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
  pinMode(REEDSwitchPin, INPUT_PULLDOWN);

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
  
  // ---------------------------------------------------
  
  display->initDisplay();
  network->initWiFi();
  initRTC();
  initAHT();
  loadEEPROM();

  // ---------------------------------------------------

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
}

int x = 1;

void loop() {

  checkAlarm();
  display->displayTime();
  // checkHumTemp();

  // int SampleSize = 64;
  // int ADCvalue = 0;
  // for (int i = 0; i < SampleSize; i++){
  //   ADCvalue += analogRead(LiPoLevelPin);  
  // }
  // ADCvalue /= SampleSize;
  // float voltage = (ADCvalue * 1.9)/2047;

  // int batteryPercent = map(ADCvalue, 0, 2047, 0, 100);
  
  // Serial.print("ADC: "); Serial.println(ADCvalue); 
  // Serial.print("Voltage: "); Serial.println(voltage);
  // Serial.printf("Battery: %d %% \n", batteryPercent);
  // delay(1000);

  // Checks Battery Level in Percentage (3.1 V - 4.2 V)
  // checkBattery();

  // WiFi ON/OFF
  if (btnMulti1 == true){
    btnMulti1 = false;
    switch (nClicks1)
    {
    case 3:
      if (isWiFiConnected){
        network->WiFiDisconnect();
      } else initNetwork();
      break;
    // case 4:
    //   network->firestoreDataUpdate();
    //   break;
    } 
    nClicks1 = 0;  
  }

  delay(1000);
  
  // button1.tick();
  // button2.tick();

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


