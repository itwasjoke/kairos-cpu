#include "rs485.h"

// Статическая переменная для поиска handle в прерываниях
static RS485_Handle *g_handle;

HAL_StatusTypeDef RS485_Init(RS485_Handle *handle) {
	g_handle = handle;

	// 1. Создание семафоров CMSIS V2
	handle->txSem = osSemaphoreNew(1, 0, NULL);
	handle->rxSem = osSemaphoreNew(1, 0, NULL);

	// 2. Настройка параметров UART
	handle->huart->Instance = handle->instance;
	handle->huart->Init.BaudRate = handle->baud_rate;
	handle->huart->Init.StopBits = handle->stop_bits;
	// Остальные параметры обычно стандартные, но их тоже можно вынести в handle
	handle->huart->Init.WordLength = UART_WORDLENGTH_8B;
	handle->huart->Init.Parity = UART_PARITY_NONE;
	handle->huart->Init.Mode = UART_MODE_TX_RX;
	handle->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	handle->huart->Init.OverSampling = UART_OVERSAMPLING_16;

	if (HAL_UART_Init(handle->huart) != HAL_OK) {
		return HAL_ERROR;
	}

	// Устанавливаем DE в режим приема
	HAL_GPIO_WritePin(handle->de_port, handle->de_pin, GPIO_PIN_RESET);

	return HAL_OK;
}

osStatus_t RS485_Transmit_DMA(RS485_Handle *handle, uint8_t *pData, uint16_t size, uint32_t timeout) {
	// 1. Включаем DE
	HAL_GPIO_WritePin(handle->de_port, handle->de_pin, GPIO_PIN_SET);

	// 2. Запускаем DMA передачу
	if (HAL_UART_Transmit_DMA(handle->huart, pData, size) != HAL_OK) {
		HAL_GPIO_WritePin(handle->de_port, handle->de_pin, GPIO_PIN_RESET);
		return osErrorResource;
	}

	// 3. Ждем семафора от прерывания
	osStatus_t status = osSemaphoreAcquire(handle->txSem, timeout);

	// 4. Выключаем DE (пин уже сброшен в коллбэке, но для надежности)
	HAL_GPIO_WritePin(handle->de_port, handle->de_pin, GPIO_PIN_RESET);

	return status;
}

osStatus_t RS485_Receive_DMA(RS485_Handle *handle, uint8_t *pData, uint16_t size, uint32_t timeout) {
    // DE должен быть в RESET
	if (HAL_UART_Receive_DMA(handle->huart, pData, size) != HAL_OK) {
		return osErrorResource;
	}

    // Ожидаем заполнения буфера по DMA
	return osSemaphoreAcquire(handle->rxSem, timeout);
}

// Вызывается HAL по окончании передачи (прерывание TC)
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == g_handle->huart) {
			// Выключаем DE немедленно, как только ушел последний бит
		HAL_GPIO_WritePin(g_handle->de_port, g_handle->de_pin, GPIO_PIN_RESET);
		Led_Blink(LED_RS_TX, 100);
		osSemaphoreRelease(g_handle->txSem);
	}
}

// Вызывается HAL по окончании приема
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == g_handle->huart) {
		Led_Blink(LED_RS_RX, 100);
		osSemaphoreRelease(g_handle->rxSem);
	}
}
