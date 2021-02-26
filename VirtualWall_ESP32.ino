 /*----------------------------------------------------------------------------------------------------
  Project Name : Roomba Virtual Wall V3
  Features: Status check domoticz, battery status
  Authors: Patrick Stel
  Based on: https://github.com/MKme/Roomba
  Modified: https://github.com/PatrickSt1991/Roomba-VirtualWall-ESP32
    
////  Features :
// 1. Connect to Wi-Fi, and check Domoticz Roomba Status.
// 2. Send IR signal if Roomba is cleaning.
// 3. Remote Battery Status Monitoring.
// 4. Using Sleep mode to reduce the energy consumed.
// 5. Check time via NTP for sleep
// 6. Manual Switch if Roomba starts manually instead of schedule
----------------------------------------------------------------------------------------------------*/
#include <IRremote.h>
#include "WiFi.h" 
#include "time.h"
#include <HTTPClient.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds  */
#define BUTTON_PIN_BITMASK 0x200000000 /* 2^33 in hex */
#define WIFI_TIMEOUT 10000 // 10seconds in milliseconds

const char* ssid = "********";
const char* password = "********";
const char* roombaWallActive = "\"Data\" : \"On\"";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
const int   Analog_channel_pin = 15;

char timeHour[3];
char timeMin[3];
char timeSec[3];

int remaining_hourDiff = 0;
int remaining_hourNight = 0;
int remaining_hour = 0;
int remaining_sec = 0;
int remaining_min = 0;
int newSleepTime = 0;
int totalSleepTime = 0;
int SecMin = 60;
int ADC_VALUE = 0;
int SEND_PIN = 5;
int FixedCleanTime = 12;

double voltage_value = 0;

unsigned long minutes = 60000; //Time for transmitting
unsigned long startAttemptTime = millis();

IRsend irsend(SEND_PIN);
HTTPClient http;

void setup()
{
  WiFi.setHostname("Virtual_Wall_Keuken_V3");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT)
  {
    delay(10);
  }
  
  Serial.begin(115200);
  irsend.enableIROut(38);//Lib function
  
  esp_sleep_get_wakeup_cause();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
      voltageCheck();
      http.begin("http://192.168.0.125:8080/json.htm?type=command&param=switchlight&idx=68&switchcmd=On");
      int httpSwitchOn = http.GET();
      http.end();
      while(millis() < minutes * 60) //60
      {
        irsend.mark(1000);
        irsend.space(1000);
      }
      http.begin("http://192.168.0.125:8080/json.htm?type=command&param=switchlight&idx=68&switchcmd=Off");
      int httpSwitchOff = http.GET();
      http.end();
      getSleepTime();
      esp_deep_sleep_start();
  }

  /* Start Time Check */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    ESP.restart();
  }

  strftime(timeHour,3, "%H", &timeinfo); //11
  int StringTimeHour = atoi(timeHour);
  
  if(StringTimeHour != FixedCleanTime) //12
  {
    getSleepTime();
    esp_deep_sleep_start();
  }

  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
  {
    voltageCheck();
    //HIER MOET NOG EEN TIJD CHECK IN !!!!!!
    http.begin("http://192.168.0.125:8080/json.htm?type=devices&rid=44");
    int httpRoombaStatus = http.GET();
    
    if (httpRoombaStatus > 0) {
      String payload = http.getString();
      http.end();

      if(payload.indexOf(roombaWallActive) > 0) {
        http.begin("http://192.168.0.125:8080/json.htm?type=command&param=switchlight&idx=68&switchcmd=On");
        int httpTimerOn = http.GET();
        http.end();

        while(millis() < minutes * 60) //60
        {
          irsend.mark(1000);
          irsend.space(1000);
        }

        http.begin("http://192.168.0.125:8080/json.htm?type=command&param=switchlight&idx=68&switchcmd=Off");
        int httpTimerOff = http.GET();
        http.end();
        getSleepTime();
        esp_deep_sleep_start();
      }else{
        getSleepTime();
        esp_deep_sleep_start();
      }
    }
  }
}

void getSleepTime(){
  /* Start Time Check */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    ESP.restart();
  }

  strftime(timeHour,3, "%H", &timeinfo); //11
  strftime(timeMin,3, "%M", &timeinfo);  //21
  strftime(timeSec,3, "%S", &timeinfo);  //24
  int StringTimeHour = atoi(timeHour);
  int StringTimeMin = atoi(timeMin);
  int StringTimeSec = atoi(timeSec);

  //Hour greater then 12
  if(StringTimeHour > FixedCleanTime)//12
  {
    remaining_hourDiff = (StringTimeHour - 11); //11
    remaining_hourNight = (24 - remaining_hourDiff);
    remaining_hour = (remaining_hourNight * 3600);
  }
  //Hour greater less 12
  if(StringTimeHour < FixedCleanTime)//12
  {
    remaining_hourDiff = (11 - StringTimeHour); //11
    remaining_hour = (remaining_hourDiff * 3600);
  }

  if(StringTimeHour == FixedCleanTime)
  {
    remaining_hour = (23 * 3600);
  }

  //Get minutes
  if(StringTimeMin != 00)
  {
    remaining_min = ((SecMin - StringTimeMin) * 60); /* To seconds */
  }

  //Get seconds
  if(StringTimeSec != 00) 
  {
    remaining_sec = (SecMin - StringTimeSec);
  }

  totalSleepTime = (remaining_hour +remaining_min + remaining_sec);
  int TIME_TO_SLEEP = (totalSleepTime); 
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
}

void voltageCheck(){
  ADC_VALUE = analogRead(Analog_channel_pin);
  voltage_value = (ADC_VALUE * 3.3)/(4095);
  delay(1000);
  String domoticzString="http://192.168.0.125:8080/json.htm?type=command&param=udevice&idx=60&nvalue=0&svalue=";  //Modify IDX to your own IDX if you want to have battery status in Domoticz
  String domoticzInput = domoticzString+voltage_value;
  http.begin(domoticzInput);
  int httpPowerVoltage = http.GET();
  http.end();
}

void loop(){}
