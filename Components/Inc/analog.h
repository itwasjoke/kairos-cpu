#ifndef ANALOG_H
#define ANALOG_H

#include "stm32f4xx_hal.h"
#include "led.h"
#include "Config.h"

typedef struct {
  ADC_HandleTypeDef *hadc;
  DAC_HandleTypeDef *hdac;
  SensorType_t channel_types_adc[4];
  uint16_t raw_data_adc[4];
  SensorType_t channel_types_dac[2];
} Analog_Handle_t;

void Analog_Init(Analog_Handle_t *handle);

void Analog_GetValues(ProjectVars_t *project_vars);

void Analog_SetOutput(ProjectVars_t *project_vars);

#endif
