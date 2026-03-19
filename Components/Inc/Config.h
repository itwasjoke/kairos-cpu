#ifndef CONFIG_H_
#define CONFIG_H_

#include "api_interface.h"

typedef enum {
  TYPE_VOLTAGE_0_10V,
  TYPE_CURRENT_4_20MA
} SensorType_t;

typedef struct{
	uint32_t ip;
	uint32_t mask;
	uint32_t gateway;
	uint16_t rs_start_byte;
	uint16_t can_start_byte;
	uint16_t user_start_byte;
} MainConfig_t;

typedef struct {
	uint16_t start_bit;
	VarType_e type;
} VarDesc_t;

typedef struct{
	uint8_t tx_bytes[20];
	uint8_t tx_count;
	uint8_t rx_count;
	uint8_t var_count;
	VarDesc_t vars[20];
} RsCommand_t;

typedef struct {
	uint16_t timeout;
	uint8_t is_big_endian;
	RsCommand_t rs_commands[5];
} RsDevice_t;

typedef struct{
	uint32_t id;
	uint16_t timeout;
	uint8_t IDC;
	uint8_t var_count;
	VarDesc_t vars[20];
} CanMessage_t;

typedef struct {
	RsDevice_t rs_devices[10];
	CanMessage_t can_messages[10];
	uint32_t rs_speed;
	uint32_t can_speed;
	uint32_t can_filter_id;
	uint32_t can_filter_mask;
	uint8_t rs_stop_bits;
	uint8_t rs_count;
	uint8_t can_count;
	SensorType_t analog_types[6];
} GpioConfig_t;

typedef struct {
	MainConfig_t main_config;
	GpioConfig_t gpio_config;
	uint32_t config_version;
} KairosConfig_t;

extern ProjectVars_t project_vars;
extern KairosConfig_t kairos_config;

#endif /* CONFIG_H_ */
