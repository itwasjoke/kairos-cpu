#include "led.h"

// Структура для управления обычными LED
typedef struct {
  GPIO_TypeDef* port;
  uint16_t pin;
  uint32_t timer; // Сколько мс осталось гореть
} Led_State_t;

static Led_State_t leds[LED_COUNT] = {
  {LED_RS_TX_GPIO_Port, LED_RS_TX_Pin, 0}, // RS_TX
  {LED_RS_RX_GPIO_Port, LED_RS_RX_Pin, 0}, // RS_RX
  {LED_CAN_TX_GPIO_Port, LED_CAN_TX_Pin, 0}, // CAN_TX
  {LED_CAN_RX_GPIO_Port, LED_CAN_RX_Pin, 0}, // CAN_RX
  {LED1_GPIO_Port, LED1_Pin, 0}, // Светодиод №1
	{LED2_GPIO_Port, LED2_Pin, 0}, // Светодиод №2
	{LED3_GPIO_Port, LED3_Pin, 0}, // Светодиод №3
	{LED4_GPIO_Port, LED4_Pin, 0}, // Светодиод №4
};

// Внешние дескрипторы таймеров (настройте свои)
extern TIM_HandleTypeDef htim8; // 4 канала
extern TIM_HandleTypeDef htim9; // 2 канала

void LedService_Init(void) {
  // Запуск ШИМ на всех каналах
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim8, TIM_CHANNEL_4);
  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
}

void Led_Blink(LedName_t led, uint32_t duration_ms) {
  if (led < LED_COUNT) {
    leds[led].timer = duration_ms;
    HAL_GPIO_WritePin(leds[led].port, leds[led].pin, GPIO_PIN_SET);
  }
}

void Led_PWM_Set(uint8_t led_num, uint8_t duty_percent) {
  if (duty_percent > 100) duty_percent = 100;

  switch(led_num) {
    case 1: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_1, duty_percent); break;
    case 2: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_2, duty_percent); break;
    case 3: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, duty_percent); break;
    case 4: __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_4, duty_percent); break;
    case 5: __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, duty_percent); break;
    case 6: __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, duty_percent); break;
  }
}

// Задача FreeRTOS
void LedServiceTask(void *argument) {
  const uint32_t step = 20; // Шаг обработки 20мс

  for(;;) {
    for (int i = 0; i < LED_COUNT; i++) {
      if (leds[i].timer > 0) {
        if (leds[i].timer <= step) {
          leds[i].timer = 0;
          HAL_GPIO_WritePin(leds[i].port, leds[i].pin, GPIO_PIN_RESET);
        } else {
          leds[i].timer -= step;
        }
      }
    }
    osDelay(step);
  }
}
