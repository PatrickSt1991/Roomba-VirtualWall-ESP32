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
----------------------------------------------------------------------------------------------------*/

#include <IRremote.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds  */
#define TIME_TO_SLEEP  3600    /* Time ESP32 will go to sleep (in seconds) 1 hour   */

const char* ssid = "********";
const char* password = "********";
const char* roombaWallActive = "\"Data\" : \"On\"";

unsigned long minutes = 60000; //Time for transmitting
int SEND_PIN = 5;
IRsend irsend(SEND_PIN);

void setup()
{
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Connected to the WiFi network");
  irsend.enableIROut(38);//Lib function

  HTTPClient http;

  http.begin("http://192.168.0.125:8080/json.htm?type=devices&rid=44");
  int httpCode = http.GET();

  if (httpCode > 0) {

    String payload = http.getString();
    
    if(payload.indexOf(roombaWallActive) > 0) {
      Serial.println("Roomba is active, starting virtual wall.");
      while(millis() < minutes * 60) //60
      {
        irsend.mark(1000);
        irsend.space(1000);
      }
      Serial.println("Done transmitting, going to sleep.");
      esp_deep_sleep_start();
    }else{
      Serial.println("Roomba is sleeping, I'm going to sleep.");
      esp_deep_sleep_start();
    }
  }
  else {
    Serial.println("Error on HTTP request");
  }
}

void loop(){}
