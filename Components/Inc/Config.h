#ifndef CONFIG_H_
#define CONFIG_H_

#include "api_interface.h"

typedef enum {
  TYPE_VOLTAGE_0_10V,
  TYPE_CURRENT_4_20MA
} SensorType_t;

typedef struct __attribute__((packed)){
	uint32_t ip;
	uint32_t mask;
	uint32_t gateway;
	uint16_t rs_start_var;
	uint16_t can_start_var;
	uint16_t user_start_var;
} MainConfig_t;

typedef struct __attribute__((packed)){
	uint16_t start_bit;
	uint32_t type;
} VarDesc_t;

typedef struct __attribute__((packed)){
	uint8_t tx_bytes[10];
	uint8_t tx_count;
	uint8_t rx_count;
	uint8_t var_count;
	VarDesc_t vars[10];
} RsCommand_t;

typedef struct __attribute__((packed)){
	uint16_t timeout;
	uint8_t is_big_endian;
	uint8_t commands_count;
	RsCommand_t rs_commands[3];
} RsDevice_t;

typedef struct __attribute__((packed)){
	uint32_t id;
	uint16_t timeout;
	uint8_t IDC;
	uint8_t var_count;
	VarDesc_t vars[10];
} CanMessage_t;

typedef struct __attribute__((packed)){
	RsDevice_t rs_devices[3];
	CanMessage_t can_messages[3];
	uint32_t rs_speed;
	uint32_t can_speed;
	uint32_t can_filter_id;
	uint32_t can_filter_mask;
	uint8_t rs_stop_bits;
	uint8_t rs_count;
	uint8_t can_count;
	uint32_t analog_types[6];
} GpioConfig_t;

typedef struct __attribute__((packed)){
	float Kp; // П - коэффициент регулятора
	float Ki; // И - коэффициент регулятора
	float Kd; // Д - коэффициент регулятора
	float minValue;  // Нижнее ограничение от перенасыщения
	float maxValue;  // Верхнее ограничение от перенасыщения
	float dTime;  // Время дискретизации (сейчас функция регулятора просто запускается в main, не знаю, как использовать)
	float sensZone; // зона чувствительности в процентах
	float maxErr; // включение агрессивного режима при ошибке больше этого процента
	float minErr; // включение точного режима при ошибке меньше этого процента
	uint16_t setpointIndex; // индекс переменной для значения, которое нужно держать в обратной связи (для project_vars)
	uint16_t outputIndex; // индекс переменной для выходного значения регулятора (для project_vars)
	uint16_t feedbackIndex; // индекс переменной для обратной связи (для project_vars)
	uint8_t direction; // 0 - прямое; 1 - обратное
	uint8_t autoTuneRequested;
} RegulatorConfig_t;

typedef struct __attribute__((packed)){
	MainConfig_t main_config;
	GpioConfig_t gpio_config;
	uint32_t config_version;
	uint8_t var_count;
  uint8_t var_types[MAX_VARS];
  RegulatorConfig_t regulator_config;
} KairosConfig_t;

extern ProjectVars_t project_vars;
extern KairosConfig_t kairos_config;
extern uint8_t new_config;

#endif /* CONFIG_H_ */
