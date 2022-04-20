#ifndef NETWORK_H  
#define NETWORK_H

#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <display.h>
#include <vector>

class Network{
    private:
        FirebaseData fbdo;
        FirebaseAuth auth;
        FirebaseConfig config;

        friend void WiFiEventConnected(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void WiFiEventGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void WiFiEventDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
        friend void FirestoreTokenStatusCallback(TokenInfo info);

    public:
        Network();
        void initWiFi();
        void firebaseInit();
        void WiFiDisconnect();
        void WiFiConnect();
        boolean firestoreDataUpdate(int alarmTimeUnix, int takenTimeUnix, std::vector<std::pair<String, int>> alarmPills, int alarmState);
};

#endif