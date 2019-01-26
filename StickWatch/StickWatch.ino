#include "sensor.h"
#include "temp_sensor.h"
#include "irq.h"
#include "io.h"
#include "pm.h"
#include "screen.h"
#include "config.h"
#include "network.h"
#include "http_requests.h"
#include "utils.h"

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
  // lightness
  u8g2.setContrast(32);

  setPinModes(); // pin modes must be set after u8g2
  ESP_LOGD(TAG, "2");
  // u8g2.fillDisplay();
  u8g2.setFont(NORMAL_FONT);
  // u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFontRefHeightExtendedText();
  // u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  // u8g2.setFontDirection(0);
  ESP_LOGD(TAG, "3");
  print_wakeup_reason();
  boolean isBootFromReset = wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED;
  showSplashScreen(isBootFromReset);
  // buzzer();
  // flashLed(1000);
  setupMPU9250();
  wifiMulti.addAP(ssid, password);
  wifiMulti.addAP("eggfly", "12345678");
  syncTimeFromWifi(isBootFromReset);
  // attachButtonEvent();
  initIRQ();
}

unsigned long powerOffButtonTime = 0;
unsigned long keepWakeUpTime = 0;
unsigned long lastUpdateChargingTime = 0;
bool isLastChargeFull = false;
bool isLastCharging = false;
int temp_farenheit = 0;

unsigned long fps_prev_time;

uint8_t cursor_x, cursor_y;

class Page {
  public:
    virtual void onKeyDown(uint8_t x, uint8_t y) {};
    virtual void onKeyUp(uint8_t x, uint8_t y);
    virtual void onDraw() {};
};

class HomePage: public Page {
  public:
    virtual void onKeyDown(uint8_t x, uint8_t y) {
      Serial.printf("HomePage: key down: x=%d, y=%d\n", x, y);
    }
    virtual void onKeyUp(uint8_t x, uint8_t y);
    virtual void onDraw() {
      drawScreenBackground();
      // drawHelloStickWatch();
      if (millis() - lastUpdateChargingTime > 1000) {
        lastUpdateChargingTime = millis();
        isLastChargeFull = isChargeFull();
        isLastCharging = isCharging();
        temp_farenheit = read_temp();
      }
      // TODO
      drawBatteryStatusWithFont(isLastCharging, isLastChargeFull);
      // u8g2.setFont(u8g2_font_6x10_tf);
      // u8g2_font_4x6_tf, u8g2_font_5x7_tf, u8g2_font_5x8_tf
      if (isTimeOK) {
        drawTime();
      }
      get_cursor_position(&cursor_x, &cursor_y);
      drawChipTemprature(temp_farenheit);
      // must draw cursor in the end
      drawCursor(cursor_x, cursor_y);
    }
};


class GamePage: public Page {
  public:
    virtual void onDraw();
};

class BashPage: public Page {
  public:
    float scroll_y = -SCREEN_HEIGHT; // stay for a screen time
    virtual void onDraw();
};

class Ball {
  public:
    float x, y;
    float speed_x, speed_y;
    Ball(): x(SCREEN_WIDTH / 2), y(SCREEN_HEIGHT / 2), speed_x(0.5), speed_y(0.5) {}
    void flyAsTimeGoesBy() {
      x += speed_x;
      y += speed_y;
      // 撞墙
      if (x <= 0 || x >= SCREEN_WIDTH) {
        speed_x = -speed_x;
      }
      if (y <= 0 || y >= SCREEN_HEIGHT ) {
        speed_y = -speed_y;
      }
    }

    void getCurrentPosition(uint8_t *x, uint8_t *y) {
      *x = this->x; *y = this->y;
    }
};

HomePage homePage;

GamePage gamePage;
Ball ball;

BashPage bashPage;

Page * currentPage = & homePage;

void HomePage::onKeyUp(uint8_t x, uint8_t y) {
  Serial.printf("HomePage: key up: x=%d, y=%d\n", x, y);
  if (0 <= x && x < 24 && 46 < y && y < SCREEN_HEIGHT) {
    Serial.println("navigating to GamePage!");
    currentPage = &gamePage;
  } else if (55 <= x && x <= 76 && 46 < y && y < SCREEN_HEIGHT) {
    Serial.println("navigating to BashPage!");
    currentPage = &bashPage;
  }
}

void Page::onKeyUp(uint8_t x, uint8_t y) {
  Serial.printf("GamePage: key up: x=%d, y=%d\n", x, y);
  Serial.println("navigating to HomePage!");
  currentPage = &homePage;
}

void GamePage::onDraw() {
  uint8_t ball_x, ball_y;
  ball.getCurrentPosition(&ball_x, &ball_y);
  u8g2.drawCircle(ball_x, ball_y, 2.5);
  ball.flyAsTimeGoesBy();
}

void BashPage::onDraw() {
  scroll_y += 0.04;
  float stay_scroll_y = 0;
  if (scroll_y >= 0) {
    stay_scroll_y = scroll_y;
  }
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);  // all glyphs available
  u8g2.drawUTF8(30, -stay_scroll_y, "程序员老黄历");
  int title_height = 13;
  int font_height = 13;
  u8g2.drawUTF8(1, 20 - stay_scroll_y, " 宜 ");
  u8g2.drawHLine(0, title_height - scroll_y, SCREEN_WIDTH - 1);
  u8g2.drawHLine(0, 38 - stay_scroll_y, SCREEN_WIDTH - 1);
  // draw vertical line
  u8g2.drawLine(26, title_height - stay_scroll_y, 26, SCREEN_HEIGHT - 1);
  u8g2.drawUTF8(60, title_height + 1 - stay_scroll_y, "重构 代码质量得到提高");
  u8g2.drawUTF8(60, title_height + 1 + font_height - stay_scroll_y, "加班 晚上的精神是最好");
  u8g2.drawUTF8(1, 45 - stay_scroll_y, "不宜");
  u8g2.setFont(NORMAL_FONT);
}

void loop() {
  // lost frame detect
  unsigned long curr_time = millis();
  unsigned long delta = curr_time - fps_prev_time;
  if (delta > 20) {
    Serial.print("some frames lost, time delta: ");
    Serial.print(delta);
    Serial.println("ms");
  }
  fps_prev_time = curr_time;

  // auto sleep only when not charging
  if (!isLastCharging && curr_time - keepWakeUpTime > 60 * 1000) {
    // increasePrefCounter();
    deepSleep();
  }
  if (digitalRead(BtnPin) == HIGH) {
    powerOffButtonTime = 0;
    // u8x8.setInverseFont(1);
    readMPU9250();
    u8g2.clearBuffer();
    // render current page
    currentPage->onDraw();
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

  handleIRQ(cursor_x, cursor_y);
}

void deepSleep() {
  drawSleepText();
  delay(500);
  screenOffAnimation();
  pureDeepSleep();
}

void onKeyUp(uint8_t x, uint8_t y) {
  currentPage->onKeyUp(x, y);
}

void onKeyDown(uint8_t x, uint8_t y) {
  currentPage->onKeyDown(x, y);
}
