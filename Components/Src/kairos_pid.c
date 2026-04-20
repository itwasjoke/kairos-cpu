#include "kairos_pid.h"

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
    float error;
    if (rc->direction == 0) {
        error = setpoint - feedback; // Прямое
    } else {
        error = feedback - setpoint; // Обратное
    }

    float absError = fabsf(error);

    // Мертвая зона
    float sensZoneAbs = (rc->sensZone / 100.0f) * fabsf(setpoint);
    if (absError < sensZoneAbs) {
        error = 0.0f;
        absError = 0.0f;
    }

    // Адаптивная логика
    float currentKp = rc->Kp;
    float currentKi = rc->Ki;
    float currentKd = rc->Kd;

    float maxErrAbs = (rc->maxErr / 100.0f) * fabsf(setpoint);
    float minErrAbs = (rc->minErr / 100.0f) * fabsf(setpoint);

    if (absError > maxErrAbs && rc->maxErr > 0) {
        currentKp *= 1.5f;
        currentKi = 0.0f;
    } else if (absError < minErrAbs && rc->minErr > 0) {
        currentKp *= 0.8f;
        currentKi *= 1.2f;
    }

    // Расчет компонентов ПИД
    float P_out = currentKp * error;

    if (!state->isFirstRun && currentKi > 0.0f) {
        state->integralSum += error * currentKi * rc->dTime;
        state->integralSum = Clamp(state->integralSum, rc->minValue, rc->maxValue);
    }

    float D_out = 0.0f;
    if (!state->isFirstRun && rc->dTime > 0.0f) {
        D_out = currentKd * (error - state->prevError) / rc->dTime;
    }

    // Выход
    float output = P_out + state->integralSum + D_out;
    vars->vars[rc->outputIndex].as_float = Clamp(output, rc->minValue, rc->maxValue);

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

void Kairos_TIM4_Init(float dTime) {
    // 1. Тактирование
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    // 2. Расчет Prescaler и ARR
    // Для F407: если APB1 Prescaler > 1, частота таймеров = APB1 Clock * 2.
    // Обычно APB1 = 42MHz, значит таймер тикает на 84MHz.
    uint32_t psc = 839; // 84MHz / 840 = 100kHz (10 мкс на тик)

    // Если используешь F103 (72MHz), то APB1 = 36MHz -> таймер 72MHz.
    // Тогда для 100kHz нужно psc = 719.

    uint32_t arr = (uint32_t)(dTime * 100000.0f) - 1;

    TIM4->PSC = psc;
    TIM4->ARR = arr;

    // 3. Сброс счетчика и очистка флагов перед запуском
    TIM4->EGR |= TIM_EGR_UG;
    TIM4->SR &= ~TIM_SR_UIF;

    // 4. Разрешаем прерывание
    TIM4->DIER |= TIM_DIER_UIE;

    // 5. Настройка NVIC
    // Ставим приоритет 5 или ниже (численно больше), чтобы дружить с FreeRTOS
    // и иметь возможность слать данные в очереди.
    NVIC_SetPriority(TIM4_IRQn, 5);
    NVIC_EnableIRQ(TIM4_IRQn);

    // 6. Старт
    TIM4->CR1 |= TIM_CR1_CEN;
}
