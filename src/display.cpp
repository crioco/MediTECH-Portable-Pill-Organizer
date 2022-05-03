#include <display.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Constructor for OLED Display Library

extern bool isWiFiConnected;
extern bool isBluetoothEnabled;
extern String WIFI_SSID;

extern char dayOfWeek[7][4];
extern char months[12][4];

extern int batteryLevel;
extern bool RTCFound;
extern bool AHTFound;
extern bool OLEDFound;

extern int displayCountStart;
extern RTC_DS3231 rtc;

void Display::initDisplay(){
    if (u8g2.begin()){
        OLEDFound = true;
        u8g2.enableUTF8Print();
        Serial.println("[Success] OLED Display On");
    } else {
        OLEDFound = false;
        Serial.println("[Failed] OLED Display Not Found");
        return;
    }
    
    if (OLEDFound){
        esp_reset_reason_t reason = esp_reset_reason();
        if (reason == ESP_RST_SW){
            FromReset();
        } else{
            FromPowerOn();
        }

        delay(3000);
        ComponentStatus();
        delay(3000);
    }
    
}
void Display::PowerSaveOn(bool enabled){
    if (enabled){
        u8g2.setPowerSave(1);
    }else{
        u8g2.setPowerSave(0);
        displayCountStart = millis();
    }
}

void Display::displayTime(){
    
    DateTime now = rtc.now();

    String day, month, date, hour, minute, second;
    // HOUR
    hour = String (now.twelveHour());

    // MINUTE
    if (now.minute() < 10) minute = "0" + String(now.minute());
    else minute = String(now.minute());

    // SECOND
    if (now.second() < 10) second = "0" + String(now.second());
    else second = String(now.second());

    String time = hour+":"+ minute+":"+second;
    String meridiem;
    if (now.hour() >= 12) meridiem = "PM";
    else meridiem = "AM";

    if (now.day() < 10) day = "0" + String(now.day());
    else day = String(now.day());

    if (now.month() < 10) month = "0" + String(now.month());
    else month = String(now.month());

    // date = month + "/" + day;
    date = month + "/" + day + "/" + String(now.year());

    // Draw on Display
    u8g2.firstPage();
    do {
        // TIME
        u8g2.setFont(u8g2_font_helvB18_tr);
        if (now.twelveHour() < 10){
            u8g2.drawStr(8, 40, time.c_str());
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(94, 40, meridiem.c_str());
        } else {
            u8g2.drawStr(1, 40, time.c_str());
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(102, 40, meridiem.c_str());
        }

        // DATE
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(52, 63, date.c_str());

        // DAY OF WEEK
        // u8g2.setFont(u8g2_font_ncenB18_tr);
        u8g2.setFont(u8g2_font_helvB14_tr);
        u8g2.drawStr(2, 63, dayOfWeek[now.dayOfTheWeek()]);
        
        // BATTERY LEVEL
        u8g2.setFont(u8g2_font_helvB08_tr);
        String battery = String(batteryLevel) + "%";
        u8g2.drawStr(20, 8, battery.c_str());
        
        u8g2.setFont(u8g2_font_siji_t_6x10); // BATTERY
        
        if (batteryLevel >= 90){
            u8g2.drawGlyph(5, 8, 57940);
        }else if (batteryLevel >= 80 && batteryLevel < 90){
            u8g2.drawGlyph(5, 8, 57939);
        }else if (batteryLevel >= 70 && batteryLevel < 80){
            u8g2.drawGlyph(5, 8, 57938);
        }else if (batteryLevel >= 60 && batteryLevel < 70){
            u8g2.drawGlyph(5, 8, 57937);
        }else if (batteryLevel >= 50 && batteryLevel < 60){
            u8g2.drawGlyph(5, 8, 57936);
        }else if (batteryLevel >= 40 && batteryLevel < 50){
            u8g2.drawGlyph(5, 8, 57935);
        }else if (batteryLevel >= 30 && batteryLevel < 40){
            u8g2.drawGlyph(5, 8, 57934);
        }else if (batteryLevel >= 20 && batteryLevel < 30){
            u8g2.drawGlyph(5, 8, 57933);
        }else{
            u8g2.drawGlyph(5, 8, 57932);
        }
        
        int x = 120;

        if (isBluetoothEnabled){
            u8g2.setFont(u8g2_font_siji_t_6x10); // BLUETOOTH open_iconic_embedded_1x_t 74
            u8g2.drawGlyph(x, 8, 57781);
            x = 110;
        }

        if (isWiFiConnected){
            u8g2.setFont(u8g2_font_open_iconic_www_1x_t); // WIFI
            u8g2.drawGlyph(x, 8, 72);
        }

    } while (u8g2.nextPage());

    // vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void Display::displayAlarm(DateTime time){
    PowerSaveOn(false);
    int x = 0;
    String hour, minute, ftime;
    hour = String(time.twelveHour());
    if (time.minute() < 10) minute = "0" + String(time.minute());
    else minute = String(time.minute());
    String fTime = hour +":" + minute;

    if (time.hour() < 12) fTime += " AM";
    else fTime += " PM";

    if (time.twelveHour() < 10) x = 25;
    else x = 20;

    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_alert_t);
        u8g2.drawGlyph(53, 30, 50); // Alarm Clock
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.drawStr(x, 56, fTime.c_str());
    } while (u8g2.nextPage());
}

// WiFi ---------------------------------------------------------------------

void Display::WiFiEnabled(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_other_t);
        u8g2.drawGlyph(53, 26, 66); // ON Switch
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(35, 50, "WiFi ON");
    } while (u8g2.nextPage());   
}

void Display::WiFiDisabled(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_other_t);
        u8g2.drawGlyph(53, 26, 65); // OFF Switch
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(32, 50, "WiFi OFF");  
    } while (u8g2.nextPage()); 

}

void Display::WiFiConnecting(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(13, 30, "Connecting to");
        u8g2.drawStr(41, 50, "WiFi...");
    } while (u8g2.nextPage());
}

void Display::WiFiReconnecting(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(13, 30, "Reconnecting");
        u8g2.drawStr(33, 50, "to WiFi...");
    } while (u8g2.nextPage());
}

void Display::WiFiConnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_wifi_t);
        u8g2.drawGlyph(53, 25, 48); // WiFi Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(20, 45, "Connected");
    } while (u8g2.nextPage());
}

void Display::WiFiNotConnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_internet_network_t);
        u8g2.drawGlyph(53, 26, 49); // Disable Internet Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(32, 45, "Failed to");
        u8g2.drawStr(7, 60, "Connect to WiFi");
    } while (u8g2.nextPage());
}

void Display::WiFiDisconnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_internet_network_t);
        u8g2.drawGlyph(53, 26, 50); // No Internet Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
         u8g2.drawStr(15, 45, "Disconnected");
        u8g2.drawStr(30, 60, "from WiFi");
    } while (u8g2.nextPage());
}

// NTP ----------------------------------------------------------------------

void Display::NTPConnecting(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_calendar_t);
        u8g2.drawGlyph(53, 22, 49); // Calendar
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(32, 40, "Updating");
        u8g2.drawStr(17, 60, "Date & Time...");

    } while (u8g2.nextPage());
}

void Display::NTPConnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_calendar_t);
        u8g2.drawGlyph(54, 22, 49); // Calendar
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        u8g2.drawGlyph(70, 26, 64); // Check
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(23, 45, "Date & Time");
        u8g2.drawStr(33, 60, "Updated");
        
    } while (u8g2.nextPage());
}

void Display::NTPNotConnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_calendar_t);
        u8g2.drawGlyph(54, 22, 49); // Calendar
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        u8g2.drawGlyph(70, 26, 68); // X
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(23, 45, "Date & Time");
        u8g2.drawStr(20, 60, "Not Updated");
    } while (u8g2.nextPage());
}


// FIREBASE --------------------------------------------------------------

void Display::FirebaseConnecting(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(10, 30, "Connecting to");
        u8g2.drawStr(25, 50, "Firebase...");
    } while (u8g2.nextPage());
}

void Display::FirebaseConnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t); 
        u8g2.drawGlyph(53, 26, 54); // Database Icon
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        u8g2.drawGlyph(65, 32, 64); // Check Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(10, 50, "Firebase Ready");
    } while (u8g2.nextPage());
}

void Display::FirebaseNotConnected(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t); 
        u8g2.drawGlyph(53, 24, 54); // Database Icon
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        u8g2.drawGlyph(65, 30, 68); // X Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(20, 45, "Firebase Not");
        u8g2.drawStr(25, 60, "Connected");
    } while (u8g2.nextPage());
}

void Display::FirebaseUploading(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_internet_network_t); 
        u8g2.drawGlyph(53, 24, 52); // Upload Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(30, 45, "Uploading");
        u8g2.drawStr(30, 60, "Pill Intake");
        }
    while (u8g2.nextPage());
}

void Display::FirebaseUpdated(bool state){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_internet_network_t); 
        u8g2.drawGlyph(53, 24, 52); // Upload Icon
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        if (state){
            u8g2.drawGlyph(65, 30, 64); // Check Icon
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(30, 45, "Pill Intake");
            u8g2.drawStr(30, 60, "Uploaded");    
        }else{
            u8g2.drawGlyph(65, 30, 68); // X Icon
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(30, 45, "Pill Intake");
            u8g2.drawStr(16, 60, "Upload Failed");      
        }
    } while (u8g2.nextPage());
}

// BLUETOOTH

void Display::BluetoothEnabled(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_circle_triangle_t); 
        u8g2.drawGlyph(53, 25, 53);
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        u8g2.drawGlyph(67, 27, 64); // Check Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(30, 46, "Bluetooth");
        u8g2.drawStr(50, 62, "ON");
    } while (u8g2.nextPage());  
}

void Display::BluetoothDisabled(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_circle_triangle_t); 
        u8g2.drawGlyph(53, 25, 53);
        u8g2.setFont(u8g2_font_open_iconic_check_1x_t);
        u8g2.drawGlyph(67, 27, 68); // X Icon
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(30, 46, "Bluetooth");
        u8g2.drawStr(46, 62, "OFF");
    } while (u8g2.nextPage());  
}

void Display::BTClientConnect(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_link_t); 
        u8g2.drawGlyph(53, 28, 48);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(25, 44, "Connected");
        u8g2.drawStr(35, 60, "to Client");
    } while (u8g2.nextPage());  
}

void Display::BTClientDisconnect(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_link_t); 
        u8g2.drawGlyph(53, 28, 50);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(15, 44, "Disconnected");
        u8g2.drawStr(25, 60, "from Client");
    } while (u8g2.nextPage());  
}

void Display::WiFiSettingsUpdate(bool state){
    PowerSaveOn(false);
    if (state){
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_streamline_interface_essential_cog_t); 
            u8g2.drawGlyph(53, 25, 48);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(18, 46, "WiFi Settings");
            u8g2.drawStr(35, 62, "Updated");
        } while (u8g2.nextPage());
    } else{
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_streamline_interface_essential_cog_t); 
            u8g2.drawGlyph(53, 25, 48);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(18, 46, "WiFi Settings");
            u8g2.drawStr(25, 62, "NOT Updated");
        } while (u8g2.nextPage());
    }
}

void Display::DeviceSettingsUpdate(bool state){
    PowerSaveOn(false);
    if (state){
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_streamline_interface_essential_cog_t); 
            u8g2.drawGlyph(53, 25, 48);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(10, 46, "Device Settings");
            u8g2.drawStr(35, 62, "Updated");
        } while (u8g2.nextPage());
    } else {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_streamline_interface_essential_cog_t); 
            u8g2.drawGlyph(53, 25, 48);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(10, 46, "Device Settings");
            u8g2.drawStr(25, 62, "NOT Updated");
        } while (u8g2.nextPage());
    }
}

void Display::PillSettingsUpdate(bool state){
    PowerSaveOn(false);
    if (state){
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_streamline_interface_essential_cog_t); 
            u8g2.drawGlyph(53, 25, 48);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(24, 46, "Pill Settings");
            u8g2.drawStr(35, 62, "Updated");
        } while (u8g2.nextPage());
    } else {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_streamline_interface_essential_cog_t); 
            u8g2.drawGlyph(53, 25, 48);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(24, 46, "Pill Settings");
            u8g2.drawStr(25, 62, "NOT Updated");
        } while (u8g2.nextPage());
    }
}

void Display::BTAuthorized(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_id_t); 
        u8g2.drawGlyph(53, 28, 53);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(23, 46, "Connection");
        u8g2.drawStr(26, 62, "Authorized");
    } while (u8g2.nextPage());  
}

void Display::BTDenied(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_id_t); 
        u8g2.drawGlyph(53, 28, 56);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(24, 46, "Connection");
        u8g2.drawStr(38, 62, "Denied");
    } while (u8g2.nextPage());  
}

// TEMPERATURE & HUMIDITY

void Display::HumTemp(float humidity, float temperature){
    PowerSaveOn(false);
    String temp = String(temperature)+" \xb0"+"C";
    String hum = String(humidity)+" %";

    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_weather_t); 
        u8g2.drawGlyph(10, 30, 54); // Thermometer Icon
        u8g2.setFont(u8g2_font_open_iconic_thing_2x_t); 
        u8g2.drawGlyph(10, 60, 72); // Humidity Icon

        u8g2.setFont(u8g2_font_helvB12_tf);
        u8g2.drawStr(40, 25, temp.c_str());
        u8g2.drawStr(40, 58, hum.c_str());

    } while (u8g2.nextPage());
}

void Display::HumTempWarning(float humidity, float temperature, int mode){
    String temp = String(temperature)+" \xb0"+"C";
    String hum = String(humidity)+" %";
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_circle_triangle_t); 
        u8g2.drawGlyph(53, 25, 52);
        u8g2.setFont(u8g2_font_helvB10_tf);

        if (mode == 1){
        u8g2.drawStr(5, 46, "HIGH HUMIDITY!!");
        u8g2.drawStr(38, 62, hum.c_str());
        }else if(mode == 2){
        u8g2.drawStr(20, 46, "HIGH TEMP!!!");
        u8g2.drawStr(38, 62, temp.c_str());
        };    
    } while (u8g2.nextPage());    
}

// DEVICE

void Display::FromPowerOn(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(35, 30, "Starting");
        u8g2.drawStr(22, 50, "MediTECH...");
    } while (u8g2.nextPage());
}

void Display::FromReset(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(28, 30, "Restarting");
        u8g2.drawStr(22, 50, "MediTECH...");
    } while (u8g2.nextPage());
}

void Display::PowerOFF(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_open_iconic_embedded_2x_t); 
        u8g2.drawGlyph(55, 25, 78);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(30, 46, "Powering ");
        u8g2.drawStr(42, 62, "OFF...");
    } while (u8g2.nextPage()); 
}

void Display::LowBattery(){
    PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_other_t); 
        u8g2.drawGlyph(53, 28, 48);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(12, 52, "LOW BATTERY");
    } while (u8g2.nextPage());  
}

void Display::ComponentStatus(){
    u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(10, 28, "RTC Module");
            if (RTCFound){
               u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
               u8g2.drawGlyph(105, 30, 65); // Check in Circle Icon
            }else{
                u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
                u8g2.drawGlyph(105, 30, 66); // X in Circle Icon
            }

            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(10, 52, "AHT Module");
            if (AHTFound){
               u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
               u8g2.drawGlyph(105, 56, 65);
            }else{
                u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
                u8g2.drawGlyph(105, 56, 66);
            }

        } while (u8g2.nextPage());
}


// ALARM --------------------------------------------------------------------------------------

void Display::AlarmStopped(){
     PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_alert_t); 
        u8g2.drawGlyph(53, 28, 48);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(13, 50, "Alarm Stopped");
    } while (u8g2.nextPage());  
}

void Display::AlarmSnooze(){
     PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_alert_t); 
        u8g2.drawGlyph(53, 28, 49);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(12, 50, "Alarm Snoozed");
    } while (u8g2.nextPage()); 
}

void Display::AlarmTakePills(){
     PowerSaveOn(false);
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_health_beauty_t); 
        u8g2.drawGlyph(53, 28, 70);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(40, 46, "Please");
        u8g2.drawStr(30, 62, "Take Pills");
    } while (u8g2.nextPage());
}

void Display::AlarmStatus(int status){
     PowerSaveOn(false);
    switch (status)
    {
    case 1:
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
            u8g2.drawGlyph(56, 27, 65);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(25, 50, "Pills Taken");
        } while (u8g2.nextPage());
        break;
    
    case 2:
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
            u8g2.drawGlyph(56, 27, 65);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(18, 50, "Pills Skipped");
        } while (u8g2.nextPage()); 
        break;
    case 3:
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
            u8g2.drawGlyph(56, 27, 66);
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(22, 50, "Pills Missed");
        } while (u8g2.nextPage()); 
        break;
    }
}

void Display::NextAlarm(String time, bool hasTwoDigits){
     PowerSaveOn(false);
    int x = 38;
    if (hasTwoDigits) x = 30;
 
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_interface_essential_alert_t); 
        u8g2.drawGlyph(53, 28, 51);
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(16, 46, "Next Alarm at");
        u8g2.drawStr(x, 62, time.c_str());
    } while (u8g2.nextPage());
}

void Display::DisplayText(String text){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(10, 46, text.c_str());
    } while (u8g2.nextPage());

}

