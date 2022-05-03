#include <spiffs_storage.h>

extern Network *network;
extern int ringDuration;
extern int snoozeDuration; 
extern int snoozeAmount;
extern String WIFI_SSID;
extern String WIFI_PASSWORD;
extern DateTime previousAlarmTime;
extern Display *display;

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

// Initialize SPIFFS
void initSPIFFS()
{
    if (!SPIFFS.begin(true))
    {
        Serial.println("[FAILED] ERROR IN INITIALIZING SPIFFS");
    }
}

// List all files in data folder
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
        DynamicJsonDocument doc(1700);
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
                String pillName = obj["pillName"];
                int container_slot = obj["containerSlot"];

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

// Show Data in data_storage.json
void readDataStorageJSON(){
    File file = SPIFFS.open("/data_storage.json");
    if (file){

        DynamicJsonDocument doc(1700);
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
            String pillName = obj["pillName"];
            int container_slot = obj["containerSlot"];

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

// Overwrite data_storage.json with data from Bluetooth
boolean writeDataStorageJSON(String json){
    bool result = false;
    File file = SPIFFS.open("/data_storage.json");
    
    DynamicJsonDocument doc(1700);
    DeserializationError error = deserializeJson(doc, json);
    if (error){
        Serial.print("ERROR. Failed to get data from Bluetooth json: ");
        Serial.println(error.c_str());
        file.close();
        return false;
    }
    file.close();

    file = SPIFFS.open("/data_storage.json", "w");
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

// Add Passed Alarm to be stored in Firestore upon connecting to Firebase
boolean addFirestoreQueue(int alarmTimeUnix, int takenTimeUnix, std::vector<std::pair<String, int>> alarmPills, int alarmState){
    File outFile = SPIFFS.open("/firestore_update.json", "r");
    DynamicJsonDocument doc(1512);
    DeserializationError error = deserializeJson(doc, outFile);
    JsonArray docArray;
    if (error){
        if (error.code() == DeserializationError::EmptyInput){
            docArray = doc.to<JsonArray>();
            Serial.println("Array created in firestore_update.json");
        }else{
            Serial.print("ERROR in opening firestore_update.json: ");
            Serial.println(error.c_str());
            return false;
        }
    }else if (doc.isNull()){
        Serial.println("doc was NULL. Array created in firestore_update.json");
        docArray = doc.to<JsonArray>();
    }else{
        docArray = doc.as<JsonArray>();
        Serial.println("Array found in firestore_update.json");
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
        pillNested["pillName"] = it->first;
        pillNested["dosage"] = it->second;

        pillArray.add(pillNested);
    }
    // docArray.add(obj);
    outFile.close();

    outFile = SPIFFS.open("/firestore_update.json", "w");

    if (serializeJson(doc, outFile) == 0){
        Serial.println("FAILED TO WRITE IN firestore_update.json");
        outFile.close();
        return false;
    }
    else {
        Serial.println("SUCCESSFULY WRITTEN IN firestore_update.json");
        outFile.close();
        return true;
    }
}

// Read data in firestore_update.json
void readFirestoreQueue(){
    File file = SPIFFS.open("/firestore_update.json");
    if(file){
        DynamicJsonDocument doc(1512);
        DeserializationError error = deserializeJson(doc, file);
        if (error){
            Serial.print("ERROR in reading firestore_update.json: ");
            Serial.println(error.c_str());
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
                    String pillName = alarmObj["pillName"];
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

// Upload Passed Alarm in firestore.update.json
boolean uploadFirestoreQueue(){
    bool result = false;
    File file = SPIFFS.open("/firestore_update.json");
    if(file){
        DynamicJsonDocument doc(1512);
        DeserializationError error = deserializeJson(doc, file);
        if (error){
            Serial.println("ERROR in reading firestore_update.json");
            result = false;
        }
        else if (!doc.isNull()){
            display->FirebaseUploading();
            Serial.println("Uploading Queued Data to Firestore...");
            JsonArray array = doc.as<JsonArray>();
            for (JsonObject obj : array){
                int alarmTime = obj["alarmTime"];
                int takenTime = obj["takenTime"];
                int alarmState = obj["alarmState"];
                JsonArray pillArray = obj["alarmPills"];
                
                std::vector<std::pair<String, int>> pillVector;
                for (JsonObject alarmObj: pillArray){
                    String pillName = alarmObj["pillName"];
                    int dosage = alarmObj["dosage"];
                    pillVector.push_back(std::make_pair(pillName, dosage));
                }

                Serial.println("Alarm Time: " + DateTime(alarmTime).timestamp());
                Serial.println("Taken Time: " + DateTime(takenTime).timestamp());
                Serial.println("Alarm State: " + String(alarmState));
                Serial.println("Pill Map: ");

                if(!network->firestoreDataUpdate(alarmTime, takenTime, pillVector, alarmState)){
                    Serial.println("Failed to Upload to Firestore. Try again later.");
                    return false;
                }
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
    }
    file.close();
    return result;
}

boolean updateWiFiConfig (String json){
    DynamicJsonDocument doc(312);
    DeserializationError error = deserializeJson(doc, json);
    if (error){
        Serial.print("ERROR unable to read received json: ");
        Serial.println(error.c_str());
        return false;
    }
    String SSID = doc["WIFI_SSID"];
    String Password = doc["WIFI_PASSWORD"];

    Serial.println("RECEIVED: " + SSID + " | " + Password);

    File file = SPIFFS.open("/config.json", "r");
    error = deserializeJson(doc, file);
    if (error){
        Serial.print("ERROR unable to read config.json: ");
        Serial.println(error.c_str());
        file.close();
        return false;
    }
    int rDuration = doc["ringDuration"];
    int sDuration = doc["snoozeDuration"];
    int sAmount = doc["snoozeAmount"];
    int prevAlarm = doc["previousAlarm"];
    file.close();

    file = SPIFFS.open("/config.json", "w");
    doc.clear();
    JsonObject obj = doc.to<JsonObject>();
    obj["WIFI_SSID"] = SSID;
    obj["WIFI_PASSWORD"] = Password;
    obj["ringDuration"] = rDuration;
    obj["snoozeDuration"] = sDuration;
    obj["snoozeAmount"] = sAmount;
    obj["previousAlarm"] = prevAlarm;
    if(serializeJson(doc, file) == 0){
        Serial.println("Failed to serialize config.json.");
        file.close();
        return false;
    }
     Serial.println("Updated WiFi Config in config.json.");
    file.close();
    return true;
}

boolean updateAlarmConfig(String json){
    DynamicJsonDocument doc(312);
    DeserializationError error = deserializeJson(doc, json);
    if (error){
        Serial.print("ERROR unable to read received json: ");
        Serial.println(error.c_str());
        return false;
    }
    int rDuration = doc["ringDuration"];
    int sDuration = doc["snoozeDuration"];
    int sAmount = doc["snoozeAmount"];

    Serial.println("RECEIVED: " + String(rDuration) + " | " + String(sDuration) + " | " + String(sAmount));

    File file = SPIFFS.open("/config.json", "r");
    error = deserializeJson(doc, file);
    if (error){
        Serial.print("ERROR unable to read config.json: ");
        Serial.println(error.c_str());
        file.close();
        return false;
    }
    String SSID = doc["WIFI_SSID"];
    String Password = doc["WIFI_PASSWORD"];
    int prevAlarm = doc["previousAlarm"];
    file.close();

    file = SPIFFS.open("/config.json", "w");
    doc.clear();
    JsonObject obj = doc.to<JsonObject>();
    obj["WIFI_SSID"] = SSID;
    obj["WIFI_PASSWORD"] = Password;
    obj["ringDuration"] = rDuration;
    obj["snoozeDuration"] = sDuration;
    obj["snoozeAmount"] = sAmount;
    obj["previousAlarm"] = prevAlarm;
    if(serializeJson(doc, file) == 0){
        Serial.println("Failed to serialize config.json.");
        file.close();
        return false;
    }
     Serial.println("Updated Alarm Config in config.json.");
    file.close();
    return true;
}

boolean storePrevAlarm(int prevAlarmUnix){
    DynamicJsonDocument doc(312);
    File file = SPIFFS.open("/config.json", "r");
    DeserializationError error = deserializeJson(doc, file);
    if (error){
        Serial.print("ERROR unable to read config.json: ");
        Serial.println(error.c_str());
        file.close();
        return false;
    }
    String SSID = doc["WIFI_SSID"];
    String Password = doc["WIFI_PASSWORD"];
    int rDuration = doc["ringDuration"];
    int sDuration = doc["snoozeDuration"];
    int sAmount = doc["snoozeAmount"];
    file.close();

    file = SPIFFS.open("/config.json", "w");
    doc.clear();
    JsonObject obj = doc.to<JsonObject>();
    obj["WIFI_SSID"] = SSID;
    obj["WIFI_PASSWORD"] = Password;
    obj["ringDuration"] = rDuration;
    obj["snoozeDuration"] = sDuration;
    obj["snoozeAmount"] = sAmount;
    obj["previousAlarm"] = prevAlarmUnix;
    if(serializeJson(doc, file) == 0){
        Serial.println("Failed to serialize config.json.");
        file.close();
        return false;
    }
     Serial.println("Updated PreviousAlarm in config.json.");
    file.close();
    return true;
}

// Update WiFi & Alarm Settings loaded using configurations in config.json
void loadConfigJSON(){
    File file = SPIFFS.open("/config.json");
    DynamicJsonDocument doc(312);
    DeserializationError error = deserializeJson(doc, file);
    if (error){
        Serial.print("ERROR unable to load configurations from config.json: ");
        Serial.println(error.c_str());
        return;
    }

    String ID = doc["WIFI_SSID"];
    String PASS = doc["WIFI_PASSWORD"];
    int rDuration = doc["ringDuration"];
    int sDuration = doc["snoozeDuration"];
    int sAmount = doc["snoozeAmount"];
    int prevAlarmUnix = doc["previousAlarm"];

    WIFI_SSID = ID;
    WIFI_PASSWORD = PASS;
    ringDuration = rDuration;
    snoozeDuration = sDuration;
    snoozeAmount = sAmount;
    previousAlarmTime = DateTime(prevAlarmUnix);

    Serial.println("Data loaded from config.json");
    file.close();
}