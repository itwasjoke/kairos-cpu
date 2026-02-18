#include "universal_pid.h"
#include <math.h>

void PID_Init(PID_State_t *state) {
    state->integral = 0.0f;
    state->prevInput = 0.0f;
    state->lastOutput = 0.0f;
}

float PID_Compute(PID_Config_t *cfg, PID_State_t *state, float setpoint, float input) {
    // 1. Расчет ошибки
    float error = setpoint - input;

    // Если режим REVERSE, инвертируем ошибку
    if (cfg->direction == PID_REVERSE) {
        error = -error;
    }

    // 2. Проверка Deadband (зоны нечувствительности)
    if (fabsf(error) < cfg->deadband) {
        return state->lastOutput; // Ничего не меняем, если мы в цели
    }

    // 3. Адаптивность (подбор коэффициентов на лету)
    float active_Kp = cfg->Kp;
    float active_Ki = cfg->Ki;
    float active_Kd = cfg->Kd;

    // Если ошибка превышает порог агрессивности - меняем коэффициенты
    if (cfg->aggressiveErrorThreshold > 0 && fabsf(error) > cfg->aggressiveErrorThreshold) {
        active_Kp *= cfg->agg_Kp_Mult;
        active_Ki *= cfg->agg_Ki_Mult; // Обычно Ki зануляют или уменьшают
        active_Kd *= cfg->agg_Kd_Mult;
    }

    // 4. Основная математика

    // Пропорциональная часть
    float P = active_Kp * error;

    // Интегральная часть (с защитой от windup ДО сложения)
    // Считаем, каким стал бы интеграл
    float delta_I = active_Ki * error * cfg->T_sample;
    float temp_I = state->integral + delta_I;

    // Дифференциальная часть (Derivative on Measurement - чтобы не было рывка при смене уставки)
    // dInput = (Input - PrevInput) / dt
    float dInput = (input - state->prevInput);
    float D = 0.0f;
    if (cfg->T_sample > 0) {
       D = -active_Kd * (dInput / cfg->T_sample);
    }

    // Суммируем выход (P + I_temp + D)
    float output = P + temp_I + D;

    // 5. Ограничение выхода (Clamping) и умный интегратор
    // Если выход вышел за рамки, мы обрезаем его И НЕ НАКАПЛИВАЕМ интеграл
    if (output > cfg->outMax) {
        output = cfg->outMax;
        // Не сохраняем temp_I в state->integral, если ошибка тянет нас дальше в насыщение
        if (error < 0) state->integral = temp_I; // Разрешаем "выходить" из насыщения
    }
    else if (output < cfg->outMin) {
        output = cfg->outMin;
        if (error > 0) state->integral = temp_I; // Разрешаем "выходить" из насыщения
    }
    else {
        // Если мы в рабочем диапазоне - сохраняем интеграл
        state->integral = temp_I;
    }

    // Сохраняем состояние для следующего такта
    state->prevInput = input;
    state->lastOutput = output;

    return output;
}
