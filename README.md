# StickWatch
A smart watch based on M5Stick of ESP32

https://item.taobao.com/item.htm?id=581055502939

Use ESP-idf + Arduino to build this project with configuration:
M5Stack-Core-ESP32, QIO, 80MHz, No OTA (Large APP), 921600, Verbose

* 为了避免"项目太大了"失败的问题，需要在Arduino IDE中选择菜单栏中的工具->Partition Scheme->Large App No OTA.

依赖：
- Wifi
- ArduinoJson 6.1.0-beta
