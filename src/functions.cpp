#include <variable_setup.h>
#include <functions.h>
#include "time.h"

using std::vector; 
using std::find;

extern bool isWiFiConnected;
bool alarmSoundOn;
bool RTCFound;
bool AHTFound;
int batteryLevel;

extern bool btnClick1;
extern bool btnDouble1;
extern bool btnLongStart1;
extern bool btnLongPress1;
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

extern int pressDuration1;
extern int pressDuration2;

extern bool isDeviceOpen;
extern bool LedMatrixOn;
extern int displayCountStart;

int buzzStartMillis = 0;
int HumTempStartMillis = 0;

int buzzCurrentMillis = 0;

float temperature;
float humidity;

// Initializes the components and variables

// Initialize D3231 RTC. I2C Address 0x68
void initRTC(){
  if (!rtc.begin()){ 
    Serial.println("[Failed] Coudn't Find RTC");
    RTCFound = false;
    Serial.flush();
    while(1) delay(10);
  } 
  Serial.println("[Success] RTC Found");
  RTCFound = true;

  // // // Adjust Date and Time of the RTC. If the Pill Organizer is turned off, 
  // // // set the date and time from the RTC on setup once powered.
  if (rtc.lostPower()){
     rtc.adjust(DateTime(__DATE__, __TIME__));
  }
  // // Adjust the Date and Time. Only Run once.
  // rtc.adjust(DateTime(__DATE__, __TIME__));

  // // clear /EOSC bit to ensure that the clock runs on battery power. Atleast Run once.
  // Wire.beginTransmission(0x68); // address DS3231
  // Wire.write(0x0E); // select register
  // Wire.write(0b00011100); // write register bitmap, bit 7 is /EOSC
  // Wire.endTransmission();

}

// NETWORK/FIREBASE -------------------------------------------------------------------------------------------------------------

void setNTP(){
  if (SYSTEM_EVENT_STA_GOT_IP){
    time_t now_t;
    #define TIMESTAMP_1_1_2021 1609459200 // Unix Timestamp of 1/1/2021

    configTime(GMT_OFFSET, DAYLIGHT_OFFSET, NTP_SERVER);
    Serial.println("Updating Date & Time from NTP Server.");

    while (now_t < TIMESTAMP_1_1_2021){
      now_t = time(nullptr);
      display->NTPConnecting();
    } delay(2000);
    
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)){
        Serial.println("[Failed] Date/Time not updated");
        display->NTPNotConnected();
        delay(2000);
        return;
    }
    display->NTPConnected();

    Serial.println("[Success] Date & Time updated");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));

    delay(2000);
  }
}

// Initialize Network & Firebase Connection
void initNetwork(){
  network = new Network();
  network->WiFiConnect();
  if(WiFi.status() == WL_CONNECTED){
    network->firebaseInit();
    setNTP();
    uploadFirestoreQueue();
  }
}

// --------------------------------------------------------------------------------------------------------------------------

// Returns a vector<int> of Keys of a map<int, int>
vector<int> getKeys(std::map<int, int> alarmList){
  vector<int> keys;
  for(std::map<int, int>::iterator it = alarmList.begin(); it != alarmList.end(); ++it){
    keys.push_back(it->first);
  }
  return keys;
}

// Returns a vector<int> of Values of a map<int, int>
vector<int> getValues(std::map<int, int> alarmList){
  vector<int> values;
  for(std::map<int, int>::iterator it = alarmList.begin(); it != alarmList.end(); ++it){
    values.push_back(it->second);
  }
  return values;
}

// ALARM -------------------------------------------------------------------------------------------------------------------

int snoozed = 0; // Amount of snoozes occured
bool alarmOn = false; // boolean if an alarm has been triggered
DateTime previousAlarmTime; // DateTime of previous alarm, prevent alarms from activating after it was stopped already
std::pair<int, vector<std::pair<String, int>>> currentAlarms; // pair <Time, vector<pair <Pill Name, Dosage>>>
DateTime listedAlarmTime; // Time of alarm according to pill intake schedule
int alarmTime = 0; // Time of alarm (including time of alarm after snooze)

// Checks for Alarm for current Day and Time
void checkAlarm(){
  
  DateTime now = rtc.now();
  int timeNow = (now.hour() * 100) + now.minute(); // Current Time in the formart of (e.g. 1930 = 19:30 or 7:30 PM)
  int previousMillis = millis();
  // int currentMillis = millis();
  int alarmState; // 1 = Taken; 2 = Skipped; 3 = Missed

  if (!alarmOn){
    // Iterates each Pill in PillList to check if there is an Alarm at the current day & time

    vector<std::pair<String, int>> vectorPairs;
    for(vector<Pill>::iterator it = PillList.begin(); it != PillList.end(); ++it){

      // Check if day == alarm
      if (find(it->days.begin(), it->days.end(), now.dayOfTheWeek()) != it->days.end()){

        // Check if time == to alarm
        if (it->alarmList.find(timeNow) != it->alarmList.end() && (previousAlarmTime.hour() != now.hour()
            || previousAlarmTime.minute() != now.minute() || previousAlarmTime.day() != now.day()) ){

          alarmTime = timeNow; // alarmTime to current time
          alarmOn = true;

          // Add position & dosage to LEDVec vector
          int slot = it->container_slot;
          int dosage = it->alarmList.find(timeNow)->second;
          LEDVec.push_back({slot, dosage});

          // Vector of pairs with Pill Names and corresponding dosage
          vectorPairs.push_back(std::make_pair(it->med_name, dosage));
        }
      }
    }
    if (alarmOn){
      display->PowerSaveOn(false); // Turn On Display
      currentAlarms = std::make_pair(timeNow, vectorPairs);
      listedAlarmTime = previousAlarmTime = now; // DateTime of previous alarm (just activated) to DateTime now
    } 
  }
  
  if (alarmTime == timeNow && alarmOn == true){

    // Print Current Alarm (Pill Names, Dosage, Time)
    Serial.print("Time: "); Serial.println(currentAlarms.first);
    for (vector<std::pair<String,int>>::iterator it = currentAlarms.second.begin(); it != currentAlarms.second.end(); ++it){
      Serial.printf("Name: %s - Dosage: %d\n", it->first.c_str(), it->second);
    }

    // Alarm
    while (millis() - previousMillis < ringDuration){

      display->displayAlarm(now);
      alarmSoundOn = true;
      // Stop Alarm. BTN 1&2 3 Clicks
      if (btnMulti1 || btnMulti2){
        btnMulti1 = btnMulti2 = false;
        if (nClicks1 == 3 || nClicks2 == 3){
          nClicks1 = nClicks2 = 0;
          Serial.println("[Alarm Stopped]");
          alarmOn = false;
          display->AlarmStopped();
          break;
        }
      }

      // Snooze Alarm
      if (btnDouble1 || btnDouble2){
        btnDouble1 = btnDouble2 = false;
        Serial.println("[Alarm Snoozed]");
        display->AlarmSnooze();
        break;
      }  
    }

    alarmSoundOn = false;
    digitalWrite(vib_pin, LOW);
    delay(2000);

    // Snoozed
    if (alarmOn){
      now = rtc.now() + TimeSpan(0, 0, snoozeDuration/60000, 0);
      alarmTime = (now.hour() * 100) + now.minute();
      previousAlarmTime = now;
      display->AlarmSnooze();
      delay(2000);
      // Display Next Alarm
      Serial.print("Next Alarm: "); Serial.println(previousAlarmTime.timestamp());
      snoozed++;
    }
    // Snoozed Limit Reached (Alarm Missed)
    if (snoozed > snoozeAmount){
      alarmOn = false;
      // Firebase (Missed)
      Serial.println("[Missed]");
      alarmState = 3; // "Missed"
      display->AlarmStatus(alarmState);
      delay(2000);
      if(!network->firestoreDataUpdate(listedAlarmTime.unixtime(), previousAlarmTime.unixtime(), currentAlarms.second, alarmState))
        addFirestoreQueue(listedAlarmTime.unixtime(), previousAlarmTime.unixtime(),currentAlarms.second, alarmState);
      alarmTime = alarmState = snoozed = 0;
      LEDVec.clear();
      storePrevAlarm(previousAlarmTime.unixtime());
      return;
    }

    // Alarm Stopped | Display Led Matrix (Pill Dosages)
    if (!alarmOn){
      while (true){
        // Display Led Matrix when device is open
        if (digitalRead(REEDSwitchPin) == LOW) {
          LedMatrixOn = true;
        }else{
          LedMatrixOn = false;
        };
        
        display->AlarmTakePills();

        // Take Pills
        if (btnMulti1 || btnMulti2){
          btnMulti1 = btnMulti2 = false;
          if (nClicks1 == 3 || nClicks2 == 3){
            nClicks1 = nClicks2 = 0;
            Serial.println("[Taken]");
            alarmState = 1; // "Taken"
            display->AlarmStatus(alarmState);
            delay(2000);
            if(!network->firestoreDataUpdate(listedAlarmTime.unixtime(), previousAlarmTime.unixtime(), currentAlarms.second, alarmState))
              addFirestoreQueue(listedAlarmTime.unixtime(), previousAlarmTime.unixtime(),currentAlarms.second, alarmState);
            alarmTime = alarmState = snoozed = 0;
            LEDVec.clear();
            break;
          }
        }

        // Skip Pills
        if (btnLongPress1 || btnLongPress2){
          btnLongPress1 = btnLongPress2 = false;
          if (pressDuration1 >= 4 || pressDuration2 >= 4){
            pressDuration1 = pressDuration2 = 0;
            Serial.println("[Skipped]");
            alarmState = 2; // "Skipped"
            display->AlarmStatus(alarmState);
            delay(2000);
            if(!network->firestoreDataUpdate(listedAlarmTime.unixtime(), previousAlarmTime.unixtime(), currentAlarms.second, alarmState))
              addFirestoreQueue(listedAlarmTime.unixtime(), previousAlarmTime.unixtime(),currentAlarms.second, alarmState);
            alarmTime = alarmState = snoozed = 0;
            LEDVec.clear();
            break;
          }
        }
      }
      LedMatrixOn = false;
      display->PowerSaveOn(false);
      displayCountStart = millis(); // Turn on Display
      storePrevAlarm(previousAlarmTime.unixtime());
    }
  }
}

// HUMIDITY & TEMPERATURE ----------------------------------------------------------------------------------------------------

// Initialize AHT10 Temp/Humd Sensor. I2C Address 0x38
void initAHT(){
  if (aht.begin(&Wire1)) {
    Serial.println("[Success] AHT10 Found");
    AHTFound = true;
  } else {
    Serial.println("[Failed] AHT10 Not Found");
    AHTFound = false;
  }
}

// Check Temp & Humidity
void checkHumTemp(){

  if(AHTFound){
    if (millis() - HumTempStartMillis > 5000){
      sensors_event_t humid, temp;
      aht.getEvent(&humid, &temp);

      temperature = temp.temperature;
      humidity = humid.relative_humidity;

      while (humidity >= humdTH){
        display->HumTempWarning(humidity, temperature, 1);
      }

      while (temperature >= tempTH){
        display->HumTempWarning(humidity, temperature, 2);
      }
      HumTempStartMillis = millis();
    } 
  }
}

// LED DISPLAY ---------------------------------------------------------------------------------------------------------------------------------------------------

int LEDcol[] = {LED1, LED2, LED3, LED4}; // Array of LED pins indicating the number of dosage.
int LEDrow[] = {LEDa, LEDb, LEDc, LEDd, LEDe}; // Array of LED pins indicating the pill to be taken by its position on the device.

// Map the container number to the corresponding pin number for the LED multiplex.
std::map<int, int> LEDmap = {{1, LEDa}, {2, LEDb}, {3, LEDc}, {4, LEDd}, {5, LEDe}};

// Displays the pills and dosage using LED matrix, argument of 2D vector with the pill position and dosage. LEDVec = {{Position Slot, Dosage}}
void displayLED (vector<vector<int>>LEDVec){
  for (int i = 0; i < LEDVec.size(); i++){
    digitalWrite(LEDmap[LEDVec[i][0]], HIGH); // Turns ON LED Row
    for (int j = 0; j < LEDVec[i][1]; j++) digitalWrite(LEDcol[j], HIGH); // Turns ON LED columns (1-4)
    delay(1);
    for (int j = 0; j < LEDVec[i][1]; j++) digitalWrite(LEDcol[j], LOW); // Turns OFF LED columns (1-4)
    digitalWrite(LEDmap[LEDVec[i][0]], LOW); // Turns OFF LED Row
  }
}

int alarmSoundCounter = 0;
void alarmSound(){
  buzzCurrentMillis = millis();
  digitalWrite(vib_pin, HIGH);
  if (buzzCurrentMillis - buzzStartMillis >= 50 && alarmSoundCounter == 0){
    digitalWrite(buzzerPin, HIGH);
    alarmSoundCounter += 1;
  }
  if (buzzCurrentMillis - buzzStartMillis >= 100 && alarmSoundCounter == 1){
    digitalWrite(buzzerPin, LOW);
    alarmSoundCounter += 1;
  }
  if (buzzCurrentMillis - buzzStartMillis >= 200 && alarmSoundCounter == 2){
    digitalWrite(buzzerPin, HIGH);
    alarmSoundCounter += 1;
  }
  if (buzzCurrentMillis - buzzStartMillis >= 250 && alarmSoundCounter == 3){
    digitalWrite(buzzerPin, LOW);
    alarmSoundCounter += 1;
  }
  if (buzzCurrentMillis - buzzStartMillis >= 350 && alarmSoundCounter == 4){
    digitalWrite(buzzerPin, HIGH);
    alarmSoundCounter += 1;
  }
  if (buzzCurrentMillis - buzzStartMillis >= 400 && alarmSoundCounter == 5){
    digitalWrite(buzzerPin, LOW);
    alarmSoundCounter += 1;
  }
  if (buzzCurrentMillis - buzzStartMillis >= 900 && alarmSoundCounter == 6){
    digitalWrite(vib_pin, LOW);
    buzzStartMillis = millis();
    alarmSoundCounter = 0;
  }
}

// void alarmSound(){
//   digitalWrite(buzzerPin, HIGH);
//   delay(50);
//   digitalWrite(buzzerPin, LOW);
//   delay(100);
//   digitalWrite(buzzerPin, HIGH);
//   delay(50);
//   digitalWrite(buzzerPin, LOW);
//   delay(100);
//   digitalWrite(buzzerPin, HIGH);
//   delay(50);
//   digitalWrite(buzzerPin, LOW);
//   delay(500);
// }

#define SampleSize 64
int batteryPercent = 0;
int checkBatteryStart = 0;

void checkBattery(){
  if (millis() - checkBatteryStart > 10000){
  int ADCvalue = 0;
  for (int i = 0; i < SampleSize; i++){
    ADCvalue += analogRead(LiPoLevelPin);  
  }
  ADCvalue /= SampleSize;
  float voltage = (ADCvalue * 1.9)/2047;

  // Change Mapping ADCValue 1650 (3.4V / 1.53V) & 2023 (4.12V / 1.85V) to 0%-100%
  batteryPercent = map(ADCvalue, 1650, 2025, 0, 100);

  batteryLevel = batteryPercent;
  if (batteryLevel > 100) batteryLevel = 100;
  else if (batteryLevel < 0) batteryLevel = 0;

  // if (batteryLevel < 5){
  //   display->LowBattery();
  //   Serial.println("LOW BATTERY.");
  //   delay(2000);
  //   display->PowerOFF();
  //   Serial.println("POWERING OFF...");
  //   delay(3000);
  //   display->PowerSaveOn(true);
  //   esp_deep_sleep_start();
  // }
  checkBatteryStart = millis();
  }
}

void displayPillList(){
    for(vector<Pill>::iterator it = PillList.begin(); it != PillList.end(); ++it){

      Serial.println("Pill Name: " + it->med_name);
      Serial.print("Container Slot: "); Serial.println(it->container_slot);

      // Check if day == alarm
      Serial.println("Alarm List: ");
      for(std::map<int, int>::iterator it1 = it->alarmList.begin(); it1 != it->alarmList.end(); ++it1){
        Serial.print(it1->first); Serial.print(" : "); Serial.println(it1->second);
      }
      Serial.println("Days: ");
      
      for (int x = 0; x < it->days.size(); x++){
        Serial.print(it->days[x]);
      }
      Serial.println();
    }
}