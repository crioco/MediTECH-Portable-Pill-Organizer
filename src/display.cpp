#include <display.h>

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Constructor for OLED Display Library

extern bool isWiFiConnected;
extern bool isBluetoothEnabled;
extern char* WIFI_SSID;
extern const char* NTP_SERVER;

extern char dayOfWeek[7][4];
extern char months[12][4];

extern int batteryLevel;
extern bool RTCFound;
extern bool AHTFound;

extern RTC_DS3231 rtc;

void Display::initDisplay(){
    if (u8g2.begin()){
        u8g2.enableUTF8Print();
        Serial.println("[Success] OLED Display On");
    } else {
        Serial.println("[Failed] OLED Display Not Found");
        return;
    }
     
    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_POWERON){
        FromPowerOn();
    } else if (reason == ESP_RST_SW){
        FromReset();
    }

    delay(2000);
    ComponentStatus();
    delay(4000);
}

void Display::PowerSaveOn(bool enabled){
    if (enabled){
        u8g2.setPowerSave(1);
    }else{
        u8g2.setPowerSave(0);
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
            u8g2.drawStr(96, 40, meridiem.c_str());
        } else {
            u8g2.drawStr(1, 40, time.c_str());
            u8g2.setFont(u8g2_font_helvB10_tr);
            u8g2.drawStr(102, 40, meridiem.c_str());
        }

        // DATE
        // u8g2.setFont(u8g2_font_ncenB08_tr);
        // u8g2.drawStr(75, 50, date.c_str());
        // u8g2.drawStr(75, 60, String(now.year()).c_str());

        // DATE
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(50, 59, date.c_str());

        // DAY OF WEEK
        // u8g2.setFont(u8g2_font_ncenB18_tr);
        u8g2.setFont(u8g2_font_helvB14_tr);
        u8g2.drawStr(2, 60, dayOfWeek[now.dayOfTheWeek()]);
        
        // BATTERY LEVEL
        u8g2.setFont(u8g2_font_helvB08_tr);
        String battery = String(batteryLevel) + "%";
        u8g2.drawStr(20, 8, battery.c_str());
        
        u8g2.setFont(u8g2_font_siji_t_6x10); // BATTERY
        
        if (batteryLevel >= 90){
            u8g2.drawGlyph(5, 8, 57940);
        }else if (batteryLevel >= 80 || batteryLevel < 90){
            u8g2.drawGlyph(5, 8, 57939);
        }else if (batteryLevel >= 70 || batteryLevel < 80){
            u8g2.drawGlyph(5, 8, 57938);
        }else if (batteryLevel >= 60 || batteryLevel < 70){
            u8g2.drawGlyph(5, 8, 57937);
        }else if (batteryLevel >= 50 || batteryLevel < 60){
            u8g2.drawGlyph(5, 8, 57936);
        }else if (batteryLevel >= 40 || batteryLevel < 50){
            u8g2.drawGlyph(5, 8, 57935);
        }else if (batteryLevel >= 30 || batteryLevel < 40){
            u8g2.drawGlyph(5, 8, 57934);
        }else if (batteryLevel >= 20 || batteryLevel < 30){
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

void Display::displayText(String text){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB10_tr);
        u8g2.drawStr(5, 35, text.c_str());
    } while (u8g2.nextPage());
}

void Display::displayAlarm(DateTime time){
    int x = 0;
    String hour, minute, ftime;
    hour = String(time.twelveHour());
    if (time.minute() < 10) minute = "0" + String(time.minute());
    else minute = String(time.minute());
    String fTime = hour +":" + minute;

    if (time.hour() < 12) fTime += " AM";
    else fTime += " PM";

    if (time.twelveHour() < 10) x = 30;
    else x = 25;

    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_open_iconic_app_4x_t);
        u8g2.drawGlyph(48, 37, 69);
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.drawStr(x, 60, fTime.c_str());
    } while (u8g2.nextPage());
}

// WiFi ---------------------------------------------------------------------

void Display::WiFiEnabled(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(30, 25, "[ENABLED]");
        u8g2.drawStr(35, 45, "WiFi ON.");
    } while (u8g2.nextPage());   
}

void Display::WiFiDisabled(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(28, 25, "[DISABLED]");
        u8g2.drawStr(33, 45, "WiFi OFF.");  
    } while (u8g2.nextPage()); 

}

void Display::WiFiConnecting(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(10, 25, "Connecting to");
        u8g2.drawStr(40, 45, "WiFi...");
    } while (u8g2.nextPage());
}

void Display::WiFiReconnecting(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(10, 25, "Reconnecting");
        u8g2.drawStr(30, 45, "to WiFi...");
    } while (u8g2.nextPage());
}

void Display::WiFiConnected(IPAddress IP){
    String IP_Address = "IP: " + IP.toString();
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(22, 20, "[CONNECTED]");
        u8g2.drawStr(5, 35, WIFI_SSID);
        u8g2.drawStr(10, 50, IP_Address.c_str());
    } while (u8g2.nextPage());
}

void Display::WiFiNotConnected(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(38, 20, "[FAILED]");
        u8g2.drawStr(38, 40, "Failed to");
        u8g2.drawStr(16, 55, "Connect to WiFi.");
    } while (u8g2.nextPage());
}

void Display::WiFiDisconnected(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(13, 20, "[DISCONNECTED]");
         u8g2.drawStr(25, 40, "Disconnected");
        u8g2.drawStr(30, 55, "from WiFi.");
    } while (u8g2.nextPage());
}

// NTP ----------------------------------------------------------------------

void Display::NTPConnecting(){
    String NTPserver = "["+ String(NTP_SERVER) +"]"; 
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(28, 15, "Updating");
        u8g2.drawStr(10, 35, "Date & Time...");
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(18, 60, NTPserver.c_str());
    } while (u8g2.nextPage());
}

void Display::NTPConnected(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(30, 25, "[UPDATED]");
        u8g2.drawStr(5, 43, "Date & Time Updated.");
    } while (u8g2.nextPage());
}

void Display::NTPNotConnected(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(38, 20, "[FAILED]");
        u8g2.drawStr(30, 40, "Date & Time");
        u8g2.drawStr(30, 55, "Not Updated.");
    } while (u8g2.nextPage());
}


// FIREBASE --------------------------------------------------------------

void Display::FirebaseConnecting(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(10, 30, "Connecting to");
        u8g2.drawStr(25, 50, "Firebase...");
    } while (u8g2.nextPage());
}

void Display::FirebaseConnected(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(23, 25, "[CONNECTED]");
        u8g2.drawStr(20, 43, "Firebase Ready.");
    } while (u8g2.nextPage());
}

void Display::FirebaseNotConnected(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(38, 20, "[FAILED]");
        u8g2.drawStr(17, 40, "Failed to connect");
        u8g2.drawStr(33, 55, "to Firebase.");
    } while (u8g2.nextPage());
}

// BLUETOOTH

void Display::BluetoothEnabled(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
        u8g2.drawGlyph(24, 10, 74);
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(30, 30, "[ENABLED]");
        u8g2.drawStr(16, 50, "BLUETOOTH ON.");
    } while (u8g2.nextPage());  
}

void Display::BluetoothDisabled(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
        u8g2.drawGlyph(24, 10, 74);
        u8g2.setFont(u8g2_font_helvB08_tr);
        u8g2.drawStr(28, 30, "[DISABLED]");
        u8g2.drawStr(13, 50, "BLUETOOTH OFF.");
    } while (u8g2.nextPage());  
}

// u8g2.setFont(u8g2_font_streamline_interface_essential_alert_t);
// u8g2.drawGlyph(95, 52, 50); // Alarm Clock
// u8g2.drawGlyph(95, 52, 49); // Snooze
// u8g2.setFont(u8g2_font_streamline_health_beauty_t);
// u8g2.drawGlyph(95, 52, 70); // Pill
// u8g2.setFont(u8g2_font_streamline_coding_apps_websites_t); 
// u8g2.drawGlyph(24, 10, 54); // Datebase


// MISC

void Display::HumTemp(float humidity, float temperature){
    String temp = String(temperature)+" \xb0"+"C";
    String hum = String(humidity)+" %";

    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_streamline_weather_t); 
        u8g2.drawGlyph(10, 30, 54); // TEMP
        u8g2.setFont(u8g2_font_open_iconic_thing_2x_t); 
        u8g2.drawGlyph(10, 60, 72); // HUMI

        u8g2.setFont(u8g2_font_helvB12_tf);
        u8g2.drawStr(40, 25, temp.c_str());
        u8g2.drawStr(40, 58, hum.c_str());

        // u8g2.setFont(u8g2_font_streamline_weather_t); 
        // u8g2.drawGlyph(25, 40, 54); // TEMP
        // u8g2.setFont(u8g2_font_open_iconic_thing_2x_t); 
        // u8g2.drawGlyph(90, 35, 72); // HUMI

        // u8g2.setFont(u8g2_font_ncenB10_tf);
        // u8g2.drawStr(7, 55, temp.c_str());
        // u8g2.drawStr(75, 55, hum.c_str());

    } while (u8g2.nextPage());
}

void Display::FromPowerOn(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(30, 30, "Starting");
        u8g2.drawStr(15, 50, "MediTECH...");
    } while (u8g2.nextPage());
}

void Display::FromReset(){
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_helvB10_tr);
        u8g2.drawStr(20, 30, "Restarting");
        u8g2.drawStr(15, 50, "MediTECH...");
    } while (u8g2.nextPage());
}

void Display::ComponentStatus(){
    u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_helvB08_tr);
            u8g2.drawStr(15, 25, "RTC Module");
            if (RTCFound){
               u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
               u8g2.drawGlyph(95, 27, 65);
            }else{
                u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
                u8g2.drawGlyph(95, 27, 66);
            }

            u8g2.setFont(u8g2_font_helvB08_tr);
            u8g2.drawStr(15, 55, "AHT Module");
            if (AHTFound){
               u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
               u8g2.drawGlyph(95, 57, 65);
            }else{
                u8g2.setFont(u8g2_font_open_iconic_check_2x_t); 
                u8g2.drawGlyph(95, 57, 66);
            }

        } while (u8g2.nextPage());
}  


