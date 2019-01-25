#ifndef _IRQ_H_
#define _IRQ_H_

#include "config.h"

// declare key event callback function
void onKeyDown();
void onKeyUp();


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
void __handleButtonIRQ() {
  int down = digitalRead(BtnPin) == LOW;
  if (down && !__button_down) {
    onKeyDown();
  }
  if (!down && __button_down) {
    onKeyUp();
  }
  __button_down = down;
}

static
void handleIRQ() {
  if (__button_irq) {
    __button_irq = 0;
    __handleButtonIRQ();
  }
}

#endif // _IRQ_H_
