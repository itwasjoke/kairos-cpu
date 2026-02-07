#include "rs485.h"

// Статическая переменная для поиска handle в прерываниях
static RS485_Handle *g_handle;

void RS485_Init(RS485_Handle *handle) {
    g_handle = handle;

    // Создаем бинарные семафоры (max_count = 1, initial_count = 0)
    handle->txSem = osSemaphoreNew(1, 0, NULL);
    handle->rxSem = osSemaphoreNew(1, 0, NULL);

    HAL_GPIO_WritePin(handle->de_port, handle->de_pin, GPIO_PIN_RESET);
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
        osSemaphoreRelease(g_handle->txSem);
    }
}

// Вызывается HAL по окончании приема
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == g_handle->huart) {
        osSemaphoreRelease(g_handle->rxSem);
    }
}
