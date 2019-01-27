#ifndef _SOME_H_
#define _SOME_H_

class Page {
  public:
    virtual void onKeyDown(uint8_t x, uint8_t y) {};
    virtual void onKeyUp(uint8_t x, uint8_t y);
    virtual void onDraw() {};
};

#endif // _SOME_H_
