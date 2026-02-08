#include "Discrete.h"

/* Массивы для удобного доступа к пинам в цикле */
static GPIO_TypeDef* DIN_Ports[] = {DIN0_GPIO_Port, DIN1_GPIO_Port, DIN2_GPIO_Port, DIN3_GPIO_Port,
                                    DOUT0_GPIO_Port, DOUT1_GPIO_Port, DOUT2_GPIO_Port, DOUT3_GPIO_Port};
static uint16_t DIN_Pins[] = {DIN0_Pin, DIN1_Pin, DIN2_Pin, DIN3_Pin,
                              DOUT0_Pin, DOUT1_Pin, DOUT2_Pin, DOUT3_Pin};

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim5;

void Get_Discrete(uint8_t *values, uint16_t *counters){
	uint8_t mask = 0;

	  // 1. Читаем 8 GPIO и формируем бит-маску
	  for (int i = 0; i < 8; i++) {
	    if (HAL_GPIO_ReadPin(DIN_Ports[i], DIN_Pins[i]) == GPIO_PIN_SET) {
	      mask |= (1 << i);
	    }
	  }
	  *values = mask;

	  // 2. Читаем значения счетчиков из 4 таймеров
	  counters[0] = (uint16_t)__HAL_TIM_GET_COUNTER(&htim1);
	  counters[1] = (uint16_t)__HAL_TIM_GET_COUNTER(&htim2);
	  counters[2] = (uint16_t)__HAL_TIM_GET_COUNTER(&htim3);
	  counters[3] = (uint16_t)__HAL_TIM_GET_COUNTER(&htim5);
}

void Set_DiscreteOutput(uint8_t ch_num, uint8_t value){
	GPIO_PinState state = (value > 0) ? GPIO_PIN_SET : GPIO_PIN_RESET;

	switch (ch_num) {
		case 0: HAL_GPIO_WritePin(DOUT0_GPIO_Port, DOUT0_Pin, state); break;
		case 1: HAL_GPIO_WritePin(DOUT1_GPIO_Port, DOUT1_Pin, state); break;
		case 2: HAL_GPIO_WritePin(DOUT2_GPIO_Port, DOUT2_Pin, state); break;
		case 3: HAL_GPIO_WritePin(DOUT3_GPIO_Port, DOUT3_Pin, state); break;
		default: break; // Неверный номер канала
	}
}
