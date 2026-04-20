#ifndef KAIROS_PID_H
#define KAIROS_PID_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "Config.h"
#include "stm32f4xx.h"

// --- Структура памяти ПИД-регулятора (RAM) ---
typedef struct {
    float integralSum;    // Накопленная сумма для И-звена
    float prevError;      // Предыдущая ошибка для Д-звена
    bool isFirstRun;      // Флаг первого запуска для мягкого старта
} PidState_t;

// --- Структура конечного автомата Автотюнинга ---
typedef enum {
    TUNE_STATE_IDLE = 0,
    TUNE_STATE_STEP_UP,
    TUNE_STATE_STEP_DOWN,
    TUNE_STATE_CALCULATE
} AutoTuneState_e;

// Состояние готовности регулятора
typedef enum {
    PID_STATE_WAIT_FOR_TUNE, // Ждем нажатия кнопки/команды "Старт тюнинга"
    PID_STATE_TUNING,        // В процессе автонастройки
    PID_STATE_READY          // Коэффициенты есть, работаем
} PidWorkStatus_e;

extern volatile PidWorkStatus_e currentPidWorkStatus;

typedef struct {
    AutoTuneState_e state;
    float relayStepPos;   // Значение выхода при шаге вверх
    float relayStepNeg;   // Значение выхода при шаге вниз
    float maxPv;          // Максимальное значение обратной связи
    float minPv;          // Минимальное значение обратной связи
    uint32_t startTime;   // Время начала цикла (в мс)
    uint32_t crossTime;   // Время пересечения уставки (в мс)
    float Ku;             // Критический коэффициент усиления
    float Tu;             // Период автоколебаний
    uint8_t cycles;       // Количество пройденных циклов колебаний
} AutoTuneState_t;

// --- Прототипы функций ---
void Kairos_PID_Init(PidState_t *state);
void Kairos_PID_Compute(KairosConfig_t *config, ProjectVars_t *vars, PidState_t *state);
void Kairos_TIM4_Init(float dTime);

void Kairos_AutoTune_Init(AutoTuneState_t *tuneState, float stepUp, float stepDown);
bool Kairos_AutoTune_Process(KairosConfig_t *config, ProjectVars_t *vars, AutoTuneState_t *tuneState, uint32_t currentTimeMs);

#endif // KAIROS_PID_H
