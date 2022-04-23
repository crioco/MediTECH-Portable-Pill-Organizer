#include <bluetooth.h>
#include <display.h>
#include <spiffs_storage.h>

String deviceBTName = "MediTECH Device";
bool isBluetoothEnabled;
BluetoothSerial SerialBT;
extern Display *display;
extern U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2;

bool bluetoothAuth = false;
bool isBTClientConnected;
bool isBTClientDisconnected; 

void callBack(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    if (event == ESP_SPP_SRV_OPEN_EVT){\
        isBTClientConnected = true;
        Serial.println("Connected to Client.");
    } else if (event == ESP_SPP_CLOSE_EVT){
        isBTClientDisconnected = true;
        Serial.println("Disconnected from Client."); 
        bluetoothAuth = false;
    }
}

void Bluetooth::initBluetooth(){
    SerialBT.begin(deviceBTName);
    isBluetoothEnabled = true;
    Serial.println("Bluetooth Enabled.");
    SerialBT.register_callback(callBack);
    display->BluetoothEnabled();
    delay(2000);
}

void Bluetooth::BTDisconnect(){
    SerialBT.end();
    Serial.println("Bluetooth Disabled.");
    isBluetoothEnabled = false;
    display->BluetoothDisabled();
    delay(2000);
}
String message;
void Bluetooth::readAuth(){
    if (SerialBT.available() > 0){
        message = SerialBT.readStringUntil('\n');
        message.remove(message.length() - 1, 1);
        Serial.println(message);
        if (message == "69"){ // nice
            SerialBT.println("Connection Authenticated.");
            bluetoothAuth = true;
            display->BTAuthorized();
            delay(2000);
        }
        else {
            SerialBT.println("Connection Denied.");
            SerialBT.disconnect();
            bluetoothAuth = false;
            display->BTDenied();
            delay(2000);
        }
    }
    delay(20);
}

String json;
void Bluetooth::readBluetoothSerial(){
    if (SerialBT.available() > 0){
        json = SerialBT.readStringUntil('\n');
        json.remove(json.length() - 1, 1);
        char mode = json.charAt(0);
        json.remove(0,1);
        Serial.println("json: " + json);
        switch (mode)
        {
        case '1': // data_storage.json
            if (writeDataStorageJSON(json)){
                getPillListfromJSON();
                SerialBT.println("Pill Settings Updated.");
                display->DeviceSettingsUpdate(true);
                delay(2000);
            } else {
                SerialBT.println("Failed to Update Data Storage.");
                display->DeviceSettingsUpdate(false);
                delay(2000);
            }
            break;
        
        case '2': // WiFi Settings
            if(updateWiFiConfig(json)){
                loadConfigJSON();
                SerialBT.println("WiFi Settings Updated.");
                display->WiFiSettingsUpdate(true);
                delay(2000);
            }else{
                SerialBT.println("Failed to Update WiFi Settings.");
                display->WiFiSettingsUpdate(false);
            }
            break;

        case '3': // Alarm Settings
            if(updateAlarmConfig(json)){
                loadConfigJSON();
                SerialBT.println("Alarm Settings Updated.");
                display->DeviceSettingsUpdate(true);
                delay(2000);
            }else{
                SerialBT.println("Failed to Update Alarm Settings.");
                display->DeviceSettingsUpdate(false);
                delay(2000);
            }
            break;
        }
    }
}