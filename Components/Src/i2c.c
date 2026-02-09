#include "i2c.h"

static osSemaphoreId_t i2c_eeprom_semaphore = NULL;

/**
 * Инициализация параметров для корректной работы i2c
 */
void i2c_init(i2c_config_t *i2c_config, I2C_HandleTypeDef *hi2c){
	if (i2c_eeprom_semaphore == NULL){
		i2c_eeprom_semaphore = osSemaphoreNew(1, 0, NULL);
	}

	osMutexAttr_t i2c_mutex_attributes = {
		.name = "i2c_mutex_eeprom",
		.attr_bits = osMutexRecursive,
		.cb_mem = NULL,
		.cb_size = 0
	};
	osMutexId_t i2c_mutex = osMutexNew(&i2c_mutex_attributes);

	i2c_config->i2c_handle = hi2c;
	i2c_config->i2c_mutex = i2c_mutex;

}

/**
  * @brief  Безопасная запись данных в память I2C устройства с использованием мьютекса и семафора
  * @param  i2c_device Конфигурация драйвера
  * @param  memAddress Адрес памяти устройства
  * @param  pData Указатель на данные для передачи
  * @param  size Размер данных
  * @retval HAL Status
  */
HAL_StatusTypeDef i2c_safe_mem_write(
	eeprom_t *i2c_device,
    uint32_t memAddress,
    uint8_t* pData,
    uint16_t size
) {
    HAL_StatusTypeDef status = HAL_ERROR;

    configASSERT(NULL != i2c_device->i2c_config->i2c_mutex || i2c_eeprom_semaphore != NULL);
    if (osMutexAcquire(i2c_device->i2c_config->i2c_mutex, osWaitForever) == osOK) {
        status = HAL_I2C_Mem_Write_IT(
        		i2c_device->i2c_config->i2c_handle,
				i2c_device->addr,
				memAddress,
				i2c_device->addr_size,
				pData,
				size
				);
        if (status == HAL_OK) {
            if (osSemaphoreAcquire(i2c_eeprom_semaphore, I2C_TIMEOUT) != osOK) {
                status = HAL_TIMEOUT;
            }
        }
        osMutexRelease(i2c_device->i2c_config->i2c_mutex);
    }
    return status;
}

/**
  * @brief  Безопасное чтение данных из памяти I2C устройства с использованием мьютекса и семафора
  * @param  i2c_device Конфигурация драйвера
  * @param  memAddress Адрес памяти устройства
  * @param  pData Указатель на буфер для приема данных
  * @param  size Размер буфера
  * @retval HAL Status
  */
HAL_StatusTypeDef i2c_safe_mem_read(
	eeprom_t *i2c_device,
    uint32_t memAddress,
    uint8_t* pData,
    uint16_t size
) {
    HAL_StatusTypeDef status = HAL_ERROR;

    configASSERT(NULL != i2c_device->i2c_config->i2c_mutex || i2c_eeprom_semaphore != NULL);
    if (osMutexAcquire(i2c_device->i2c_config->i2c_mutex, osWaitForever) == osOK) {
        status = HAL_I2C_Mem_Read_IT(
        		i2c_device->i2c_config->i2c_handle,
				i2c_device->addr | 1,
				memAddress,
				i2c_device->addr_size,
				pData,
				size
				);
        if (status == HAL_OK) {
            if (osSemaphoreAcquire(i2c_eeprom_semaphore, I2C_TIMEOUT) != osOK) {
                status = HAL_TIMEOUT;
            }
        }
        osMutexRelease(i2c_device->i2c_config->i2c_mutex);
    }
    return status;
}

/**
  * @brief I2C-коллбэк для завершения передачи данных в память
  * @param hi2c Указатель на структуру I2C_HandleTypeDef
  */
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C1) {
        osSemaphoreRelease(i2c_eeprom_semaphore);
    }
}

/**
  * @brief I2C-коллбэк для завершения приема данных из памяти
  * @param hi2c Указатель на структуру I2C_HandleTypeDef
  */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C1) {
        osSemaphoreRelease(i2c_eeprom_semaphore);
    }
}

/**
  * @brief I2C-коллбэк для завершения передачи мастер-режима
  * @param hi2c Указатель на структуру I2C_HandleTypeDef
  */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C1) {
        osSemaphoreRelease(i2c_eeprom_semaphore);
    }
}

/**
  * @brief I2C-коллбэк для завершения приема мастер-режима
  * @param hi2c Указатель на структуру I2C_HandleTypeDef
  */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C1) {
        osSemaphoreRelease(i2c_eeprom_semaphore);
    }
}

/**
  * @brief I2C-коллбэк для обработки ошибок
  * @param hi2c Указатель на структуру I2C_HandleTypeDef
  */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef* hi2c) {
    if (hi2c->Instance == I2C2) {
        osSemaphoreRelease(i2c_eeprom_semaphore);
    }
}

