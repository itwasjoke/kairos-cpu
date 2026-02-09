/*
 * eeprom.h
 *
 *  Created on: Feb 6, 2026
 *      Author: itwasjoke
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "i2c.h"

#define EEPROM_TOTAL_SIZE       4096

// Задержка после операции записи для завершения внутреннего цикла записи EEPROM
#define EEPROM_WRITE_DELAY_MS   6

// Размер страницы EEPROM в байтах
#define EEPROM_PAGE_SIZE        8

// 7-битный адрес устройства EEPROM на шине I2C
#define EEPROM_DEVICE_ADDR 0x50 << 1

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
);

/**
 * @brief Записывает один байт в EEPROM по указанному адресу
 * @param mem_address 16-битный адрес ячейки памяти
 * @param data Байт данных для записи
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_write_byte(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t data);

/**
 * @brief Читает один байт из EEPROM по указанному адресу
 * @param mem_address 16-битный адрес ячейки памяти
 * @param data Указатель на переменную, куда будет записан прочитанный байт
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_read_byte(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t* data);

/**
 * @brief Записывает буфер данных в EEPROM по указанному адресу
 * Учитывает постраничную запись
 * @param mem_address 16-битный адрес ячейки памяти
 * @param pData Указатель на буфер с данными
 * @param size Размер буфера
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_write_buffer(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t* pData, uint16_t size);

/**
 * @brief Читает буфер данных из EEPROM по указанному адресу
 * @param mem_address 16-битный адрес ячейки памяти
 * @param pData Указатель на буфер для приема данных
 * @return Статус операции HAL
 */
HAL_StatusTypeDef eeprom_read_buffer(eeprom_t *i2c_eeprom, uint16_t mem_address, uint8_t* pData, uint16_t size);


#endif /* EEPROM_H_ */
