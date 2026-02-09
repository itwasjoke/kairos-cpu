
#include "api_interface.h"
#include "Config.h"
#include <string.h>

// --- Реализация API (Impl) ---

int32_t impl_get_int32(uint16_t id) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_INT32)
        return project_vars.vars[id].as_int32;
    return 0;
}

void impl_set_int32(uint16_t id, int32_t val) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_INT32)
        project_vars.vars[id].as_int32 = val;
}

int16_t impl_get_int16(uint16_t id) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_INT16)
        return project_vars.vars[id].as_int16;
    return 0;
}

void impl_set_int16(uint16_t id, int16_t val) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_INT16)
        project_vars.vars[id].as_int16 = val;
}

float impl_get_float(uint16_t id) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_FLOAT)
        return project_vars.vars[id].as_float;
    return 0.0f;
}

void impl_set_float(uint16_t id, float val) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_FLOAT)
        project_vars.vars[id].as_float = val;
}

uint8_t impl_get_bool(uint16_t id) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_BOOL)
        return project_vars.vars[id].as_bool;
    return 0;
}

void impl_set_bool(uint16_t id, uint8_t val) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_BOOL)
        project_vars.vars[id].as_bool = val;
}

// Сборка таблицы API
const SystemAPI_t api = {
    .get_int32 = impl_get_int32,
    .set_int32 = impl_set_int32,
    .get_int16 = impl_get_int16,
    .set_int16 = impl_set_int16,
    .get_float = impl_get_float,
    .set_float = impl_set_float,
		.get_bool = impl_get_bool,
		.set_bool = impl_set_bool
};
