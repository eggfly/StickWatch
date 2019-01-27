
#include <Arduino.h>
#include <U8x8lib.h>

#define SensorPin 13

// U8x8 constructor for your display
U8X8_SH1107_64X128_4W_HW_SPI u8x8(14, /* dc=*/ 27, /* reset=*/ 33);

// Create a U8x8log object
U8X8LOG u8x8log;

// Define the dimension of the U8x8log window
#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8

// Allocate static memory for the U8x8log window
uint8_t u8log_buffer[U8LOG_WIDTH * U8LOG_HEIGHT];

void setup() {
  // Startup U8x8
  u8x8.begin();

  // Set a suitable font. This font will be used for U8x8log
  u8x8.setFont(u8x8_font_chroma48medium8_r);

  // Start U8x8log, connect to U8x8, set the dimension and assign the static memory
  u8x8log.begin(u8x8, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);

  // Set the U8x8log redraw mode
  u8x8log.setRedrawMode(1);    // 0: Update screen with newline, 1: Update screen for every char
  Serial.begin(115200);

  //Connect the pin set it to output
  pinMode(SensorPin, OUTPUT);
  ledcSetup(1, 38000, 10);
}

void loop()
{
  analogReadResolution(12);
  int dacValue = analogRead(SensorPin);
  float batteryVoltage = (20.0 + 68.0) / 68.0 * 3.3 * dacValue / 4096.0;
  Serial.printf("%.2f V\n", batteryVoltage);
  u8x8log.printf("%.2f V", batteryVoltage);
  u8x8log.print("\n");
  ledcWrite(1, ledcRead(1) ? 0 : 512);
  delay(100);
}
