#ifndef _IO_H_
#define _IO_H_

#include "config.h"

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile int interruptCounter = 0;

// declare key event callback function
void onKeyDown();
void onKeyUp();

void setPinModes() {
  // pinMode(BtnPin, INPUT_PULLDOWN);  // m5stick其实硬件上自带了 外部上拉电阻，貌似可以不要下面这行
  pinMode(BtnPin, INPUT_PULLUP);
  pinMode(LedPin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);

  pinMode(BatterySensorPin, INPUT);
  // analogReadResolution(12); // no need
}

void readBatteryVoltage(float *batteryVoltage, int *dacValue) {
  int sum = 0;
  for (int i = 0; i < SamplesCount; i++) {
    sum += analogRead(BatterySensorPin);
  }
  int value = sum / SamplesCount;
  *dacValue = value;
  *batteryVoltage = (20.0 + 68.0) / 68.0 * 3.3 * value / 4096.0;
}

static const float levels[] = {4.20, 4.06, 3.98, 3.92, 3.87, 3.82, 3.79, 3.77, 3.74, 3.68, 3.45, 3.00};

float getBatteryLevel(float voltage) {
  float level = 1;
  if (voltage >= levels[0]) {
    level = 1;
  } else if (voltage >= levels[1]) {
    level = 0.9;
    level += 0.1 * (voltage - levels[1]) / (levels[0] - levels[1]);
  } else if (voltage >= levels[2]) {
    level = 0.8;
    level += 0.1 * (voltage - levels[2]) / (levels[1] - levels[2]);
  } else if (voltage >= levels[3]) {
    level = 0.7;
    level += 0.1 * (voltage - levels[3]) / (levels[2] - levels[3]);
  } else if (voltage >= levels[4]) {
    level = 0.6;
    level += 0.1 * (voltage - levels[4]) / (levels[3] - levels[4]);
  } else if (voltage >= levels[5]) {
    level = 0.5;
    level += 0.1 * (voltage - levels[5]) / (levels[4] - levels[5]);
  } else if (voltage >= levels[6]) {
    level = 0.4;
    level += 0.1 * (voltage - levels[6]) / (levels[5] - levels[6]);
  } else if (voltage >= levels[7]) {
    level = 0.3;
    level += 0.1 * (voltage - levels[7]) / (levels[6] - levels[7]);
  } else if (voltage >= levels[8]) {
    level = 0.2;
    level += 0.1 * (voltage - levels[8]) / (levels[7] - levels[8]);
  } else if (voltage >= levels[9]) {
    level = 0.1;
    level += 0.1 * (voltage - levels[9]) / (levels[8] - levels[9]);
  } else if (voltage >= levels[10]) {
    level = 0.05;
    level += 0.05 * (voltage - levels[10]) / (levels[9] - levels[10]);
  } else if (voltage >= levels[11]) {
    level = 0.00;
    level += 0.05 * (voltage - levels[11]) / (levels[10] - levels[11]);
  } else {
    level = 0.00;
  }
  return level;
}

/**
  void IRAM_ATTR handle_btn_isr() {
  // TODO need mutex?
  //portENTER_CRITICAL_ISR(&mux);
  if (digitalRead(BtnPin) == LOW) {
    Serial.println("handle_btn_isr: button key down");
    onKeyDown();
  } else {
    Serial.println("handle_btn_isr: button key up");
    onKeyUp();
  }
  interruptCounter++;
  //portEXIT_CRITICAL_ISR(&mux);
  }

  void attachButtonEvent() {
  attachInterrupt(digitalPinToInterrupt(BtnPin), handle_btn_isr, CHANGE);
  }
*/

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

#endif // _IO_H_
