#include <bluetooth.h>
#include <display.h>

String deviceBTName = "MediTECH Device";
bool isBluetoothEnabled;
BluetoothSerial SerialBT;
extern Display *display;

void Bluetooth::initBluetooth(){
    SerialBT.begin(deviceBTName);
    isBluetoothEnabled = true;
    Serial.print("Bluetooth Enabled.");
    Serial.println(ESP.getFreeHeap());
    display->BluetoothEnabled();
    delay(2000);
}

void Bluetooth::BTDisconnect(){
    SerialBT.disconnect();
    Serial.println("Bluetooth Disabled.");
    isBluetoothEnabled = false;
    display->BluetoothDisabled();
    delay(2000);
}

void Bluetooth::readData(){
    if (Serial.available()){
        SerialBT.write(Serial.read());
    }
    if (SerialBT.available()){
        Serial.write(SerialBT.read());
    }
    delay(20);
}