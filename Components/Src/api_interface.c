
#include "api_interface.h"
#include "Config.h"
#include <string.h>

// --- Реализация API (Impl) ---

int impl_get_int(uint16_t id) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_INT)
        return project_vars.vars[id].as_int;
    return 0;
}

void impl_set_int(uint16_t id, int32_t val) {
    if (id >= 0 && project_vars.var_types[id] == VAR_TYPE_INT)
        project_vars.vars[id].as_int = val;
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
    .get_int = impl_get_int,
    .set_int = impl_set_int,
    .get_float = impl_get_float,
    .set_float = impl_set_float,
		.get_bool = impl_get_bool,
		.set_bool = impl_set_bool
};
