#ifndef API_INTERFACE_H_
#define API_INTERFACE_H_

#include <stdint.h>

#define USER_CODE_ADDR  0x08060000       // Начало сектора 7
#define MAGIC_KEY       0xCAFEBABE       // Метка валидности кода

// --- 1. Структуры переменных ---

typedef enum {
    VAR_TYPE_NONE  = 0,
    VAR_TYPE_INT   = 1, // int
    VAR_TYPE_FLOAT = 2, // float
    VAR_TYPE_BOOL  = 3  // uint8_t
} VarType_e;

typedef union {
    int  as_int;
    uint8_t  as_bool;
    float    as_float;
} VarValue_u;

// Конфигурация проекта
#define MAX_VARS 255
typedef struct __attribute__((packed)) {
    uint32_t magic_key;
    VarValue_u vars[MAX_VARS];
    uint16_t var_count;
    uint8_t var_types[MAX_VARS];
} ProjectVars_t;

// --- 2. API Функции (Таблица экспорта) ---

typedef struct {
    // Геттеры
    int (*get_int)(uint16_t id);
    float   (*get_float)(uint16_t id);
    uint8_t  (*get_bool)(uint16_t id);

    // Сеттеры
    void    (*set_int)(uint16_t id, int val);
    void    (*set_float)(uint16_t id, float val);
    void    (*set_bool)(uint16_t id, uint8_t val);
} SystemAPI_t;

// Сигнатура точки входа в пользовательский код
typedef void (*PluginEntry_t)(const SystemAPI_t *api);

extern const SystemAPI_t api;

#endif /* INTERFACE_API_H_ */
