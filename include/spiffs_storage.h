#ifndef SPIFFS_STORAGE_H
#define SPIFFS_STORAGE_H 

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
#include <RTClib.h>
#include <network.h>

void initSPIFFS();
void listAllFiles();
void getPillListfromJSON();
void readDataStorageJSON();
boolean writeDataStorageJSON(String json);
boolean addFirestoreQueue(int alarmTimeUnix, int takenTimeUnix, std::vector<std::pair<String, int>> alarmPills, int alarmState);
void readFirestoreQueue();
boolean uploadFirestoreQueue();
boolean updateWiFiConfig(String json);
boolean updateAlarmConfig(String json);
void loadConfigJSON();
boolean storePrevAlarm(int prevAlarmUnix);

#endif