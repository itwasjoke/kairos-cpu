#ifndef UNIVERSAL_PID_H
#define UNIVERSAL_PID_H

#include <stdint.h>
#include <stdbool.h>

// Режимы работы
typedef enum {
    PID_DIRECT,  // Выход++ -> Датчик++ (Нагрев, Свет)
    PID_REVERSE  // Выход++ -> Датчик-- (Охлаждение)
} PID_Direction_t;

// 1. КОНФИГУРАЦИЯ (Задается один раз)
typedef struct {
    // Базовые коэффициенты
    float Kp;
    float Ki;
    float Kd;

    // Лимиты выхода (для DAC STM32: 0..4095)
    float outMin;
    float outMax;

    // Направление
    PID_Direction_t direction;

    // Время одного такта (в секундах)
    float T_sample;

    // Адаптивность: порог ошибки для переключения коэффициентов
    // Если 0 - адаптивность выключена
    float aggressiveErrorThreshold;

    // Множители для агрессивного режима (например, 1.5 для P, 0.0 для I)
    float agg_Kp_Mult;
    float agg_Ki_Mult;
    float agg_Kd_Mult;

    // Зона нечувствительности (если ошибка меньше этого - выход не меняем)
    float deadband;

} PID_Config_t;

// 2. СОСТОЯНИЕ (Меняется каждый такт)
typedef struct {
    float integral;       // Накопленная сумма
    float prevInput;      // Предыдущее измерение (для D-члена без рывков)
    float lastOutput;     // Последний рассчитанный выход
} PID_State_t;

// Функции
void PID_Init(PID_State_t *state);
float PID_Compute(PID_Config_t *cfg, PID_State_t *state, float setpoint, float input);

#endif
