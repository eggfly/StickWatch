#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

const float MAX_CURSOR_ACC = 16;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

U8G2_SH1107_64X128_F_4W_HW_SPI u8g2(U8G2_R3, /* cs=*/ 14, /* dc=*/ 27, /* reset=*/ 33);

#define FONT u8g2_font_wqy12_t_gb2312

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
    u8g2.drawStr(40, 30, dateStringBuff);
    u8g2.drawStr(40, 40, timeStringBuff);
  }
}

void drawSleepText() {
  u8g2.clearBuffer();
  u8g2.drawStr(10, 30, "Deep sleep now...");
  u8g2.sendBuffer();
}

void drawHelloStickWatch() {
  u8g2.drawStr(6, 6, "Hello,");
  u8g2.drawStr(6, 16, "DIY Stick Watch!");
  u8g2.drawStr(70, 1, reason);
}

void drawBatteryStatus(bool isCharging, bool isChargeFull) {
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  // u8g2.setFont(u8g2_font_5x7_mf);
  if (isCharging) {
    u8g2.drawUTF8(1, 51, "充电中/");
  } else {
    u8g2.drawUTF8(1, 51, "未充电/");
  }
  if (isChargeFull) {
    u8g2.drawUTF8(42, 51, "充满了!");
  } else {
    u8g2.drawUTF8(42, 51, "未充满");
  }
  u8g2.setFont(u8g2_font_6x10_mf);
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

#endif // _SCREEN_H_
