
#define LedPin 19

#define BuzzerPin 26

#define BtnPin 35

void setup() {
  pinMode(LedPin, OUTPUT);
  pinMode(BuzzerPin, OUTPUT);
  pinMode(BtnPin, INPUT);
  Serial.begin(115200);

  // buzzer beep
  for (int i = 0; i < 200; i++) {
    digitalWrite(BuzzerPin, HIGH);
    delayMicroseconds(2);
    digitalWrite(BuzzerPin, LOW);
    delayMicroseconds(1998);
  }

  // led flash
  for (int i = 0; i < 20; i++) {
    digitalWrite(LedPin, 1 - digitalRead(LedPin));
    delay(500);
  }
  //Increment boot number and print it every reboot

  // esp_sleep_enable_ext0_wakeup(GPIO_NUM_33, 1); //1 = High, 0 = Low

  //If you were to use ext1, you would use it like
  //esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK,ESP_EXT1_WAKEUP_ANY_HIGH);

  //Go to sleep now
  Serial.println("Going to sleep now");
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  //This is not going to be called
}
