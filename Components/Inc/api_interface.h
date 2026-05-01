#ifndef API_INTERFACE_H_
#define API_INTERFACE_H_

#include <stdint.h>

#define USER_CODE_ADDR  0x08060000       // Начало сектора 7
#define MAGIC_KEY       0xCAFEBABE       // Метка валидности кода

#define KAIROS_ID								0

#define ain_ch0_id							1
#define ain_ch1_id							2
#define ain_ch2_id							3
#define ain_ch3_id							4

#define aout_ch0_id							5
#define aout_ch1_id							6

#define din_ch0_id							7
#define din_ch1_id							8
#define din_ch2_id							9
#define din_ch3_id							10

#define counter_ch0_id					11
#define counter_ch1_id					12
#define counter_ch2_id					13
#define counter_ch3_id					14

#define dout_ch0_id							15
#define dout_ch1_id							16
#define dout_ch2_id							17
#define dout_ch3_id							18

#define dout_pwm_freq_ch0_id			19
#define dout_pwm_duty_ch0_id			20

#define setpoint_id							21

// --- 1. Структуры переменных ---

typedef enum {
    VAR_TYPE_NONE  = 0,
    VAR_TYPE_INT8   = 1, // int
		VAR_TYPE_INT16   = 2, // int
		VAR_TYPE_INT32   = 3, // int
    VAR_TYPE_FLOAT = 4, // float
    VAR_TYPE_BOOL  = 5  // uint8_t
} VarType_e;

typedef union {
    uint8_t  as_uint8;
    uint16_t  as_uint16;
    uint32_t  as_uint32;
    uint8_t  as_bool;
    float    as_float;
} VarValue_u;

// Конфигурация проекта
#define MAX_VARS 255
typedef struct {
    VarValue_u vars[MAX_VARS];
} ProjectVars_t;

// --- 2. API Функции (Таблица экспорта) ---

typedef struct {
    // Геттеры
    uint8_t (*get_uint8)(uint16_t id);
    uint16_t (*get_uint16)(uint16_t id);
    uint32_t (*get_uint32)(uint16_t id);
    float   (*get_float)(uint16_t id);
    uint8_t  (*get_bool)(uint16_t id);

    // Сеттеры
    void    (*set_uint8)(uint16_t id, uint8_t val);
    void    (*set_uint16)(uint16_t id, uint16_t val);
    void    (*set_uint32)(uint16_t id, uint32_t val);
    void    (*set_float)(uint16_t id, float val);
    void    (*set_bool)(uint16_t id, uint8_t val);
} SystemAPI_t;

// Сигнатура точки входа в пользовательский код
typedef void (*PluginEntry_t)(const SystemAPI_t *api);

extern const SystemAPI_t api;

#endif /* INTERFACE_API_H_ */
