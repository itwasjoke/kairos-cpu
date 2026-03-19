#ifndef DISCRETE_H_
#define DISCRETE_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "Config.h"

void Get_Discrete(ProjectVars_t *project_vars);

void Set_DiscreteOutput(ProjectVars_t *project_vars);

#endif /* DISCRETE_H_ */
