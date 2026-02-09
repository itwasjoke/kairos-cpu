#include "eeprom.h"

/**
 * @brief Инициализирует драйвер EEPROM
 * @param Конфигурация драйвера
 * @param hi2c_handle Указатель на структуру I2C_HandleTypeDef
 * @param addr Адрес устройства
 * @param addr_size	Размер адреса ячейки EEPROM в байтах
 */
HAL_StatusTypeDef eeprom_init(
		eeprom_t *i2c_eeprom,
		i2c_config_t *i2c_config,
		uint8_t addr,
		uint8_t addr_size
) {
	if (i2c_config->i2c_mutex == NULL){
		return HAL_ERROR;
	}
	i2c_eeprom->addr = addr;
	i2c_eeprom->addr_size = addr_size;
	i2c_eeprom->i2c_config = i2c_config;
	return HAL_OK;
}

/**
 * @brief Записывает один байт в EEPROM по указанному адресу
 * @param i2c_eeprom Конфигурация драйвера
 * @param mem_address 16-битный адрес ячейки памяти
 * @param data Байт данных для записи
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_write_byte(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t data)
{
    HAL_StatusTypeDef status = i2c_safe_mem_write(
		i2c_eeprom,
        mem_address,                // 16-битный адрес ячейки памяти
        &data,                      // Указатель на данные для записи
        1                           // Размер данных (1 байт)
    );

    if (status == HAL_OK) {
        HAL_StatusTypeDef poll_status;
        do {
            poll_status = HAL_I2C_Master_Transmit(i2c_eeprom->i2c_config->i2c_handle, i2c_eeprom->addr, NULL, 0, 100);
        } while (poll_status != HAL_OK);

        // После записи EEPROM требуется время для внутреннего цикла
    //    osDelay(EEPROM_WRITE_DELAY_MS);
    }

    return status;
}

/**
 * @brief Читает один байт из EEPROM по указанному адресу
 * @param i2c_eeprom Конфигурация драйвера
 * @param mem_address 16-битный адрес ячейки памяти
 * @param data Указатель на переменную, куда будет записан прочитанный байт
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_read_byte(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t* data)
{
    HAL_StatusTypeDef status = i2c_safe_mem_read(
		i2c_eeprom,
        mem_address,                // 16-битный адрес ячейки памяти
        data,                       // Указатель на буфер для прочитанных данных
        1                           // Размер данных (1 байт)
    );

    return status;
}

/**
 * @brief Записывает буфер данных в EEPROM по указанному адресу
 * Учитывает постраничную запись
 * @param i2c_eeprom Конфигурация драйвера
 * @param mem_address 16-битный адрес ячейки памяти
 * @param pData Указатель на буфер с данными
 * @param size Размер буфера
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_write_buffer(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t* pData, uint16_t size)
{
    // Проверка на корректность адреса и размера буфера
    if (mem_address >= EEPROM_TOTAL_SIZE || size == 0 || (mem_address + size) > EEPROM_TOTAL_SIZE) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = HAL_OK;
    uint16_t bytes_to_write;
    uint16_t current_address = mem_address;
    uint8_t* current_data_ptr = pData;
    uint16_t remaining_size = size;

    // Цикл, пока есть данные для записи
    while (remaining_size > 0) {
        // Вычисляем, сколько байт можно записать до конца текущей страницы
        bytes_to_write = EEPROM_PAGE_SIZE - (current_address % EEPROM_PAGE_SIZE);

        // Если оставшихся данных меньше, чем до конца страницы, пишем все оставшиеся данные
        if (bytes_to_write > remaining_size) {
            bytes_to_write = remaining_size;
        }

        // Вызываем безопасную функцию записи с мьютексом
        status = i2c_safe_mem_write(
			i2c_eeprom,
            current_address,
            current_data_ptr,
            bytes_to_write
        );

        if (status != HAL_OK) {
            return status; // Возвращаем ошибку, если запись не удалась
        }

        // Задержка для завершения внутренней записи
        HAL_StatusTypeDef poll_status;
        do {
            poll_status = HAL_I2C_Master_Transmit(i2c_eeprom->i2c_config->i2c_handle, i2c_eeprom->addr, NULL, 0, 100);
        } while (poll_status != HAL_OK);
//        osDelay(EEPROM_WRITE_DELAY_MS);

        // Обновляем указатели и счетчики для следующего шага
        current_address += bytes_to_write;
        current_data_ptr += bytes_to_write;
        remaining_size -= bytes_to_write;
    }

    return status;
}

/**
 * @brief Читает буфер данных из EEPROM по указанному адресу
 * @param i2c_eeprom Конфигурация драйвера
 * @param mem_address 16-битный адрес ячейки памяти
 * @param pData Указатель на буфер для приема данных
 * @param size Размер буфера
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_read_buffer(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t* pData, uint16_t size)
{
    // Проверка на корректность адреса и размера буфера
    if (mem_address >= EEPROM_TOTAL_SIZE || size == 0 || (mem_address + size) > EEPROM_TOTAL_SIZE) {
        return HAL_ERROR;
    }

    HAL_StatusTypeDef status = i2c_safe_mem_read(
		i2c_eeprom,
        mem_address,
        pData,
        size
    );

    return status;
}


