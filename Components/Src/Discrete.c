#include "Discrete.h"

/* Массивы для удобного доступа к пинам в цикле */
static GPIO_TypeDef* DIN_Ports[] = {DIN0_GPIO_Port, DIN1_GPIO_Port, DIN2_GPIO_Port, DIN3_GPIO_Port};
static uint16_t DIN_Pins[] = {DIN0_Pin, DIN1_Pin, DIN2_Pin, DIN3_Pin};

static GPIO_TypeDef* DOUT_Ports[] = {DOUT0_GPIO_Port, DOUT1_GPIO_Port, DOUT2_GPIO_Port, DOUT3_GPIO_Port};
static uint16_t DOUT_Pins[] = {DOUT0_Pin, DOUT1_Pin, DOUT2_Pin, DOUT3_Pin};

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;

void Get_Discrete(ProjectVars_t *project_vars){
	  // 1. Читаем 4 дискретных входа
	  for (int i = 0; i < 4; i++) {
	    if (HAL_GPIO_ReadPin(DIN_Ports[i], DIN_Pins[i]) == GPIO_PIN_SET) {
	    	project_vars->vars[din_ch0_id + i].as_bool = 0;
	    } else {
	    	project_vars->vars[din_ch0_id + i].as_bool = 1;
	    }
	  }

	  // 2. Читаем значения счетчиков из 4 таймеров
	  project_vars->vars[counter_ch0_id].as_uint32 = __HAL_TIM_GET_COUNTER(&htim1);
	  project_vars->vars[counter_ch0_id+1].as_uint32 = __HAL_TIM_GET_COUNTER(&htim2);
	  project_vars->vars[counter_ch0_id+2].as_uint32 = __HAL_TIM_GET_COUNTER(&htim3);
	  project_vars->vars[counter_ch0_id+3].as_uint32 = __HAL_TIM_GET_COUNTER(&htim5);
}

static uint16_t last_freq = 0;
static uint8_t last_duty = 0;
static bool pwm_running = false;

void Set_DiscreteOutput(ProjectVars_t *project_vars) {
    // Состояние из структуры (кэшируем для удобства)
    uint16_t current_freq = project_vars->vars[dout_pwm_freq_ch0_id].as_uint16;
    float current_duty = project_vars->vars[dout_pwm_duty_ch0_id].as_float;
    bool should_be_on = (project_vars->vars[dout_ch0_id].as_bool > 0);

    // Проверяем, нужен ли нам ШИМ в данный момент
    // ШИМ активен, только если есть частота, заполнение и общая команда "ВКЛ"
    if (should_be_on && current_freq > 0 && current_duty > 0) {

        // 1. Если параметры изменились — пересчитываем регистры
        if (current_freq != last_freq || current_duty != last_duty) {
            uint32_t timer_clock_after_psc = 100000;
            uint32_t arr = (timer_clock_after_psc / current_freq) - 1;

            // Защита от выхода за границы 100%
            if (current_duty > 100) current_duty = 100;
            uint32_t pulse = ((arr + 1) * current_duty) / 100;

            __HAL_TIM_SET_AUTORELOAD(&htim4, arr);
            __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse);

            last_freq = current_freq;
            last_duty = current_duty;
        }

        // 2. Если ШИМ еще не запущен — запускаем один раз
        if (!pwm_running) {
            HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
            pwm_running = true;
        }

    } else {
        // РЕЖИМ СТАТИЧЕСКОГО ВЫХОДА (Обычный дискретный сигнал)
        // 3. Если ШИМ работал — останавливаем его
        if (pwm_running) {
            HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
            pwm_running = false;
            // Сбрасываем кэш параметров, чтобы при возврате в ШИМ он гарантированно обновился
            last_freq = 0;
            last_duty = 0;
        }

        // 4. Управляем ножкой как обычным GPIO
        GPIO_PinState state = should_be_on ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(DOUT_Ports[0], DOUT_Pins[0], state);
    }

    // Обработка остальных 3-х каналов (они просто дискретные)
    for (uint8_t i = 1; i < 4; i++) {
        GPIO_PinState state = (project_vars->vars[dout_ch0_id + i].as_bool > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
        HAL_GPIO_WritePin(DOUT_Ports[i], DOUT_Pins[i], state);
    }
}
