#ifndef DISCRETE_H_
#define DISCRETE_H_

#include "stm32f4xx_hal.h"
#include "main.h"

void Get_Discrete(uint8_t *values, uint16_t *counters);

void Set_DiscreteOutput(uint8_t ch_num, uint8_t value);

#endif /* DISCRETE_H_ */
