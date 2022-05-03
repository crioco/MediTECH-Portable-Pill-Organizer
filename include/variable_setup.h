#ifndef VARIABLESETUP_H
#define VARIABLESETUP_H 

#include <definition_libraries.h>

int GMT_OFFSET = 28800;
int DAYLIGHT_OFFSET = 0;
const char* NTP_SERVER = "ph.pool.ntp.org";

RTC_DS3231 rtc;
// AHTxx aht(AHTXX_ADDRESS_X38, AHT1x_SENSOR);

Adafruit_AHTX0 aht;

Network *network;
Display *display;
Bluetooth *bluetooth;

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

#define tempTH 35 // Temoerature Threshold in Celcius
#define humdTH 75 // Humidity Threshold in Percent

// {{Position Slot 1, Dosage}, {Position Slot 2, Dosage}}}
std::vector<std::vector<int>> LEDVec;

// ALARM ----------------------------------------------------------------------------------------------------------------------

int ringDuration = 60000; // Duration of alarm in milliseconds (1 min) 60000
int snoozeDuration = 60000; // Interval between Alarms after snooze in milliseconds (1 min) 60000
int snoozeAmount = 2; // Number of snoozes

// LED Rows -------------------------------------------------------------------------------------------------------------------
extern const int LEDa = 32; // RED
extern const int LEDb = 33; // WHITE
extern const int LEDc = 25; // YELLOW
extern const int LEDd = 26; // GREEN (BLACK)
extern const int LEDe = 27; // BLUE

// LED Columns ----------------------------------------------------------------------------------------------------------------
extern const int LED1 = 0; // BLUE
extern const int LED2 = 4; // YELLOW 
extern const int LED3 = 16; // WHITE
extern const int LED4 = 17; // RED

#endif