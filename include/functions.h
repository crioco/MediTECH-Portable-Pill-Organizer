#ifndef FUNCTIONS_H 
#define FUNCTIONS_H 

#define SDA_1 21 // ORANGE
#define SCL_1 22 // GREEN

#define SDA_2 19 // ORANGE
#define SCL_2 18 // GREEN

#define buzzerPin 2
#define vib_pin 0

#define button1_pin 23
#define button2_pin 3
#define button3_pin 15

#define LiPoLevelPin 34
#define REEDSwitchPin 5

#define DisplaySleepDelay 30000 // in milliseconds

void initRTC();
void initAHT();
void initDisplay();
void initNetwork();
void loadEEPROM();
void displayTime();
void checkAlarm();
void checkHumTemp();
void displayLED(std::vector<std::vector<int>> LEDVec);
std::vector<int> getKeys(std::map<int, int> alarmList); // Returns a vector<int> of Keys of a map<int, int>
std::vector<int> getValues(std::map<int, int> alarmList); // Returns a vector<int> of Values of a map<int, int>
void alarmSound();
void setNTP();
void displayTest();
void checkBattery();

#endif // ends the if statement #ifndef