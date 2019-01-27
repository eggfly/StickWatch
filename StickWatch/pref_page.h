#ifndef _PAGE_H_
#define _PAGE_H_

#include "page.h"

class PrefPage: public Page {
  public:
    virtual void onDraw();
};

float batteryVoltage = 0;
int dacValue = 0;
unsigned long batteryVoltageUpdateTime = 0;

void PrefPage::onDraw() {
  char buf[50];
  char percent_buf[50];
  unsigned long currentTime = millis();
  if (currentTime - batteryVoltageUpdateTime >= 1000) {
    batteryVoltageUpdateTime = currentTime;
    readBatteryVoltage(&batteryVoltage, &dacValue);
    Serial.printf("battery update: %.2fV, dacValue=%d\n", batteryVoltage, dacValue);
  }
  sprintf(buf, "Battery: %.2fV (%d)", batteryVoltage, dacValue);
  sprintf(percent_buf, "Battery percent: %.0f%%", getBatteryLevel(batteryVoltage) * 100);
  u8g2.drawStr(1, 1, buf);
  u8g2.drawStr(1, 10, percent_buf);
}

#endif // _PAGE_H_
