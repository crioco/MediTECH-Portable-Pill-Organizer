#ifndef DISPLAY_H  
#define DISPLAY_H

#include <U8g2lib.h>
#include <RTClib.h>

class Display{
    public:
        void initDisplay();
        void displayTime();
        void displayText(String text);
        void displayAlarm(DateTime time);

        void WiFiEnabled();
        void WiFiDisabled();
        void WiFiConnecting();
        void WiFiReconnecting();
        void WiFiConnected(IPAddress IP);
        void WiFiNotConnected();
        void WiFiDisconnected();

        void NTPConnecting();
        void NTPConnected();
        void NTPNotConnected();

        void FirebaseConnecting();
        void FirebaseConnected();
        void FirebaseNotConnected();

        void Greeting();
        void ComponentStatus();
};

#endif