#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    3300        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

static const adc_unit_t unit = ADC_UNIT_1;

static esp_adc_cal_characteristics_t *adc_chars;
static const adc_channel_t channel = ADC_CHANNEL_6;     //GPIO34 if ADC1, GPIO14 if ADC2
static const adc_atten_t atten = ADC_ATTEN_DB_11;

static void print_char_val_type(esp_adc_cal_value_t val_type)
{
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
    printf("Characterized using Two Point Value\n");
  } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    printf("Characterized using eFuse Vref\n");
  } else {
    printf("Characterized using Default Vref\n");
  }
}

void setup()
{
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten((adc1_channel_t)channel, atten);


  //Characterize ADC
  adc_chars = (esp_adc_cal_characteristics_t*)calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  //Continuously sample ADC1
  while (1) {
    uint32_t adc_reading = 0;
    //Multisampling
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
      if (unit == ADC_UNIT_1) {
        adc_reading += adc1_get_raw((adc1_channel_t)channel);
      } else {
        int raw;
        adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
        adc_reading += raw;
      }
    }
    adc_reading /= NO_OF_SAMPLES;
    //Convert adc_reading to voltage in mV
    uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
void loop() {
}
