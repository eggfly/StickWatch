
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include "driver/adc.h"
#include "esp_log.h"
#include "esp_pm.h"
#include <esp_sleep.h>

#include <Preferences.h>
#include "sensor.h"

#include <WiFi.h>

#include <WiFiMulti.h>
#include <HTTPClient.h>

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

static const char *TAG = "example";

const char* ssid       = "MIWIFI8";
const char* password   = "12345678";

#define HUNDRED_NANO_SECONDS (1000*1000*10)
const long  gmtOffset_sec = 3600 * 8;


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BtnPin 35
#define LedPin 19
#define BuzzerPin 26

// ================ Power IC IP5306 ===================
#define IP5306_ADDR           117
#define IP5306_REG_SYS_CTL0   0x00
#define IP5306_REG_READ1      0x71
#define IP5306_REG_READ2      0x72
#define CHARGE_FULL_BIT       3
#define POWER_LOAD_BIT        2

//RTC_DATA_ATTR int bootCount = 0;

U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R3, /* cs=*/ 14, /* dc=*/ 27, /* reset=*/ 33);

WiFiMulti wifiMulti;

char * reason;
char REASON_UNDEFINED[] = "UNDEFINED";
char REASON_ALL[] = "ALL";
char REASON_EXT0[] = "RTC_IO";
char REASON_EXT1[] = "RTC_CNTL";
char REASON_TIMER[] = "TIMER";
char REASON_TOUCHPAD[] = "TOUCHPAD";
char REASON_ULP[] = "ULP";
char REASON_GPIO[] = "GPIO";
char REASON_UART[] = "UART";
char REASON_OTHERS[] = "OTHERS";

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED : ESP_LOGI(TAG, "In case of deep sleep, reset was not caused by exit from deep sleep"); reason = REASON_UNDEFINED; break;
    case ESP_SLEEP_WAKEUP_ALL : ESP_LOGI(TAG, "Not a wakeup cause, used to disable all wakeup sources"); reason = REASON_ALL; break;
    case ESP_SLEEP_WAKEUP_EXT0 : ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_IO"); reason = REASON_EXT0; break;
    case ESP_SLEEP_WAKEUP_EXT1 : ESP_LOGI(TAG, "Wakeup caused by external signal using RTC_CNTL"); reason = REASON_EXT1; break;
    case ESP_SLEEP_WAKEUP_TIMER : ESP_LOGI(TAG, "Wakeup caused by timer"); reason = REASON_TIMER; break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : ESP_LOGI(TAG, "Wakeup caused by touchpad"); reason = REASON_TOUCHPAD; break;
    case ESP_SLEEP_WAKEUP_ULP : ESP_LOGI(TAG, "Wakeup caused by ULP program"); reason = REASON_ULP; break;
    case ESP_SLEEP_WAKEUP_GPIO : ESP_LOGI(TAG, "Wakeup caused by GPIO (light sleep only)"); reason = REASON_GPIO; break;
    case ESP_SLEEP_WAKEUP_UART : ESP_LOGI(TAG, "Wakeup caused by UART (light sleep only)"); reason = REASON_UART; break;
    default : ESP_LOGI(TAG, "Wakeup was not caused by deep sleep: %d\n", wakeup_reason); reason = REASON_OTHERS; break;
  }
}


//void set_freq(int freq) {
//  if (freq > 240) freq /= 1000000;
//  rtc_cpu_freq_t max_freq;
//  if (!rtc_clk_cpu_freq_from_mhz(freq, &max_freq)) {
//    Serial.println("Not a valid frequency");
//  }
//  esp_pm_config_esp32_t pm_config;
//  pm_config.max_cpu_freq = max_freq;
//  pm_config.min_cpu_freq = RTC_CPU_FREQ_XTAL;
//  pm_config.light_sleep_enable = false;
//
//  if (esp_pm_configure(&pm_config) != ESP_OK) {
//    Serial.println("Error configuring frequency");
//  }
//  rtc_clk_cpu_freq_set(max_freq);
//}

void log(char * str) {
  Serial.print(millis()); Serial.print(":"); Serial.println(str);
}

unsigned int increasePrefCounter() {
  Preferences preferences;
  preferences.begin("my-app", false);
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

void setPinModes() {
  // pinMode(BtnPin, INPUT_PULLUP);
  pinMode(BtnPin, INPUT_PULLUP);
  pinMode(LedPin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
}

void setup() {
  setPinModes();
  // simply to sleep when power chip reboot
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    led(true, 200);
    pureDeepSleep();
  }

  ESP_LOGD(TAG, "0");
  Serial.begin(115200);
  Wire.begin(21, 22, 100000);
  setPowerBoostKeepOn(true);

  // set_freq(240);
  ESP_LOGD(TAG, "1");

  u8g2.begin();
  setPinModes(); // pin modes must be set after u8g2
  ESP_LOGD(TAG, "2");
  // u8g2.fillDisplay();
  // u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setFont(u8g2_font_6x10_tf);
  // u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  // u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  // u8g2.setFontDirection(0);
  ESP_LOGD(TAG, "3");

  // delay(1500);
  // u8g2.clearDisplay();
  // u8g2.clear();
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false).
  // Note: Namespace name is limited to 15 chars.


  ESP_LOGD(TAG, "4");
  showSplashScreen();
  // buzzer();
  flashLed(1000);

  setupMPU9250();

  wifiMulti.addAP(ssid, password);
  wifiMulti.addAP("eggfly", "12345678");

  syncTimeFromWifi();

  //Increment boot number and print it every reboot
  // ++bootCount;
  // ESP_LOGI(TAG, "Boot count: %d", bootCount);
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
}

void setPowerBoostKeepOn(bool en) {
  Wire.beginTransmission(IP5306_ADDR);
  Wire.write(IP5306_REG_SYS_CTL0);
  if (en) Wire.write(0x37); // Set bit1: 1 enable 0 disable boost keep on
  else Wire.write(0x35);    // 0x37 is default reg value
  int error = Wire.endTransmission();
  Serial.print("Wire.endTransmission result: ");
  Serial.println(error);
}

void buzzer() {
  for (int i = 0; i < 200; i++) {
    digitalWrite(BuzzerPin, HIGH);
    delayMicroseconds(20);
    digitalWrite(BuzzerPin, LOW);
    delayMicroseconds(980);
  }
}

void led(boolean on, int milliseconds) {
  digitalWrite(LedPin, on ? 1 : 0);
  delay(milliseconds);
}

void flashLed(int milliseconds) {
  for (int i = 0; i < milliseconds / 200; i++) {
    digitalWrite(LedPin, 1);
    delay(100);
    digitalWrite(LedPin, 0);
    delay(100);
  }
}

bool isTimeOK = false;

void syncTimeFromWifi() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.println("RTC already have time, will not connect to time server");
    isTimeOK = true;
    printLocalTime();
  } else {
    ESP_LOGE(TAG, "Failed to obtain time");
    Serial.printf("Connecting to %s ", ssid);
    boolean wifiConnected = true;
    unsigned long wifiStartTime = millis();
    showConnectingWifi();
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
      if (millis() - wifiStartTime >= 10 * 1000) {
        wifiConnected = false;
        break;
      }
    }
    Serial.println("");
    if (wifiConnected) {
      showWifiConnected();
      Serial.println("Yes! WIFI Connected");
      syncTimeByHttp();
    } else {
      Serial.println("No, WIFI not connected");
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
  }
}

void syncTimeByHttp() {
  HTTPClient http;
  Serial.print("[HTTP] begin...\n");
  // configure traged server and url
  http.begin("http://worldclockapi.com/api/json/utc/now"); //HTTP

  Serial.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.printf("length: %d\n", payload.length());
      DynamicJsonDocument doc;
      deserializeJson(doc, payload);
      JsonObject obj = doc.as<JsonObject>();

      String currentFileTime = obj[String("currentFileTime")];
      Serial.println( currentFileTime);
      uint64_t c_time = obj["currentFileTime"].as<unsigned long long>();

      struct timeval tv = {};
      tv.tv_sec = c_time / HUNDRED_NANO_SECONDS - 11644473600 + gmtOffset_sec;
      tv.tv_usec = 0;
      settimeofday(&tv, NULL);
      printLocalTime();
      // Serial.println( int64String(c_time));
    }
  } else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  struct tm timeinfo;
  //init and get the time
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed config time by server");
  } else {
    Serial.println("Successfully config time by server:");
    isTimeOK = true;
    printLocalTime();
  }
  //disconnect WiFi as it's no longer needed
}

void printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain local time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void showSplashScreen() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 20, "LOADING...");
  u8g2.sendBuffer();
}

void showConnectingWifi() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 20, "CONNECTING WIFI...");
  u8g2.sendBuffer();
}

void showWifiConnected() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 20, "WIFI CONNECTED, NTP..");
  u8g2.sendBuffer();
}


unsigned long powerOffButtonTime = 0;

const float MAX_CURSOR_ACC = 16;

unsigned long keepWakeUpTime = 0;

void drawCursor() {
  int cursor_x = SCREEN_WIDTH / 2 + (int)(SCREEN_WIDTH / 2 * (IMU.roll / MAX_CURSOR_ACC));
  int cursor_y = SCREEN_HEIGHT / 2 - (int)(SCREEN_HEIGHT / 2 * (IMU.pitch / MAX_CURSOR_ACC));
  if (cursor_x < 0 ) {
    cursor_x = 0;
  }
  if (cursor_x >= SCREEN_WIDTH) {
    cursor_x = SCREEN_WIDTH - 1;
  }
  if (cursor_y < 0) {
    cursor_y = 0;
  }
  if (cursor_y >= SCREEN_HEIGHT - 1) {
    cursor_y = SCREEN_HEIGHT - 2;
  }
  u8g2.drawTriangle(cursor_x, cursor_y, cursor_x, cursor_y + 9, cursor_x + 6, cursor_y + 6);
  u8g2.drawLine(cursor_x + 2, cursor_y + 3, cursor_x + 4, cursor_y + 11);
}

void loop() {
  if (millis() - keepWakeUpTime > 60 * 1000) {
    // increasePrefCounter();
    deepSleep();
  }
  if (digitalRead(BtnPin) == 1) {
    powerOffButtonTime = 0;
    // u8x8.setInverseFont(1);
    readMPU9250();
    u8g2.clearBuffer();
    // u8g2.setFont(u8g2_font_6x10_tf);
    // u8g2_font_4x6_tf, u8g2_font_5x7_tf, u8g2_font_5x8_tf
    u8g2.drawStr(10, 10, "Hello,");
    u8g2.drawStr(10, 20, "DIY Stick Watch!");
    u8g2.drawStr(70, 1, reason);
    if (isTimeOK) {
      drawTime();
    }
    drawCursor();
    u8g2.sendBuffer();
  } else {
    unsigned long currTime = millis();
    keepWakeUpTime = currTime;
    if (powerOffButtonTime == 0) {
      powerOffButtonTime = currTime;
    }
    if (powerOffButtonTime != 0 && currTime - powerOffButtonTime > 1000) {
      Serial.println("long pressed, delay and going to sleep now");
      deepSleep();
    }
  }
}

void drawTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get local time, should not be here?");
  } else {
    char dateStringBuff[50]; //50 chars should be enough
    char timeStringBuff[50]; //50 chars should be enough
    strftime(dateStringBuff, sizeof(dateStringBuff), "%Y-%m-%d %a", &timeinfo);
    strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeinfo);
    // u8g2.setFont(u8g2_font_5x8_tf);
    u8g2.drawStr(40, 40, dateStringBuff);
    u8g2.drawStr(40, 50, timeStringBuff);
  }
}

void pureDeepSleep() {
  // https://esp32.com/viewtopic.php?t=3083
  // esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);

  // esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
  adc_power_off();
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, LOW); //1 = High, 0 = Low
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void deepSleep() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 30, "Deep sleep now...");
  u8g2.sendBuffer();
  delay(500);
  screenOffAnimation();
  pureDeepSleep();
}

void screenOffAnimation() {
  for (int i = 0; i < 32; i++) {
    u8g2.clearBuffer();
    u8g2.drawLine(0, i, 128, i);
    u8g2.drawLine(0, 64 - i, 128, 64 - i);
    u8g2.sendBuffer();
    delay(3);
  }
}
