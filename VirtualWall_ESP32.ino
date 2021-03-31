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
// 5. Keep time via DS3231
// 6. Manual Switch if Roomba starts manually instead of schedule
----------------------------------------------------------------------------------------------------*/
#include <IRremote.h>
#include "WiFi.h" 
#include "time.h"
#include <HTTPClient.h>
#include <Wire.h>
#include "ds3231.h"

#define WIFI_TIMEOUT 10000 // 10seconds in milliseconds

const char* ssid = "********";
const char* password = ""********";";
const char* roombaWallActive = "\"Data\" : \"On\"";
const int   Analog_channel_pin = 15;
const int   timeClock = 27;
const int   pushButton = 33;

int ADC_VALUE = 0;
int SEND_PIN = 5;

// time when to wake up
uint8_t wake_HOUR = 11; //Node-red uses GMT +1 so its always 1200 and I set the timer during winter time.
uint8_t wake_MINUTE = 0;
uint8_t wake_SECOND = 30;

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

  esp_sleep_enable_ext0_wakeup(GPIO_NUM_33,1);
  esp_sleep_enable_ext1_wakeup(0x8000000,ESP_EXT1_WAKEUP_ALL_LOW);

  irsend.enableIROut(38);//Lib function
  Wire.begin();
  DS3231_init(DS3231_INTCN);
  DS3231_clear_a1f();
  set_alarm();
  print_wakeup_reason();
  esp_deep_sleep_start();
}

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    voltageCheck();
    buttonWakeUp();
  }

  if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1)
  {
    voltageCheck();
    timerWakeUp();  
  }
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

void set_alarm(void){
    uint8_t flags[5] = { 0, 0, 0, 1, 1 };

    DS3231_set_a1(wake_SECOND, wake_MINUTE, wake_HOUR, 0, flags);
    DS3231_set_creg(DS3231_INTCN | DS3231_A1IE);
}

void buttonWakeUp(){
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
    esp_deep_sleep_start();
}

void timerWakeUp(){
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
      esp_deep_sleep_start();
    }else{
      esp_deep_sleep_start();
    }
  }
}

void loop(){
 //not used
}
