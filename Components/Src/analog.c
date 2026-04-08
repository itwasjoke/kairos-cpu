#include "analog.h"

static Analog_Handle_t *handle;

void Analog_Init(Analog_Handle_t *comp_handle) {
	handle = comp_handle;

  // Запускаем АЦП в режиме DMA на 4 замера
  HAL_ADC_Start_DMA(handle->hadc, (uint32_t*)handle->raw_data_adc, 4);
  HAL_DAC_Start(handle->hdac, DAC_CHANNEL_1);
  HAL_DAC_Start(handle->hdac, DAC_CHANNEL_2);
}

/**
 * @brief Внутренняя функция для конвертации сырых данных в физику и ШИМ
 */
static float process_channel(uint16_t raw_val, SensorType_t type, uint8_t led_num) {
  float raw_norm = (float)raw_val / 4095.0f;

  // 1. Управление светодиодом (0..99%)
  Led_PWM_Set(led_num, (uint8_t)(raw_norm * 99.0f));

  // 2. Расчет физического значения
  if (type == TYPE_VOLTAGE_0_10V) {
    return raw_norm * 10.0f;
  } else {
    return raw_norm * 20.0f;
  }
}

/**
 * @brief Получение данных 4xADC и 2xDAC в один массив (6 элементов)
 * @param output_array Массив float минимум из 6 элементов
 */
void Analog_GetValues(ProjectVars_t *project_vars) {
  // Константы каналов для автоматизации цикла ЦАП
  const uint32_t dac_channels[2] = { DAC_CHANNEL_1, DAC_CHANNEL_2 };

  // 1. Обработка 4 каналов АЦП (Индикация на LED 1-4)
  for (int i = 0; i < 4; i++) {
  	project_vars->vars[ANALOG_ID+i].as_float = process_channel(handle->raw_data_adc[i],
                                      handle->channel_types_adc[i],
                                      i + 1);
  }
}

void Analog_SetOutput(ProjectVars_t *project_vars) {
    float max_val = 0.0f;
    float min_val = 0.0f;

    for (uint8_t i = 0; i < 2; i++){

    	float value = project_vars->vars[ANALOG_ID+4+i].as_float;
      // Определяем границы в зависимости от типа канала
      if (handle->channel_types_dac[i] == TYPE_VOLTAGE_0_10V) {
          min_val = 0.0f;
          max_val = 10.0f;
      } else {
          min_val = 0.0f;
          max_val = 20.0f;
      }

      if (value < min_val) value = min_val;
      if (value > max_val) value = max_val;

      uint32_t dac_raw;
      dac_raw = (uint32_t)((value - min_val) / (max_val - min_val) * 4095.0f);
      uint32_t dac_channel = (i == 1) ? DAC_CHANNEL_1 : DAC_CHANNEL_2;

      HAL_DAC_SetValue(handle->hdac, dac_channel, DAC_ALIGN_12B_R, dac_raw);

      uint8_t pwm_val = (uint8_t)(((float)dac_raw / 4095.0f) * 99.0f);
      Led_PWM_Set(i + 5, pwm_val);
    }
}
