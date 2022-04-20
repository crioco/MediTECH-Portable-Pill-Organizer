#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h>
// [days] from APP make 0 to 7 because dayOfWeek of SUNDAY in Dart is 7 while it is 0 in Arduino

class Bluetooth{
    public:
        void initBluetooth();
        void BTDisconnect();
        void readAuth();
        void readBluetoothSerial();
};

#endif