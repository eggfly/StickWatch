/* NEC remote infrared RMT example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "driver/periph_ctrl.h"
#include "soc/rmt_reg.h"

static const char* NEC_TAG = "NEC";

//CHOOSE SELF TEST OR NORMAL TEST
#define RMT_RX_SELF_TEST   0

/******************************************************/
/*****                SELF TEST:                  *****/
/*Connect RMT_TX_GPIO_NUM with RMT_RX_GPIO_NUM        */
/*TX task will send NEC data with carrier disabled    */
/*RX task will print NEC data it receives.            */
/******************************************************/
#if RMT_RX_SELF_TEST
#define RMT_TX_CARRIER_EN    0   /*!< Disable carrier for self test mode  */
#else
//Test with infrared LED, we have to enable carrier for transmitter
//When testing via IR led, the receiver waveform is usually active-low.
#define RMT_TX_CARRIER_EN    1   /*!< Enable carrier for IR transmitter test with IR led */
#endif

#define RMT_TX_CHANNEL    1     /*!< RMT channel for transmitter */
#define RMT_TX_GPIO_NUM  17     /*!< GPIO number for transmitter signal */
#define RMT_CLK_DIV      100    /*!< RMT counter clock divider */
#define RMT_TICK_10_US    (80000000/RMT_CLK_DIV/100000)   /*!< RMT counter value for 10 us.(Source clock is APB clock) */

#define NEC_HEADER_HIGH_US    9000                         /*!< NEC protocol header: positive 9ms */
#define NEC_HEADER_LOW_US     4500                         /*!< NEC protocol header: negative 4.5ms*/
#define NEC_BIT_ONE_HIGH_US    560                         /*!< NEC protocol data bit 1: positive 0.56ms */
#define NEC_BIT_ONE_LOW_US    (2250-NEC_BIT_ONE_HIGH_US)   /*!< NEC protocol data bit 1: negative 1.69ms */
#define NEC_BIT_ZERO_HIGH_US   560                         /*!< NEC protocol data bit 0: positive 0.56ms */
#define NEC_BIT_ZERO_LOW_US   (1120-NEC_BIT_ZERO_HIGH_US)  /*!< NEC protocol data bit 0: negative 0.56ms */
#define NEC_BIT_END            560                         /*!< NEC protocol end: positive 0.56ms */
#define NEC_BIT_MARGIN         20                          /*!< NEC parse margin time */

#define NEC_ITEM_DURATION(d)  ((d & 0x7fff)*10/RMT_TICK_10_US)  /*!< Parse duration time from memory register value */
#define NEC_DATA_ITEM_NUM   34  /*!< NEC code item number: header + 32bit data + end */
#define RMT_TX_DATA_NUM  1    /*!< NEC tx test data number */
#define rmt_item32_tIMEOUT_US  9500   /*!< RMT receiver timeout value(us) */

/*
   @brief Build register value of waveform for NEC one data bit
*/
static inline void nec_fill_item_level(rmt_item32_t* item, int high_us, int low_us)
{
  item->level0 = 1;
  item->duration0 = (high_us) / 10 * RMT_TICK_10_US;
  item->level1 = 0;
  item->duration1 = (low_us) / 10 * RMT_TICK_10_US;
}

/*
   @brief Generate NEC header value: active 9ms + negative 4.5ms
*/
static void nec_fill_item_header(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_HEADER_HIGH_US, NEC_HEADER_LOW_US);
}

/*
   @brief Generate NEC data bit 1: positive 0.56ms + negative 1.69ms
*/
static void nec_fill_item_bit_one(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_BIT_ONE_HIGH_US, NEC_BIT_ONE_LOW_US);
}

/*
   @brief Generate NEC data bit 0: positive 0.56ms + negative 0.56ms
*/
static void nec_fill_item_bit_zero(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_BIT_ZERO_HIGH_US, NEC_BIT_ZERO_LOW_US);
}

/*
   @brief Generate NEC end signal: positive 0.56ms
*/
static void nec_fill_item_end(rmt_item32_t* item)
{
  nec_fill_item_level(item, NEC_BIT_END, 0x7fff);
}

/*
   @brief Check whether duration is around target_us
*/
inline bool nec_check_in_range(int duration_ticks, int target_us, int margin_us)
{
  if (( NEC_ITEM_DURATION(duration_ticks) < (target_us + margin_us))
      && ( NEC_ITEM_DURATION(duration_ticks) > (target_us - margin_us))) {
    return true;
  } else {
    return false;
  }
}

/*
   @brief Build NEC 32bit waveform.
*/
static int nec_build_items(int channel, rmt_item32_t* item, int item_num, uint16_t addr, uint16_t cmd_data)
{
  int i = 0, j = 0;
  if (item_num < NEC_DATA_ITEM_NUM) {
    return -1;
  }
  nec_fill_item_header(item++);
  i++;
  for (j = 0; j < 16; j++) {
    if (addr & 0x1) {
      nec_fill_item_bit_one(item);
    } else {
      nec_fill_item_bit_zero(item);
    }
    item++;
    i++;
    addr >>= 1;
  }
  for (j = 0; j < 16; j++) {
    if (cmd_data & 0x1) {
      nec_fill_item_bit_one(item);
    } else {
      nec_fill_item_bit_zero(item);
    }
    item++;
    i++;
    cmd_data >>= 1;
  }
  nec_fill_item_end(item);
  i++;
  return i;
}

/*
   @brief RMT transmitter initialization
*/
static void nec_tx_init()
{
  rmt_config_t rmt_tx;
  rmt_tx.channel = (rmt_channel_t)RMT_TX_CHANNEL;
  rmt_tx.gpio_num = (gpio_num_t)RMT_TX_GPIO_NUM;
  rmt_tx.mem_block_num = 1;
  rmt_tx.clk_div = RMT_CLK_DIV;
  rmt_tx.tx_config.loop_en = false;
  rmt_tx.tx_config.carrier_duty_percent = 50;
  rmt_tx.tx_config.carrier_freq_hz = 38000;
  rmt_tx.tx_config.carrier_level = (rmt_carrier_level_t)1;
  rmt_tx.tx_config.carrier_en = RMT_TX_CARRIER_EN;
  rmt_tx.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  rmt_tx.tx_config.idle_output_en = true;
  rmt_tx.rmt_mode = RMT_MODE_TX;
  rmt_config(&rmt_tx);
  rmt_driver_install(rmt_tx.channel, 0, 0);
}



/**
   @brief RMT transmitter demo, this task will periodically send NEC data. (100 * 32 bits each time.)

*/
static void rmt_example_nec_tx_task(void*)
{
  vTaskDelay(10);
  nec_tx_init();
  esp_log_level_set(NEC_TAG, ESP_LOG_INFO);
  int channel = RMT_TX_CHANNEL;
  uint16_t cmd = 0x0;
  uint16_t addr = 0x11;
  int nec_tx_num = RMT_TX_DATA_NUM;
  for (;;) {
    ESP_LOGI(NEC_TAG, "RMT TX DATA, CMD=%d, ADDR=%d", cmd, addr);
    size_t size = (sizeof(rmt_item32_t) * NEC_DATA_ITEM_NUM * nec_tx_num);
    //each item represent a cycle of waveform.
    rmt_item32_t* item = (rmt_item32_t*) malloc(size);
    int item_num = NEC_DATA_ITEM_NUM * nec_tx_num;
    memset((void*) item, 0, size);
    int i, offset = 0;
    while (1) {
      //To build a series of waveforms.
      i = nec_build_items(channel, item + offset, item_num - offset, ((~addr) << 8) | addr, ((~cmd) << 8) |  cmd);
      if (i < 0) {
        break;
      }
      cmd++;
      addr++;
      offset += i;
    }
    for (int i = 0; i < item_num; i++) {
      rmt_item32_t rmt = item[i];
      ESP_LOGD(NEC_TAG, "%d: level0=%d, duration0=%d, level1=%d, duration1=%d", i, rmt.level0,  rmt.duration0, rmt.level1, rmt.duration1);
    }
    //To send data according to the waveform items.
    rmt_write_items((rmt_channel_t)channel, item, item_num, true);
    //Wait until sending is done.
    rmt_wait_tx_done((rmt_channel_t)channel, portMAX_DELAY);
    //before we free the data, make sure sending is already done.
    free(item);
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
  vTaskDelete(NULL);
}

void setup() {
  xTaskCreate(rmt_example_nec_tx_task, "rmt_nec_tx_task", 2048, NULL, 10, NULL);
}

void loop() {
}
