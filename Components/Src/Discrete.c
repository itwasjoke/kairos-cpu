#include "Discrete.h"

/* Массивы для удобного доступа к пинам в цикле */
static GPIO_TypeDef* DIN_Ports[] = {DIN0_GPIO_Port, DIN1_GPIO_Port, DIN2_GPIO_Port, DIN3_GPIO_Port};
static uint16_t DIN_Pins[] = {DIN0_Pin, DIN1_Pin, DIN2_Pin, DIN3_Pin};

static GPIO_TypeDef* DOUT_Ports[] = {DOUT0_GPIO_Port, DOUT1_GPIO_Port, DOUT2_GPIO_Port, DOUT3_GPIO_Port};
static uint16_t DOUT_Pins[] = {DOUT0_Pin, DOUT1_Pin, DOUT2_Pin, DOUT3_Pin};

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;

void Get_Discrete(ProjectVars_t *project_vars){
	  // 1. Читаем 4 дискретных входа
	  for (int i = 0; i < 4; i++) {
	    if (HAL_GPIO_ReadPin(DIN_Ports[i], DIN_Pins[i]) == GPIO_PIN_SET) {
	    	project_vars->vars[DISCRETE_ID + i].as_bool = 1;
	    } else {
	    	project_vars->vars[DISCRETE_ID + i].as_bool = 0;
	    }
	  }

	  // 2. Читаем значения счетчиков из 4 таймеров
	  project_vars->vars[DISCRETE_ID+8].as_uint32 = __HAL_TIM_GET_COUNTER(&htim1);
	  project_vars->vars[DISCRETE_ID+8+1].as_uint32 = __HAL_TIM_GET_COUNTER(&htim2);
	  project_vars->vars[DISCRETE_ID+8+2].as_uint32 = __HAL_TIM_GET_COUNTER(&htim3);
	  project_vars->vars[DISCRETE_ID+8+3].as_uint32 = __HAL_TIM_GET_COUNTER(&htim5);
}

void Set_DiscreteOutput(ProjectVars_t *project_vars){
	for (uint8_t i = 0; i < 4; i++){
		// Выходы идут после входов (DISCRETE_ID + 4)
		GPIO_PinState state = (project_vars->vars[DISCRETE_ID + 4 + i].as_bool > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;
		HAL_GPIO_WritePin(DOUT_Ports[i], DOUT_Pins[i], state);
	}
}
