#ifndef LED_H_
#define LED_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
#include "main.h"

// Типы светодиодов
typedef enum {
  LED_RS_TX, LED_RS_RX,
  LED_CAN_TX, LED_CAN_RX,
  LED_1,
	LED_2,
	LED_3,
	LED_4,
  LED_COUNT
} LedName_t;

// Инициализация сервиса
void LedService_Init(void);

// Функция "Мигнуть" для обычных LED
void Led_Blink(LedName_t led, uint32_t duration_ms);

// Функция для ШИМ LED (№1-6)
// Канал 1-4 на одном таймере, 1-2 на другом
void Led_PWM_Set(uint8_t led_num, uint8_t duty_percent);

#endif /* LED_H_ */
