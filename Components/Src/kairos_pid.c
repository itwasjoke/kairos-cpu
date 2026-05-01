#include "kairos_pid.h"

extern TIM_HandleTypeDef htim7;
volatile PidWorkStatus_e currentPidWorkStatus = PID_STATE_WAIT_FOR_TUNE;

void Kairos_PID_Init(PidState_t *state) {
    state->integralSum = 0.0f;
    state->prevError = 0.0f;
    state->isFirstRun = true;
}

void Kairos_AutoTune_Init(AutoTuneState_t *tuneState, float stepUp, float stepDown) {
    tuneState->state = TUNE_STATE_IDLE;
    tuneState->relayStepPos = stepUp;
    tuneState->relayStepNeg = stepDown;
    tuneState->cycles = 0;
    tuneState->maxPv = -100000.0f; // Специально задаем крайние значения для поиска экстремумов
    tuneState->minPv = 100000.0f;
}

// Вспомогательная функция для ограничения значений осталась без изменений
static float Clamp(float value, float min, float max) {
    if (value > max) return max;
    if (value < min) return min;
    return value;
}

void Kairos_PID_Compute(KairosConfig_t *config, ProjectVars_t *vars, PidState_t *state) {
    RegulatorConfig_t *rc = &config->regulator_config;

    float setpoint = vars->vars[rc->setpointIndex].as_float;
    float feedback = vars->vars[rc->feedbackIndex].as_float;

    // 1. УЧЕТ НАПРАВЛЕНИЯ РЕГУЛИРОВАНИЯ
    // 0 - Прямое (охлаждение/сброс), 1 - Обратное (нагрев/затемнение)
    float error = (rc->direction == 0) ? (setpoint - feedback) : (feedback - setpoint);
    float absError = fabsf(error);

    // 2. МЕРТВАЯ ЗОНА (Теперь в абсолютных единицах, например 0.05В)
    if (absError <= rc->sensZone) {
        error = 0.0f;
        absError = 0.0f;
    }

    // 3. АДАПТИВНАЯ ЛОГИКА (Gain Scheduling)
    float currentKp = rc->Kp;
    float currentKi = rc->Ki;
    float currentKd = rc->Kd;

    // Считаем пороги. Если в настройках 0, то адаптивность для этой зоны выключена.
    // Пороги считаем в % от уставки, чтобы сохранить логику твоих переменных maxErr/minErr.
    float maxErrThreshold = (rc->maxErr > 0) ? ((rc->maxErr / 100.0f) * fabsf(setpoint)) : -1.0f;
    float minErrThreshold = (rc->minErr > 0) ? ((rc->minErr / 100.0f) * fabsf(setpoint)) : -1.0f;

    if (maxErrThreshold > 0 && absError > maxErrThreshold) {
        // ЗОНА 1: "Форсаж" (Ошибка очень большая)
        // Задача: Быстро долететь до рабочей зоны.
        // Действие: Увеличиваем P для скорости. I уменьшаем в 2 раза, но НЕ ВЫКЛЮЧАЕМ.
        // Это копит интеграл, но защищает от эффекта взведенной пружины (windup), пока мы летим.
        currentKp *= 1.5f;
        currentKi *= 0.5f;
    }
    else if (minErrThreshold > 0 && absError < minErrThreshold && absError > 0.0f) {
        // ЗОНА 3: "Снайперская доводка" (Мы в миллиметре от цели)
        // Задача: Плавно лечь на уставку без перелета и подавить шум датчика.
        // Действие: P снижаем, чтобы выход не дергался от микро-шумов. I форсируем,
        // чтобы быстро и плавно "съесть" остаточную статическую ошибку.
        currentKp *= 0.6f;
        currentKi *= 1.2f;
    }
    // ЗОНА 2: "Крейсерская" (Ошибка между min и max, либо адаптивность выключена нулями).
    // Работают твои классические заданные коэффициенты.

    // 4. ПРОПОРЦИОНАЛЬНАЯ ЧАСТЬ
    float P_out = currentKp * error;

    // 5. ИНТЕГРАЛЬНАЯ ЧАСТЬ (со строгим Anti-Windup)
    if (!state->isFirstRun && currentKi > 0.0f) {
        float integralStep = error * currentKi * rc->dTime;
        state->integralSum += integralStep;

        // Важнейший момент: жестко зажимаем сумму интеграла в границах выхода.
        // Это гарантирует, что если лампа на максимуме, а ошибка еще есть,
        // регулятор не "улетит" в бесконечность и сможет мгновенно пойти назад.
        state->integralSum = Clamp(state->integralSum, rc->minValue, rc->maxValue);
    }

    // 6. ДИФФЕРЕНЦИАЛЬНАЯ ЧАСТЬ
    float D_out = 0.0f;
    if (!state->isFirstRun && rc->dTime > 0.0f) {
        D_out = currentKd * (error - state->prevError) / rc->dTime;
    }

    // 7. СУММАТОР И ИТОГОВЫЙ ВЫХОД
    float output = P_out + state->integralSum + D_out;

    // Обрезаем финальный выход (на случай, если P или D дали резкий выброс)
    vars->vars[rc->outputIndex].as_float = Clamp(output, rc->minValue, rc->maxValue);

    // 8. СОХРАНЕНИЕ СОСТОЯНИЯ
    state->prevError = error;
    state->isFirstRun = false;
}

bool Kairos_AutoTune_Process(KairosConfig_t *config, ProjectVars_t *vars, AutoTuneState_t *tuneState, uint32_t currentTimeMs) {
    RegulatorConfig_t *rc = &config->regulator_config;
    float setpoint = vars->vars[rc->setpointIndex].as_float;
    float feedback = vars->vars[rc->feedbackIndex].as_float;

    if (feedback > tuneState->maxPv) tuneState->maxPv = feedback;
    if (feedback < tuneState->minPv) tuneState->minPv = feedback;

    switch (tuneState->state) {
        case TUNE_STATE_IDLE:
            vars->vars[rc->outputIndex].as_float = tuneState->relayStepPos;
            tuneState->startTime = currentTimeMs;
            tuneState->state = TUNE_STATE_STEP_UP;
            break;

        case TUNE_STATE_STEP_UP:
            // УЧЕТ НАПРАВЛЕНИЯ: При прямом ждем превышения, при обратном - падения
            if ((rc->direction == 0 && feedback > setpoint) ||
                (rc->direction == 1 && feedback < setpoint)) {
                vars->vars[rc->outputIndex].as_float = tuneState->relayStepNeg;
                tuneState->crossTime = currentTimeMs;
                tuneState->state = TUNE_STATE_STEP_DOWN;
            }
            break;

        case TUNE_STATE_STEP_DOWN:
            // УЧЕТ НАПРАВЛЕНИЯ: При прямом ждем падения, при обратном - превышения
            if ((rc->direction == 0 && feedback < setpoint) ||
                (rc->direction == 1 && feedback > setpoint)) {
                tuneState->cycles++;
                if (tuneState->cycles >= 3) {
                    tuneState->state = TUNE_STATE_CALCULATE;
                } else {
                    vars->vars[rc->outputIndex].as_float = tuneState->relayStepPos;
                    tuneState->startTime = currentTimeMs;
                    tuneState->maxPv = -100000.0f;
                    tuneState->minPv = 100000.0f;
                    tuneState->state = TUNE_STATE_STEP_UP;
                }
            }
            break;

        case TUNE_STATE_CALCULATE:
            tuneState->Tu = (float)(currentTimeMs - tuneState->startTime) / 1000.0f;
            float a = (tuneState->maxPv - tuneState->minPv) / 2.0f;
            float d = (tuneState->relayStepPos - tuneState->relayStepNeg) / 2.0f;
            tuneState->Ku = (4.0f * d) / (3.14159265f * a);

            rc->Kp = 0.6f * tuneState->Ku;
            rc->Ki = (1.2f * tuneState->Ku) / tuneState->Tu;
            rc->Kd = (0.075f * tuneState->Ku) * tuneState->Tu;

            tuneState->state = TUNE_STATE_IDLE;
            return true;
    }
    return false;
}

void Kairos_TIM7_Init(float dTime) {
    // 1. Расчет ARR (Auto-Reload Register)
    // Частота тактирования таймера после прескалера 839:
    // 84MHz / 840 = 100 000 Гц (10 мкс на один тик)
    uint32_t arr = (uint32_t)(dTime * 100000.0f) - 1;

    // 2. Установка нового значения периода
    // Можно напрямую через регистр для скорости, либо через макрос HAL
    __HAL_TIM_SET_AUTORELOAD(&htim7, arr);

    // 3. Сброс счетчика, чтобы первый период был полным (аналог EGR -> UG)
    __HAL_TIM_SET_COUNTER(&htim7, 0);

    // 4. Очистка флага прерывания, который мог выставиться при установке UG
    __HAL_TIM_CLEAR_IT(&htim7, TIM_IT_UPDATE);

    // 5. Запуск таймера в режиме прерываний
    HAL_TIM_Base_Start_IT(&htim7);
}
