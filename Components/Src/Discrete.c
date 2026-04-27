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

void Set_DiscreteOutput(ProjectVars_t *project_vars){
	GPIO_PinState state = (project_vars->vars[dout_ch0_id].as_bool > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;

	if (project_vars->vars[dout_pwm_freq_ch0_id].as_uint32 != 0
			&& project_vars->vars[dout_pwm_duty_ch0_id].as_uint32 != 0
			&& state == GPIO_PIN_SET
			) {

		uint32_t timer_clock_after_psc = 100000;
		uint32_t arr = (timer_clock_after_psc / (uint32_t)project_vars->vars[dout_pwm_freq_ch0_id].as_uint32) - 1;
		uint32_t pulse = (uint32_t)(((arr + 1) * project_vars->vars[dout_pwm_duty_ch0_id].as_uint32) / 100.0f);

		// 3. Обновление регистров таймера
		__HAL_TIM_SET_AUTORELOAD(&htim4, arr);
		__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulse); // Укажи нужный канал

		// 4. Запуск ШИМ
		HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);

	} else if (project_vars->vars[dout_pwm_freq_ch0_id].as_uint32 != 0
			&& project_vars->vars[dout_pwm_duty_ch0_id].as_uint32 != 0
			) {
		HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);

	} else {
			HAL_GPIO_WritePin(DOUT_Ports[0], DOUT_Pins[0], state);
	}

	for (uint8_t i = 1; i < 4; i++){
			GPIO_PinState state = (project_vars->vars[dout_ch0_id + i].as_bool > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
			HAL_GPIO_WritePin(DOUT_Ports[i], DOUT_Pins[i], state);
	}
}
