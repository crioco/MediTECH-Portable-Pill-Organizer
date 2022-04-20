#include <bluetooth.h>
#include <display.h>
#include <spiffs_storage.h>

String deviceBTName = "MediTECH Device";
bool isBluetoothEnabled;
BluetoothSerial SerialBT;
extern Display *display;

bool bluetoothAuth = false;

void callBack(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    if (event == ESP_SPP_SRV_OPEN_EVT){
        Serial.println("Connected to Client.");
    } else if (event == ESP_SPP_CLOSE_EVT){
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
            Serial.println("Authenticated.");
            bluetoothAuth = true;
        }
        else {
            SerialBT.disconnect();
            bluetoothAuth = false;
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
        // Serial.println("mode: " + mode);
        Serial.println("json: " + json);
        switch (mode)
        {
        case '1': // data_storage.json
            if (writeDataStorageJSON(json)){
                getPillListfromJSON();
                SerialBT.println("Pill Settings Updated.");
            } else {
                SerialBT.println("Failed to Update Data Storage.");
            }
            break;
        
        case '2': // WiFi Settings
            if(updateWiFiConfig(json)){
                loadConfigJSON();
                SerialBT.println("WiFi Settings Updated.");
            }else{
                SerialBT.println("Failed to Update WiFi Settings.");
            }
            break;

        case '3': // Alarm Settings
            if(updateAlarmConfig(json)){
                loadConfigJSON();
                SerialBT.println("Alarm Settings Updated.");
            }else{
                SerialBT.println("Failed to Update Alarm Settings.");
            }
            break;
        
        default:
            Serial.println("ECHO: " + json);
            break;
        }
    }
}