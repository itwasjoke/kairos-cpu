#ifndef ANALOG_H
#define ANALOG_H

#include "stm32f4xx_hal.h"
#include "led.h"

typedef enum {
  TYPE_VOLTAGE_0_10V,
  TYPE_CURRENT_4_20MA
} SensorType_t;

typedef struct {
  ADC_HandleTypeDef *hadc;
  DAC_HandleTypeDef *hdac;
  SensorType_t channel_types_adc[4];
  uint16_t raw_data_adc[4];
  SensorType_t channel_types_dac[2];
} Analog_Handle_t;

void Analog_Init(Analog_Handle_t *handle);

void Analog_GetValues(float *output_array);

void Analog_SetOutput(uint8_t channel_idx, float value);

#endif
