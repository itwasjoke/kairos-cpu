#ifndef RS485_H
#define RS485_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef struct {
    UART_HandleTypeDef *huart;
    USART_TypeDef * instance;
    GPIO_TypeDef *de_port;
    uint16_t de_pin;
    uint32_t baud_rate;
	uint32_t stop_bits;
    osSemaphoreId_t txSem;
    osSemaphoreId_t rxSem;
} RS485_Handle;

// Инициализация (создание семафоров)
HAL_StatusTypeDef RS485_Init(RS485_Handle *handle);

// Передача через DMA + Семафор
osStatus_t RS485_Transmit_DMA(RS485_Handle *handle, uint8_t *pData, uint16_t size, uint32_t timeout);

// Прием через DMA + Семафор
osStatus_t RS485_Receive_DMA(RS485_Handle *handle, uint8_t *pData, uint16_t size, uint32_t timeout);

// Коллбэки (их нужно вызвать в main.c или stm32f4xx_it.c)
void RS485_TxCpltCallback(UART_HandleTypeDef *huart);
void RS485_RxCpltCallback(UART_HandleTypeDef *huart);

#endif
