#include <spiffs_storage.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <map>

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

extern std::vector<Pill> PillList;

void initSPIFFS(){
    if(!SPIFFS.begin(true)){
        Serial.println("[FAILED] ERROR IN INITIALIZING SPIFFS");   
    }
}

void listAllFiles(){
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while(file){
        Serial.print("File: "); Serial.println(file.name());
        file = root.openNextFile();
    }
    root.close();
    file.close();

}

// JSON to Pill Object
void readDataJSON(){
    String pills[] = {"pill_1", "pill_2", "pill_3", "pill_4", "pill_5"};

    File file = SPIFFS.open("/data_storage.json");
    if(file){
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, file);

        for (int i = 0; i < 5; i++){
            String pill_name = doc[pills[i]]["pill_name"];
            int container_slot = doc[pills[i]]["container_slot"];
            // int[] days = doc[pills[i]]["days"];
            Serial.println(pill_name);
        }

        
        
    }
    file.close();
}

// JSON from App to .json file 
void writeDataJSON(){
    File outfile = SPIFFS.open("/data_storage.json", "w");
    StaticJsonDocument<200> doc;
    doc["pill_1"] = "value";

    // PillList
    JsonObject alarmObj = doc.createNestedObject("alarmList");
    //Iterate true alarmList
    alarmObj["time1"] = 1 ;// dosage
    alarmObj["time2"] = 2; 
    serializeJson(doc, outfile);

    outfile.close();

}