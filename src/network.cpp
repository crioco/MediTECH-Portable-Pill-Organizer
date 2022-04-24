#include <network.h>
#include <addons/TokenHelper.h>
#include <spiffs_storage.h>

// char* WIFI_SSID = (char*)"CaCO"; //
// char* WIFI_PASSWORD = (char*)"tigerleo62"; //

// String WIFI_SSID = "AN5506-04-FA_de9d0";
// String WIFI_PASSWORD = "12d2162f"; 

String WIFI_SSID = "";
String WIFI_PASSWORD = ""; 

#define API_KEY "AIzaSyADgJBV7cD__GtL8MZN9L_c2hC6bxde-Ik"
#define FIREBASE_PROJECT_ID "meditech-9904d"
#define USER_EMAIL "device001@meditech.com"
#define USER_PASSWORD "Meditech123"
#define DEVICEID "DEVICE001"

// static Network *instance = NULL;
bool isWiFiEnabled = false; // WiFi Enabled (When enabled reconnects WiFi when disconnected)
bool isWiFiConnected = false; // 
extern Display *display; 

long WiFiConnectingDuration = 3600000; // 1 minute in milliseconds
Network::Network(){
    // instance = this;
}

// Callback when connected to Wi-Fi successfully
void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.println("[WiFi CONNECTED]");
}
// Callback when successfully assigned Local IP from Wi-Fi Network & Connects to Firestore database

void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
    Serial.print("LOCAL IP ADDRESS: "); Serial.println(WiFi.localIP());
    display->WiFiConnected();
    isWiFiConnected = true;
    delay(2000);
}
// Callback when disconnected from the Wi-Fi Network and attempts to reconnect

void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    isWiFiConnected = false;
    display->WiFiDisconnected();
    delay(2000);
    Serial.println("[WiFi DISCONNECTED]");
    if (isWiFiEnabled == true) {
        unsigned long previousMillis = millis();
        unsigned long currentMillis = millis();
        while (WiFi.status() != WL_CONNECTED && (currentMillis - previousMillis) < WiFiConnectingDuration ){
            WiFi.reconnect();
            currentMillis = millis();
            display->WiFiReconnecting();
        }
        if(WiFi.status() == WL_CONNECTED) return;
        display->WiFiNotConnected();
        delay(2000);
    };
}

// Callback Firestore Token Status
void FirestoreTokenStatusCallback(TokenInfo info){
    Serial.printf("[Firestore Token Info]: type = %s, status = %s\n", getTokenType(info).c_str(), getTokenStatus(info).c_str());
}

// Initializes the WI-Fi connection
void Network::initWiFi(){   
    WiFi.onEvent(WiFiEventConnected, SYSTEM_EVENT_STA_CONNECTED);
    WiFi.onEvent(WiFiEventGotIP, SYSTEM_EVENT_STA_GOT_IP);
    WiFi.onEvent(WiFiEventDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}

void Network::WiFiConnect(){
    isWiFiEnabled = true;
    display->WiFiEnabled();
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
    delay(1000);

    unsigned long previousMillis = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - previousMillis) < WiFiConnectingDuration ){
        display->WiFiConnecting();
    } delay(2000);
    if(WiFi.status() == WL_CONNECTED) return;
    display->WiFiNotConnected();
    delay(2000);
}

void Network::WiFiDisconnect(){
    isWiFiEnabled = false;
    display->WiFiDisabled();
    delay(2000);
    WiFi.disconnect();
    delay(2000);
}

// Initializes the Firestore settings and connection
void Network::firebaseInit(){
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.token_status_callback = FirestoreTokenStatusCallback;

    display->FirebaseConnecting();

    Firebase.begin(&config, &auth);
    delay(2000);
    display->FirebaseConnected();
    delay(2000);
}

// Create/Update Data into the Firestore database
boolean Network::firestoreDataUpdate(int alarmTimeUnix, int takenTimeUnix, std::vector<std::pair<String, int>> alarmPills, int alarmState){
    if (WiFi.status() == WL_CONNECTED && Firebase.ready()){

        DateTime alarmTime = DateTime(alarmTimeUnix);
        DateTime takenTime = DateTime(takenTimeUnix);

        String alarmYear, alarmMonth, alarmDay, alarmHour, alarmMinute;
        alarmYear = String(alarmTime.year());

        if (alarmTime.month() < 10) alarmMonth = "0" + String(alarmTime.month());
        else alarmMonth = String(alarmTime.month());

        if (alarmTime.day() < 10) alarmDay = "0" + String(alarmTime.day());
        else alarmDay = String(alarmTime.day());

        if (alarmTime.hour() < 10) alarmHour = "0" + String(alarmTime.hour());
        else alarmHour = String(alarmTime.hour());

        if (alarmTime.minute() < 10) alarmMinute = "0" + String(alarmTime.minute());
        else alarmMinute = String(alarmTime.minute());
        
        String collectionName = DEVICEID; // DeviceName (DEVICE001)
        String documentName = alarmYear + alarmMonth + alarmDay +"-"+ alarmHour + alarmMinute; // yearMonthday-hourMinute (of Alarm) e.g. 20220404-1642
        String documentPath = collectionName +"/"+ documentName; // "House/Room_1"

        FirebaseJson content;

        bool isMissed, isTaken;

        switch (alarmState)
        {
        case 1: // Taken
            isTaken = true;
            isMissed = false;
            break;
        case 2: // Skipped
            isTaken = false;
            isMissed = false;
            break;
        case 3: // Missed
            isTaken = false;
            isMissed = true;
            break;
        }

        // Iterate through pills scheduled in alarm with pill names and dosages
        for (std::vector<std::pair<String, int>>::iterator it = alarmPills.begin(); it != alarmPills.end(); ++it){
            String pill_name = it->first;
            content.set("fields/pills/mapValue/fields/"+pill_name+"/mapValue/fields/dosage/integerValue", it->second);
            content.set("fields/pills/mapValue/fields/"+pill_name+"/mapValue/fields/isTaken/booleanValue", isTaken);
        }
        
        content.set("fields/isMissed/booleanValue", isMissed);

        DateTime tempTime = alarmTime + TimeSpan(0,-8,0,0);
        String alarmTimestamp = tempTime.timestamp()+"Z";

        tempTime = takenTime + TimeSpan(0,-8,0,0);
        String takenTimestamp = tempTime.timestamp()+"Z";

        content.set("fields/alarmTime/timestampValue", alarmTimestamp); // Date have to be in GMT+0 (e.g 2022-04-03T13:34:00Z)
        content.set("fields/takenTime/timestampValue", takenTimestamp);

        // // Tries to Patch (Update) the Data if it already exists
        // if(Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID,"", documentPath.c_str(), content.raw(), 
        //     "pills.pill_name1.dosage,
        //      pills.pill_name1.isTaken,
        //      isMissed"))
        //     {
        //     Serial.printf("ok\ndocument updated\n%s\n\n", fbdo.payload().c_str());
        //     return;
        // }else{
        //     Serial.println(fbdo.errorReason());
        // }

        // Creates a new Document in Firestore database
        // if(Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID,"", documentPath.c_str(), content.raw())){
        //     Serial.printf("ok\ndocument created\n%s\n\n", fbdo.payload().c_str());
        //     return true;
        // }else{
        //     Serial.println(fbdo.errorReason());
        //     return false;
        // }
        
        int uploadStartMillis = millis();
        
        // Creates a new Document in Firestore database
        while(millis() - uploadStartMillis < 60000){
            if (Firebase.Firestore.createDocument(&fbdo, FIREBASE_PROJECT_ID,"", documentPath.c_str(), content.raw())){
                Serial.printf("[Success] Document Created in Firestore.\n%s\n\n", fbdo.payload().c_str());
                return true;
            } else Serial.println(fbdo.errorReason());
            delay(3000);
        } 
        
        Serial.println("[Failed] Unable to Create Document in Firestore.");
        return false;
        
        
    }
    // IF NOT CONNECTED TO WIFI / FIREBASE
    else {
        addFirestoreQueue(alarmTimeUnix, takenTimeUnix,alarmPills, alarmState);
    }   
}