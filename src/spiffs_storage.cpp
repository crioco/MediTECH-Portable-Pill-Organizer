#include <spiffs_storage.h>

extern Network *network;
extern int ringDuration;
extern int snoozeDuration; 
extern int snoozeAmount;
extern char* WIFI_SSID;
extern char* WIFI_PASSWORD; 

struct Pill
{
    String med_name;
    std::vector<int> days;
    std::map<int, int> alarmList;
    int container_slot;

    Pill(String med_name, std::vector<int> days, std::map<int, int> alarmList, int container_slot)
    {
        this->med_name = med_name;
        this->days = days;
        this->alarmList = alarmList;
        this->container_slot = container_slot;
    }
};

extern std::vector<Pill> PillList;

void initSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("[FAILED] ERROR IN INITIALIZING SPIFFS");
    }
}

void listAllFiles()
{
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    while (file)
    {
        Serial.print("File: ");
        Serial.println(file.name());
        file = root.openNextFile();
    }
    root.close();
    file.close();
}

// JSON to Pill Object
void getPillListfromJSON()
{
    File file = SPIFFS.open("/data_storage.json");
    if (file)
    {
        StaticJsonDocument<1700> doc;
        DeserializationError error = deserializeJson(doc, file);

        if (error){
            Serial.print("ERROR Coverting JSON to Pill Object: ");
            Serial.println(error.c_str());
        }
        else{
            PillList.clear();
            JsonArray array = doc.as<JsonArray>();
            for (JsonObject obj : array)
            {
                String pillName = obj["pill_name"];
                int container_slot = obj["container_slot"];

                JsonArray days = obj["days"];
                std::vector<int> daysVector;
                for (int day : days)
                {
                    daysVector.push_back(day);
                }

                std::map<int, int> alarmMap;
                JsonArray alarmList = obj["alarmList"];
                for (JsonObject alarmObj : alarmList){
                    alarmMap.insert({alarmObj["time"].as<int>(), alarmObj["dosage"].as<int>()});
                }
                PillList.push_back(Pill(pillName, daysVector, alarmMap, container_slot));
            }
            Serial.println("Pill List Updated.");
        }
    }
    file.close();
}

void readDataStorageJSON(){
    File file = SPIFFS.open("/data_storage.json");
    if (file){
        StaticJsonDocument<1700> doc;
        DeserializationError error = deserializeJson(doc, file);

        if (error)
        {
            Serial.print("ERROR reading data_storage.json: ");
            Serial.println(error.c_str());
        }
        JsonArray array = doc.as<JsonArray>();
        Serial.println("data_storage.json");
        for (JsonObject obj : array)
        {
            String pillName = obj["pill_name"];
            int container_slot = obj["container_slot"];

            JsonArray days = obj["days"];
            std::vector<int> daysVector;
            for (int day : days)
            {
                daysVector.push_back(day);
            }

            std::map<int, int> alarmMap;
            JsonArray alarmList = obj["alarmList"];
            for (JsonObject alarmObj : alarmList)
            {
                alarmMap.insert({alarmObj["time"].as<int>(), alarmObj["dosage"].as<int>()});
            }

            Serial.print("Pill Name: "); Serial.println(pillName);
            Serial.print("Days: ");
            for (int x : daysVector){
                Serial.print(String(x) + " ");
            }
            Serial.println();
            Serial.print("Container: "); Serial.println(container_slot);
            Serial.println("Alarm List: ");
            for (std::map<int,int>::iterator it = alarmMap.begin(); it!= alarmMap.end(); ++it){
                Serial.print(it->first); Serial.print(" : ");
                Serial.println(it->second);
            }    
        }
        Serial.print("PillList Size: "); Serial.println(PillList.size());
    }
    file.close();
}

boolean writeDataStorageJSON(String json){
    bool result = false;
    File file = SPIFFS.open("/data_storage.json", "r");
    
    DynamicJsonDocument doc(1700);
    DeserializationError error = deserializeJson(doc, json);
    if (error){
        Serial.println("ERROR: Failed to get data from Bluetooth json.");
        file.close();
        return false;
    }
    file.close();

    file = SPIFFS.open("/data_storage.json", "w");
    String pill_name = doc[0]["pill_name"];
    Serial.println(pill_name);
    if (serializeJsonPretty(doc, file) && !doc.isNull()){
        Serial.println("data_storage.json Overwritten.");
        delay(2000);
        result = true;
    } else {
        Serial.println("ERROR: data_storage.json unable to be overwritten.");
        result = false;
    }
    file.close();
    return result;
}

boolean addFirestoreQueue(int alarmTimeUnix, int takenTimeUnix, std::vector<std::pair<String, int>> alarmPills, int alarmState){
    File outFile = SPIFFS.open("/firestore_update.json", "r");
    StaticJsonDocument<1512> doc;
    DeserializationError error = deserializeJson(doc, outFile);
    JsonArray docArray;
    if (error){
        if (error.code() == DeserializationError::EmptyInput){
            docArray = doc.to<JsonArray>();
        }else{
             Serial.print("ERROR in opening firestore_update.json: ");
            Serial.println(error.c_str());
            return false;
        }
    }else{
        docArray = doc.as<JsonArray>();
    }
    
    JsonObject obj = docArray.createNestedObject();

    // StaticJsonDocument<512> docObject;
    // JsonObject obj = docObject.to<JsonObject>();
    obj["alarmState"] = alarmState;
    obj["alarmTime"] = alarmTimeUnix;
    obj["takenTime"] = takenTimeUnix;

    JsonArray pillArray = obj.createNestedArray("alarmPills");
    
    for (std::vector<std::pair<String, int>>::iterator it = alarmPills.begin(); it != alarmPills.end(); ++it){
        StaticJsonDocument<384> tempDoc;
        JsonObject pillNested = tempDoc.to<JsonObject>();
        pillNested["pill_name"] = it->first;
        pillNested["dosage"] = it->second;

        pillArray.add(pillNested);
    }
    // docArray.add(obj);
    outFile.close();

    outFile = SPIFFS.open("/firestore_update.json", "w");

    if (serializeJson(doc, outFile) == 0){
        Serial.println("FAILED TO WRITE IN firestore_update.json");
        return false;
    }
    else {
        Serial.println("SUCCESSFULY WRITTEN IN firestore_update.json");
        return true;
    }
    outFile.close();
}

void readFirestoreQueue(){
    File file = SPIFFS.open("/firestore_update.json");
    if(file){
        StaticJsonDocument<1512> doc;
        DeserializationError error = deserializeJson(doc, file);
        if (error){
            Serial.println("ERROR in reading firestore_update.json");
        }
        else{
            Serial.println("firestore_update.json");
            JsonArray array = doc.as<JsonArray>();
            for (JsonObject obj : array){
                int alarmTime = obj["alarmTime"];
                int takenTime = obj["takenTime"];
                int alarmState = obj["alarmState"];
                JsonArray pillArray = obj["alarmPills"];

                std::map<String, int> pillMap;
                for (JsonObject alarmObj: pillArray){
                    String pillName = alarmObj["pill_name"];
                    int dosage = alarmObj["dosage"];
                    pillMap.insert({pillName, dosage});
                }

                Serial.println("Alarm Time: " + DateTime(alarmTime).timestamp());
                Serial.println("Taken Time: " + DateTime(takenTime).timestamp());
                Serial.println("Alarm State: " + String(alarmState));
                Serial.print("Pill Map: ");
                for (std::map<String,int>::iterator it = pillMap.begin(); it!= pillMap.end(); ++it){
                    Serial.print(it->first); Serial.print(" : ");
                    Serial.println(String(it->second));
                }
        }
        }
    }
    file.close();
}

boolean uploadFirestoreQueue(){
    bool result = false;
    File file = SPIFFS.open("/firestore_update.json");
    if(file){
        StaticJsonDocument<1512> doc;
        DeserializationError error = deserializeJson(doc, file);
        if (error && doc.isNull()){
            Serial.println("ERROR in reading firestore_update.json");
        }
        Serial.println("Uploading Queued Data to Firestore...");
        JsonArray array = doc.as<JsonArray>();
        for (JsonObject obj : array){
            int alarmTime = obj["alarmTime"];
            int takenTime = obj["takenTime"];
            int alarmState = obj["alarmState"];
            JsonArray pillArray = obj["alarmPills"];
            
            std::vector<std::pair<String, int>> pillVector;
            for (JsonObject alarmObj: pillArray){
                String pillName = alarmObj["pill_name"];
                int dosage = alarmObj["dosage"];
                pillVector.push_back(std::make_pair(pillName, dosage));
            }

            Serial.println("Alarm Time: " + DateTime(alarmTime).timestamp());
            Serial.println("Taken Time: " + DateTime(takenTime).timestamp());
            Serial.println("Alarm State: " + String(alarmState));
            Serial.print("Pill Map: ");

            network->firestoreDataUpdate(alarmTime, takenTime, pillVector, alarmState);
            delay(500);
        }

        File fileClear = SPIFFS.open("/firestore_update.json", "w");
        doc.clear();
        if (serializeJson(doc, fileClear) == 0){
            Serial.println("FAILED TO WRITE IN firestore_update.json");
            result = false;
        }
        else {
            Serial.println("Clear firestore_update.json");
            result = true;
        }
        fileClear.close();
    }
    file.close();
    return result;
}

// boolean updateWiFiConfig (String wifiID, String wifiPassword){
//     File file = SPIFFS.open("/config.json", "r");
//     DynamicJsonDocument doc(256);
//     DeserializationError error = deserializeJson(doc, file);
//     if (error){
//         Serial.print("ERROR unable to read config.json: ");
//         Serial.println(error.c_str());
//         return false;
//     }
//     doc["WIFI_SSID"] = wifiID;
//     doc["WIFI_PASSWORD"] = password;
//     file.close();

//     file = SPIFFS.open("/config.json", "w");
//     if(serializeJson(doc, file) == 0){
//         Serial.println("Failed to rewrite config.json");
//         return false;
//     }
//     file.close();
//     return true;
// }

// boolean updateAlarmConfig(int ringDuration, int snoozeDuration, int snoozeAmount){
//     return false;

// }

void loadConfigJSON(){
    File file = SPIFFS.open("/config.json");
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, file);
    if (error){
        Serial.print("ERROR unable to load configurations from config.json: ");
        Serial.println(error.c_str());
        return;
    }
    
    WIFI_SSID = (char*)(doc["WIFI_SSID"].as<String>()).c_str();
    WIFI_PASSWORD = (char*)(doc["WIFI_PASSWORD"].as<String>()).c_str();
    ringDuration = doc["ringDuration"].as<int>();
    snoozeDuration = doc["snoozeDuration"].as<int>();
    snoozeAmount = doc["snoozeAmount"].as<int>();

    Serial.println("Data loaded from config.json");
    file.close();
}