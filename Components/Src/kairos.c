#include "Config.h"
#include "api_interface.h"

PluginEntry_t user_plugin = NULL;     // Указатель на загруженную функцию
ProjectVars_t project_vars;               // Переменные в RAM
KairosConfig_t kairos_config;

void check_user_code(){
	// Проверка наличия кода во Flash
	uint32_t *flash_ptr = (uint32_t*)USER_CODE_ADDR;

	// Мы договорились, что первые 4 байта бинарника - это Magic Key
	if (*flash_ptr == MAGIC_KEY) {
			// Код начинается сразу после MagicKey (4 байта) + Size (4 байта) = смещение 8
			uint32_t code_addr = USER_CODE_ADDR + 8;

			// Бит 0 должен быть 1 для Thumb режима
			if (!(code_addr & 1)) code_addr |= 1;

			user_plugin = (PluginEntry_t)code_addr;
	}
}

void KairosCycle(){
	// 1. Загрузка переменных из I2C (Ваша функция)
	// 2. Загрузка конфигурации из I2C

	check_user_code();

	for(;;) {
		// Чтение датчиков

		// Пользовательский код
    if (user_plugin != NULL) {
        user_plugin(&api);
    }

    // Сохранение
	}
}
