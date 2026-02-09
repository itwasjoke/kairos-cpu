/*
 * i2c.h
 *
 *  Created on: Feb 6, 2026
 *      Author: itwasjoke
 */

#ifndef I2C_H_
#define I2C_H_

#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"

// Таймаут для операций I2C в мс
#define I2C_TIMEOUT          4

typedef struct {
	osMutexId_t i2c_mutex;
	I2C_HandleTypeDef* i2c_handle;
} i2c_config_t;

typedef struct {
	i2c_config_t *i2c_config;
	uint8_t addr;
	uint8_t addr_size;
} eeprom_t;

/**
  * @brief  Безопасная запись данных в память I2C устройства с использованием мьютекса
  * @param  i2c_device Конфигурация драйвера
  * @param  memAddress Адрес памяти устройства
  * @param  memAddSize Размер адреса памяти
  * @param  pData Указатель на данные для передачи
  * @param  size Размер данных
  * @retval HAL Status
  */
HAL_StatusTypeDef i2c_safe_mem_write(
	eeprom_t *i2c_device,
    uint32_t memAddress,
    uint8_t* pData,
    uint16_t size
);

/**
  * @brief  Безопасное чтение данных из памяти I2C устройства с использованием мьютекса
  * @param  i2c_device Конфигурация драйвера
  * @param  memAddress Адрес памяти устройства
  * @param  memAddSize Размер адреса памяти
  * @param  pData Указатель на буфер для приема данных
  * @param  size Размер буфера
  * @retval HAL Status
  */
HAL_StatusTypeDef i2c_safe_mem_read(
	eeprom_t *i2c_device,
    uint32_t memAddress,
    uint8_t* pData,
    uint16_t size
);

#endif /* I2C_H_ */
