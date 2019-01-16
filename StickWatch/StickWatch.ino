
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

#include "MPU9250.h"
#include "quaternionFilters.h"

#include <WiFi.h>

#include <WiFiMulti.h>
#include <HTTPClient.h>
// #include <Int64String.h>

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 8 /* Time ESP32 will sleep while 32s is the IP5306 chip delay time */

#define ARDUINOJSON_USE_LONG_LONG 1
#include "ArduinoJson.h"

static const char *TAG = "example";

const char* ssid       = "MIWIFI8";
const char* password   = "12345678";

#define HUNDRED_NANO_SECONDS (1000*1000*10)
const long  gmtOffset_sec = 3600 * 8;


#define AHRS true         // Set to false for basic data read
#define SerialDebugMPU9250 false  // Set to true to get Serial output for debugging
#define SerialDebugCalibrate false  // Set to true to get Serial output for debugging

MPU9250 IMU;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BtnPin 35
#define LedPin 19
#define BuzzerPin 26

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

unsigned int increasePrefCounter () {
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
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER) {
    led(true, 100);
    pureDeepSleep();
  }

  ESP_LOGD(TAG, "0");
  Serial.begin(115200);
  Wire.begin();

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

void setupMPU9250() {
  byte c = IMU.readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);
  Serial.print("MPU9250 "); Serial.print("I AM "); Serial.print(c, HEX);
  Serial.print(" I should be "); Serial.println(0x71, HEX);
  if (c == 0x71) {
    Serial.println("MPU9250 is online...");
    // Start by performing self test and reporting values
    IMU.MPU9250SelfTest(IMU.SelfTest);
    if (SerialDebugCalibrate) {
      Serial.print("x-axis self test: acceleration trim within : ");
      Serial.print(IMU.SelfTest[0], 1); Serial.println("% of factory value");
      Serial.print("y-axis self test: acceleration trim within : ");
      Serial.print(IMU.SelfTest[1], 1); Serial.println("% of factory value");
      Serial.print("z-axis self test: acceleration trim within : ");
      Serial.print(IMU.SelfTest[2], 1); Serial.println("% of factory value");
      Serial.print("x-axis self test: gyration trim within : ");
      Serial.print(IMU.SelfTest[3], 1); Serial.println("% of factory value");
      Serial.print("y-axis self test: gyration trim within : ");
      Serial.print(IMU.SelfTest[4], 1); Serial.println("% of factory value");
      Serial.print("z-axis self test: gyration trim within : ");
      Serial.print(IMU.SelfTest[5], 1); Serial.println("% of factory value");
    }
    Serial.println("MPU9250 acceleration and gyration self test done!");

    // Calibrate gyro and accelerometers, load biases in bias registers
    IMU.calibrateMPU9250(IMU.gyroBias, IMU.accelBias);

    IMU.initMPU9250();
    // Initialize device for active mode read of acclerometer, gyroscope, and
    // temperature
    Serial.println("MPU9250 initialized for active data mode....");

    // Read the WHO_AM_I register of the magnetometer, this is a good test of
    // communication
    byte d = IMU.readByte(AK8963_ADDRESS, WHO_AM_I_AK8963);
    Serial.print("AK8963 "); Serial.print("I AM "); Serial.print(d, HEX);
    Serial.print(" I should be "); Serial.println(0x48, HEX);

    // Get magnetometer calibration from AK8963 ROM
    IMU.initAK8963(IMU.magCalibration);
    // Initialize device for active mode read of magnetometer
    Serial.println("AK8963 initialized for active data mode....");
    if (SerialDebugCalibrate) {
      Serial.println("Calibration values: ");
      Serial.print("X-Axis sensitivity adjustment value ");
      Serial.println(IMU.magCalibration[0], 2);
      Serial.print("Y-Axis sensitivity adjustment value ");
      Serial.println(IMU.magCalibration[1], 2);
      Serial.print("Z-Axis sensitivity adjustment value ");
      Serial.println(IMU.magCalibration[2], 2);
    }
    Serial.println("AK8963 magCalibration done!");
  }
}

unsigned long powerOffButtonTime = 0;
void readMPU9250() {

  // If intPin goes high, all data registers have new data
  // On interrupt, check if data ready interrupt
  if (IMU.readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)
  {
    IMU.readAccelData(IMU.accelCount);  // Read the x/y/z adc values
    IMU.getAres();

    // Now we'll calculate the accleration value into actual g's
    // This depends on scale being set
    IMU.ax = (float)IMU.accelCount[0] * IMU.aRes; // - accelBias[0];
    IMU.ay = (float)IMU.accelCount[1] * IMU.aRes; // - accelBias[1];
    IMU.az = (float)IMU.accelCount[2] * IMU.aRes; // - accelBias[2];

    IMU.readGyroData(IMU.gyroCount);  // Read the x/y/z adc values
    IMU.getGres();

    // Calculate the gyro value into actual degrees per second
    // This depends on scale being set
    IMU.gx = (float)IMU.gyroCount[0] * IMU.gRes;
    IMU.gy = (float)IMU.gyroCount[1] * IMU.gRes;
    IMU.gz = (float)IMU.gyroCount[2] * IMU.gRes;

    IMU.readMagData(IMU.magCount);  // Read the x/y/z adc values
    IMU.getMres();
    // User environmental x-axis correction in milliGauss, should be
    // automatically calculated
    IMU.magbias[0] = +470.;
    // User environmental x-axis correction in milliGauss TODO axis??
    IMU.magbias[1] = +120.;
    // User environmental x-axis correction in milliGauss
    IMU.magbias[2] = +125.;

    // Calculate the magnetometer values in milliGauss
    // Include factory calibration per data sheet and user environmental
    // corrections
    // Get actual magnetometer value, this depends on scale being set
    IMU.mx = (float)IMU.magCount[0] * IMU.mRes * IMU.magCalibration[0] -
             IMU.magbias[0];
    IMU.my = (float)IMU.magCount[1] * IMU.mRes * IMU.magCalibration[1] -
             IMU.magbias[1];
    IMU.mz = (float)IMU.magCount[2] * IMU.mRes * IMU.magCalibration[2] -
             IMU.magbias[2];
  } // if (readByte(MPU9250_ADDRESS, INT_STATUS) & 0x01)

  // Must be called before updating quaternions!
  IMU.updateTime();

  // Sensors x (y)-axis of the accelerometer is aligned with the y (x)-axis of
  // the magnetometer; the magnetometer z-axis (+ down) is opposite to z-axis
  // (+ up) of accelerometer and gyro! We have to make some allowance for this
  // orientationmismatch in feeding the output to the quaternion filter. For the
  // MPU-9250, we have chosen a magnetic rotation that keeps the sensor forward
  // along the x-axis just like in the LSM9DS0 sensor. This rotation can be
  // modified to allow any convenient orientation convention. This is ok by
  // aircraft orientation standards! Pass gyro rate as rad/s
  //  MadgwickQuaternionUpdate(ax, ay, az, gx*PI/180.0f, gy*PI/180.0f, gz*PI/180.0f,  my,  mx, mz);
  MahonyQuaternionUpdate(IMU.ax, IMU.ay, IMU.az, IMU.gx * DEG_TO_RAD,
                         IMU.gy * DEG_TO_RAD, IMU.gz * DEG_TO_RAD, IMU.my,
                         IMU.mx, IMU.mz, IMU.deltat);
  // Serial print and/or display at 0.5 s rate independent of data rates
  IMU.delt_t = millis() - IMU.count;

  if (IMU.delt_t > 100)
  {
    if (SerialDebugMPU9250)
    {
      Serial.print("ax = ");
      Serial.print((int)1000 * IMU.ax);
      Serial.print(" ay = ");
      Serial.print((int)1000 * IMU.ay);
      Serial.print(" az = ");
      Serial.print((int)1000 * IMU.az);
      Serial.println(" mg");

      Serial.print("gx = ");
      Serial.print( IMU.gx, 2);
      Serial.print(" gy = ");
      Serial.print( IMU.gy, 2);
      Serial.print(" gz = ");
      Serial.print( IMU.gz, 2);
      Serial.println(" deg/s");

      Serial.print("mx = ");
      Serial.print( (int)IMU.mx );
      Serial.print(" my = ");
      Serial.print( (int)IMU.my );
      Serial.print(" mz = ");
      Serial.print( (int)IMU.mz );
      Serial.println(" mG");

      //Serial.print("q0 = ");
      Serial.print(*getQ());
      // Serial.print(" qx = ");
      Serial.print(*(getQ() + 1));
      // Serial.print(" qy = ");
      Serial.print(*(getQ() + 2));
      // Serial.print(" qz = ");
      Serial.println(*(getQ() + 3));
    }

    IMU.yaw   = atan2(2.0f * (*(getQ() + 1) * *(getQ() + 2) + *getQ() *
                              *(getQ() + 3)), *getQ() * *getQ() + * (getQ() + 1) * *(getQ() + 1)
                      - * (getQ() + 2) * *(getQ() + 2) - * (getQ() + 3) * *(getQ() + 3));
    IMU.pitch = -asin(2.0f * (*(getQ() + 1) * *(getQ() + 3) - *getQ() *
                              *(getQ() + 2)));
    IMU.roll  = atan2(2.0f * (*getQ() * *(getQ() + 1) + * (getQ() + 2) *
                              *(getQ() + 3)), *getQ() * *getQ() - * (getQ() + 1) * *(getQ() + 1)
                      - * (getQ() + 2) * *(getQ() + 2) + * (getQ() + 3) * *(getQ() + 3));
    IMU.pitch *= RAD_TO_DEG;
    IMU.yaw   *= RAD_TO_DEG;
    // Declination of SparkFun Electronics (40°05'26.6"N 105°11'05.9"W) is
    //  8° 30' E  ± 0° 21' (or 8.5°) on 2016-07-19
    // - http://www.ngdc.noaa.gov/geomag-web/#declination
    IMU.yaw   -= 8.5;
    IMU.roll  *= RAD_TO_DEG;

    if (SerialDebugMPU9250) {
      // Serial.println("Yaw, Pitch, Roll: ");
      Serial.print(IMU.yaw, 2);
      Serial.print(", ");
      Serial.print(IMU.pitch, 2);
      Serial.print(", ");
      Serial.println(IMU.roll, 2);
    }

    IMU.count = millis();
    IMU.sumCount = 0;
    IMU.sum = 0;

  } // if (IMU.delt_t > 100)
}

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
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
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
