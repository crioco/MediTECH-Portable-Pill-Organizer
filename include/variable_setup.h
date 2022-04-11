#ifndef VARIABLESETUP_H
#define VARIABLESETUP_H 

#include <definition_libraries.h>

int GMT_OFFSET = 28800;
int DAYLIGHT_OFFSET = 0;
const char* NTP_SERVER = "ph.pool.ntp.org";

RTC_DS3231 rtc;
Adafruit_AHTX0 aht;

Network *network;
Display *display;

// String hour, minute, second;

char dayOfWeek[7][4] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
char months[12][4] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};

// Pill OBJECT ---------------------------------------------------------------------------------------------------------------

struct Pill {
    String med_name;
    std::vector<int> days;
    std::map<int, int> alarmList;
    int container_slot;

    Pill(String med_name, std::vector<int> days, std::map<int, int> alarmList, int container_slot){
        this->med_name = med_name;
        this->days = days;
        this->alarmList = alarmList;
        this->container_slot = container_slot;
    }
};

std::vector<Pill> PillList;

// TEMP & HUMIDITY -----------------------------------------------------------------------------------------------------------

bool THSensor_ON = false;
#define tempTH 0 // Temoerature Threshold
#define humdTH 0 // Humidity Threshold

// {{Position Slot 1, Dosage}, {Position Slot 2, Dosage}}}
std::vector<std::vector<int>> LEDVec;

// ALARM ----------------------------------------------------------------------------------------------------------------------

int ringDuration = 60000; // Duration of alarm in milliseconds (5 min) 300000
int snoozeDuration = 60000; // Interval between Alarms after snooze in milliseconds (10 min) 600000
int snoozeAmount = 1; // Number of snoozes

// LED Rows -------------------------------------------------------------------------------------------------------------------
extern const int LEDa = 32; // 2
extern const int LEDb = 33; // 14
extern const int LEDc = 25; // 12
extern const int LEDd = 26; // 13
extern const int LEDe = 27; // 15

// LED Columns ----------------------------------------------------------------------------------------------------------------
extern const int LED1 = 16; // 0
extern const int LED2 = 4; // 25
extern const int LED3 = 0; // 33
extern const int LED4 = 2; // 32

#endif