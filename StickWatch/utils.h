#ifndef _UTILS_H_
#define _UTILS_H_

#include <esp_log.h>

#include <Preferences.h>

#define PreferenceNamespace "my-app"
#define PrefNameWifi "wifi"

void log(char * str) {
  Serial.print(millis()); Serial.print(":"); Serial.println(str);
}

String getWifiPref() {
  Preferences preferences;
  preferences.begin(PreferenceNamespace, true);
  String wifiStr = preferences.getString(PrefNameWifi, "");
  // Print the counter to Serial Monitor
  Serial.print("Current wifi value: ");
  Serial.println(wifiStr);
  preferences.end();
  return wifiStr;
}

size_t saveWifiPref(String wifiStr) {
  Preferences preferences;
  preferences.begin(PreferenceNamespace, false);
  size_t ret = preferences.putString(PrefNameWifi, "");
  // Print the counter to Serial Monitor
  Serial.print("Save wifi value: "); Serial.print(wifiStr);  Serial.print(", ret:"); Serial.println(ret);
  preferences.end();
  return ret;
}

unsigned int increasePrefCounter() {
  Preferences preferences;
  preferences.begin(PreferenceNamespace, false);
  unsigned int counter = preferences.getUInt("counter", 0);
  // Print the counter to Serial Monitor
  Serial.printf("Current counter value: %u\n", counter);
  // Store the counter to the Preferences
  counter++;
  preferences.putUInt("counter", counter);
  Serial.printf("Set counter value: %u\n", counter);
  preferences.end();
  return counter;
}


void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain local time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

#endif // _UTILS_H_
