 /*----------------------------------------------------------------------------------------------------
  Project Name : Roomba Virtual Wall V2
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

char timeMin[3];
char timeSec[3];

int remaining_sec = 0;
int remaining_min = 0;
int newSleepTime = 0;
int SecMin = 60;
int ADC_VALUE = 0;
int SEND_PIN = 5;

double voltage_value = 0;

unsigned long minutes = 60000; //Time for transmitting
unsigned long startAttemptTime = millis();

IRsend irsend(SEND_PIN);

void setup()
{
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT)
  {
    delay(10);
  }
  
  Serial.begin(115200);
  irsend.enableIROut(38);//Lib function

  HTTPClient http;
  
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    ESP.restart();
  }
 
  strftime(timeMin,3, "%M", &timeinfo);
  strftime(timeSec,3, "%S", &timeinfo);

  int StringTimeMin = atoi(timeMin);
  int StringTimeSec = atoi(timeSec);

  if(StringTimeMin != 00)
  {
    remaining_min = ((SecMin - StringTimeMin) * 60); /* To seconds */
  }
  
  if(StringTimeSec != 00) 
  {
    remaining_sec = (SecMin - StringTimeSec);
  }

  int TIME_TO_SLEEP = (remaining_min + remaining_sec); 
  esp_sleep_get_wakeup_cause();
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1); //1 = High, 0 = Low


  ADC_VALUE = analogRead(Analog_channel_pin);
  voltage_value = (ADC_VALUE * 3.3)/(4095);
  delay(1000);
  String domoticzString="http://192.168.0.125:8080/json.htm?type=command&param=udevice&idx=60&nvalue=0&svalue=";  //Modify IDX to your own IDX if you want to have battery status in Domoticz
  String domoticzInput = domoticzString+voltage_value;
  http.begin(domoticzInput);
  int httpUpdateCode = http.GET();

  esp_sleep_wakeup_cause_t = esp_sleep_get_wakeup_cause();

  if (esp_sleep_wakeup_cause_t == ESP_SLEEP_WAKEUP_EXT0)
  {
      while(millis() < minutes * 60) //60
      {
        irsend.mark(1000);
        irsend.space(1000);
      }
      esp_deep_sleep_start();
  }

  http.begin("http://192.168.0.125:8080/json.htm?type=devices&rid=44");   //Modify to your Domoticz IDX if you have Roomba connected to Domoticz
  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();
    
    if(payload.indexOf(roombaWallActive) > 0) {
      while(millis() < minutes * 60) //60
      {
        irsend.mark(1000);
        irsend.space(1000);
      }
      esp_deep_sleep_start();
    }else{
      esp_deep_sleep_start();
    }
  }
}
void loop(){}
