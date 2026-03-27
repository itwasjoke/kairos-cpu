
#include "api_interface.h"
#include "Config.h"
#include <string.h>

// --- Реализация API (Impl) ---

uint8_t impl_get_8int(uint16_t id) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_INT8)
        return project_vars.vars[id].as_uint8;
    return 0;
}

void impl_set_8int(uint16_t id, uint8_t val) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_INT8)
        project_vars.vars[id].as_uint8 = val;
}

uint16_t impl_get_16int(uint16_t id) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_INT16)
        return project_vars.vars[id].as_uint16;
    return 0;
}

void impl_set_16int(uint16_t id, uint16_t val) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_INT16)
        project_vars.vars[id].as_uint16 = val;
}

uint32_t impl_get_32int(uint16_t id) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_INT32)
        return project_vars.vars[id].as_uint32;
    return 0;
}

void impl_set_32int(uint16_t id, uint32_t val) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_INT32)
        project_vars.vars[id].as_uint32 = val;
}

float impl_get_float(uint16_t id) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_FLOAT)
        return project_vars.vars[id].as_float;
    return 0.0f;
}

void impl_set_float(uint16_t id, float val) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_FLOAT)
        project_vars.vars[id].as_float = val;
}

uint8_t impl_get_bool(uint16_t id) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_BOOL)
        return project_vars.vars[id].as_bool;
    return 0;
}

void impl_set_bool(uint16_t id, uint8_t val) {
    if (id >= 0 && kairos_config.var_types[id] == VAR_TYPE_BOOL)
        project_vars.vars[id].as_bool = val;
}

// Сборка таблицы API
const SystemAPI_t api = {
    .get_uint8 = impl_get_8int,
    .set_uint8 = impl_set_8int,
    .get_uint16 = impl_get_16int,
    .set_uint16 = impl_set_16int,
    .get_uint32 = impl_get_32int,
    .set_uint32 = impl_set_32int,
    .get_float = impl_get_float,
    .set_float = impl_set_float,
		.get_bool = impl_get_bool,
		.set_bool = impl_set_bool
};
