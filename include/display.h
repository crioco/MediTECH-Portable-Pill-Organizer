#ifndef DISPLAY_H  
#define DISPLAY_H

#include <U8g2lib.h>
#include <RTClib.h>

class Display{
    public:
        void initDisplay(); //
        void displayTime(); //
        void displayAlarm(DateTime time);

        void WiFiEnabled();
        void WiFiDisabled();
        void WiFiConnecting();
        void WiFiReconnecting();
        void WiFiConnected();
        void WiFiNotConnected();
        void WiFiDisconnected();

        void NTPConnecting();
        void NTPConnected();
        void NTPNotConnected();

        void FirebaseConnecting();
        void FirebaseConnected();
        void FirebaseNotConnected();
        void FirebaseUploading();
        void FirebaseUpdated(bool state);

        void BluetoothEnabled();
        void BluetoothDisabled();
        void BTClientConnect();
        void BTClientDisconnect();
        void BTAuthorized();
        void BTDenied();

        void WiFiSettingsUpdate(bool state);
        void DeviceSettingsUpdate(bool state);
        void PillSettingsUpdate(bool state);

        void HumTemp(float humidity, float temperature);
        void HumTempWarning(float humidity, float temperature, int mode);

        void FromPowerOn();
        void FromReset();
        void ComponentStatus();

        void PowerSaveOn(bool enabled);
        void PowerOFF();
        void LowBattery();
        void AlarmSnooze();
        void AlarmStopped();
        void AlarmTakePills();
        void AlarmStatus(int status);
        void NextAlarm(String time, bool hasTwoDigits);

        void DisplayText(String text);
};

#endif