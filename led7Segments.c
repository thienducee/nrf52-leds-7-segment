#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "nordic_common.h"
#include "nrf_gpio.h"
#include "led7Segments.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define TIMER_PERIOD      100          /**< Timer period. LED7_SEGMENT timer will expire after 100 ms */

#define LED_A 5
#define LED_B 6
#define LED_C 7
#define LED_D 8
#define LED_E 9
#define LED_F 10
#define LED_G 11
#define PWR_SEG_ONE 0
#define PWR_SEG_TWO 1
#define PWR_SEG_THREE 2
#define PWR_SEG_FOUR 3

#define LEDS_LIST {LED_A, LED_B, LED_C, LED_D, LED_E, LED_F, LED_G}
#define LED7_LIST {PWR_SEG_ONE, PWR_SEG_TWO, PWR_SEG_THREE, PWR_SEG_FOUR}

#define LEDS_NUMBER 7
#define LED7S_NUMBER 4


TimerHandle_t led7_segments_timer_handle;  /**< LED7_SEGMENT using FreeRTOS timer. */

static const uint8_t leds_list[LEDS_NUMBER] = LEDS_LIST;
static const uint8_t led7s_list[LED7S_NUMBER] = LED7_LIST;
static const uint8_t chars[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};

static int display_number = 0;
static uint8_t led7s_index = 0;
static uint8_t led7_state = 0x00;



static void led7_set_led_segment(uint8_t char_code) {
  uint8_t i;
  //clear pin
  for(i = 0; i < LEDS_NUMBER; i++) {
    nrf_gpio_pin_write(leds_list[i], 0);
  }

  for(i = 0; i < LEDS_NUMBER; i++) {
    nrf_gpio_pin_write(leds_list[i], char_code & (1 << (i)));
  }
}

static void led7_setup_pin_out(void) {
  uint8_t i;
  for(i = 0; i < LEDS_NUMBER; i++) {
    nrf_gpio_cfg_output(leds_list[i]);
    nrf_gpio_pin_write(leds_list[i], 1);
  }

    nrf_gpio_pin_write(PWR_SEG_ONE, 1);
    nrf_gpio_pin_write(PWR_SEG_TWO, 1);
}

static uint8_t get_part_of_number(int display_number, uint8_t index) {
  uint8_t number = 0;
  if (LED7S_NUMBER == 1) {
  } else if (LED7S_NUMBER == 2) {
      switch(index) {
        case 1:
          number = display_number/10;
          break;
        case 2:
          number = display_number%10;
          break;
      }
  } else if (LED7S_NUMBER == 3) {
      switch(index) {
        case 1:
          number = display_number/100;
          break;
        case 2:
          number = (display_number%100)/10;
          break;
        case 3:
          number = (display_number%100)%10;
          break;
      }
  } else if (LED7S_NUMBER == 4) {
      switch(index) {
        case 1:
          number = display_number/1000;
          break;
        case 2:
          number = (display_number%1000)/100;
          break;
        case 3:
          number = ((display_number%1000)%100)/10;
          break;
        case 4:
          number = ((display_number%1000)%100)%10;
          break;
      }
  }
  return number;
}

void set_display_number(int value) {
  display_number = value;
}

static void led7_segments_timer_callback (void * pvParameter)
{
    UNUSED_PARAMETER(pvParameter);

    //ASSERT(led7s_index < LED7S_NUMBER);
    if(led7s_index < LED7S_NUMBER) {

      NRF_LOG_INFO("led7 index :%d", led7s_index);
      led7_state = 1 << led7s_index;
      NRF_LOG_RAW_INFO("led7_state : 0x%x\r\n", led7_state);
      //clear pin
      for(uint8_t i = 0; i < LED7S_NUMBER; i++) {
        nrf_gpio_pin_write(led7s_list[i], 0);
      }

      for(uint8_t i = 0; i < LED7S_NUMBER; i++) {
        nrf_gpio_pin_write(led7s_list[i], led7_state & (1 << (i)));
      }
      uint8_t number_part = get_part_of_number(display_number, led7s_index + 1);

      led7_set_led_segment(chars[number_part]);
      NRF_LOG_INFO("number part led7s_index = %d, value = %d", led7s_index + 1, number_part);
      led7s_index++;
    } else { 
        led7s_index = 0;
    }
}

void led7_intit() {
  NRF_LOG_INFO("LED7 init");
  led7_setup_pin_out();
  /* Start timer for LED7_SEGMENT */
  led7_segments_timer_handle = xTimerCreate( "LED7_SEGMENT", TIMER_PERIOD, pdTRUE, NULL, led7_segments_timer_callback);
  UNUSED_VARIABLE(xTimerStart(led7_segments_timer_handle, 0));
  set_display_number(12);
}