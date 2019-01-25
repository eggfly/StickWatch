#ifndef _IRQ_H_
#define _IRQ_H_

#include "config.h"

// declare key event callback function
void onKeyDown(uint8_t x, uint8_t y);
void onKeyUp(uint8_t x, uint8_t y);

static volatile int __button_irq = 0;

static
IRAM_ATTR
void __irq_button() {
  __button_irq = 1;
}

static
void initIRQ() {
  attachInterrupt(digitalPinToInterrupt(BtnPin), __irq_button, CHANGE);
}

static int __button_down = 0;
static
void __handleButtonIRQ(uint8_t x, uint8_t y) {
  int down = digitalRead(BtnPin) == LOW;
  if (down && !__button_down) {
    onKeyDown(x, y);
  }
  if (!down && __button_down) {
    onKeyUp(x, y);
  }
  __button_down = down;
}

static
void handleIRQ(uint8_t x, uint8_t y) {
  if (__button_irq) {
    __button_irq = 0;
    __handleButtonIRQ(x, y);
  }
}

#endif // _IRQ_H_
