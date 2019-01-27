#ifndef _CONFIG_H_
#define _CONFIG_H_

// user configurable variables start
const char* ssid       = "MIWIFI8";
const char* password   = "12345678";
// 光标敏感度
const float MAX_CURSOR_ACC = 12;
// user configurable variables end

// program internal config start
#define BtnPin 35
#define LedPin 19
#define BuzzerPin 26

#define BatterySensorPin 34

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define HUNDRED_NANO_SECONDS (1000*1000*10)
const long  gmtOffset_sec = 3600 * 8;

#define NORMAL_FONT u8g2_font_6x10_mf

// static const char *TAG = "example";
// program internal config end

#endif // _CONFIG_H_
