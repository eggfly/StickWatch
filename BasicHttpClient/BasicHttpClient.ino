#include <Arduino.h>

#include <WiFi.h>
#include <WiFiMulti.h>

#include <HTTPClient.h>

#include <Int64String.h>

#define ARDUINOJSON_USE_LONG_LONG 1

#include "ArduinoJson.h"

#define USE_SERIAL Serial

WiFiMulti wifiMulti;


void setup() {

  USE_SERIAL.begin(115200);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  wifiMulti.addAP("MIWIFI8", "12345678");

}

void loop() {
  // wait for WiFi connection
  if ((wifiMulti.run() == WL_CONNECTED)) {

    HTTPClient http;

    USE_SERIAL.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin("http://worldclockapi.com/api/json/utc/now"); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        // {
        //"$id":"1",
        //"currentDateTime":"2019-01-13T08:35Z",
        //"utcOffset":"00:00:00",
        //"isDayLightSavingsTime":false,
        //"dayOfTheWeek":"Sunday",
        //"timeZoneName":"UTC",
        //"currentFileTime":131918421379694089,
        //"ordinalDate":"2019-13",
        //"serviceResponse":null
        //}
        USE_SERIAL.printf("length: %d\n", payload.length());
        DynamicJsonDocument doc;
        deserializeJson(doc, payload);
        JsonObject obj = doc.as<JsonObject>();

        String currentFileTime = obj[String("currentFileTime")];
        USE_SERIAL.println( currentFileTime);
        uint64_t c_time = obj["currentFileTime"].as<unsigned long long>();


        struct timeval tv;
        tv.tv_sec = c_time / 1000 / 1000 / 10 - 11644473600 + 8 * 60 * 60;
        settimeofday(&tv, NULL);
        printLocalTime();
        USE_SERIAL.println( int64String(c_time));

        String currentDateTime = obj[String("currentDateTime")];
        USE_SERIAL.println( currentDateTime);
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }

  delay(5000);
}


void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain local time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
