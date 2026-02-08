#include "analog.h"

static Analog_Handle_t *handle;

void Analog_Init(Analog_Handle_t *comp_handle) {
	handle = comp_handle;

  // Запускаем АЦП в режиме DMA на 4 замера
  HAL_ADC_Start_DMA(handle->hadc, (uint32_t*)handle->raw_data_adc, 4);
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
    // 0..20мА (или 4..20мА, если подставите формулу из прошлого ответа)
    return raw_norm * 20.0f;
  }
}

/**
 * @brief Получение данных 4xADC и 2xDAC в один массив (6 элементов)
 * @param output_array Массив float минимум из 6 элементов
 */
void Analog_GetValues(float *output_array) {
  // Константы каналов для автоматизации цикла ЦАП
  const uint32_t dac_channels[2] = { DAC_CHANNEL_1, DAC_CHANNEL_2 };

  // 1. Обработка 4 каналов АЦП (Индикация на LED 1-4)
  for (int i = 0; i < 4; i++) {
    output_array[i] = process_channel(handle->raw_data_adc[i],
                                      handle->channel_types_adc[i],
                                      i + 1);
  }

  // 2. Обработка 2 каналов ЦАП (Индикация на LED 5-6)
  for (int i = 0; i < 2; i++) {
    uint32_t current_raw = HAL_DAC_GetValue(handle->hdac, dac_channels[i]);

    output_array[i + 4] = process_channel((uint16_t)current_raw,
                                          handle->channel_types_dac[i],
                                          i + 5);
  }
}

void Analog_SetOutput(uint8_t channel_idx, float value) {
    if (channel_idx > 1) return; // Защита от неверного индекса

    float max_val = 0.0f;
    float min_val = 0.0f;

    // Определяем границы в зависимости от типа канала
    if (handle->channel_types_dac[channel_idx] == TYPE_VOLTAGE_0_10V) {
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
    uint32_t dac_channel = (channel_idx == 0) ? DAC_CHANNEL_1 : DAC_CHANNEL_2;

    HAL_DAC_SetValue(handle->hdac, dac_channel, DAC_ALIGN_12B_R, dac_raw);
    HAL_DAC_Start(handle->hdac, dac_channel);

    uint8_t pwm_val = (uint8_t)(((float)dac_raw / 4095.0f) * 99.0f);
    Led_PWM_Set(channel_idx + 5, pwm_val);
}
