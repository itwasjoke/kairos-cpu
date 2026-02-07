#include "can.h"

static CAN_Config_t *g_can_config;

HAL_StatusTypeDef CAN_Bus_Init(CAN_Config_t *config) {
  g_can_config = config;
  config->hcan->Instance = config->instance;

  // Настройка таймингов исходя из частоты 42МГц
  // Устанавливаем точку сэмплирования ~78% (TS1=10, TS2=3, +1 Sync = 14 TQ)
  config->hcan->Init.SyncJumpWidth = CAN_SJW_1TQ;
  config->hcan->Init.TimeSeg1 = CAN_BS1_10TQ;
  config->hcan->Init.TimeSeg2 = CAN_BS2_3TQ;

  switch (config->baud_rate) {
    case 1000000: config->hcan->Init.Prescaler = 3;  break;
    case 500000:  config->hcan->Init.Prescaler = 6;  break;
    case 250000:  config->hcan->Init.Prescaler = 12; break;
    case 125000:  config->hcan->Init.Prescaler = 24; break;
    default:      return HAL_ERROR; // Неподдерживаемая скорость
  }

  config->hcan->Init.Mode = CAN_MODE_NORMAL;
  config->hcan->Init.TimeTriggeredMode = DISABLE;
  config->hcan->Init.AutoBusOff = DISABLE;
  config->hcan->Init.AutoWakeUp = DISABLE;
  config->hcan->Init.AutoRetransmission = DISABLE;
  config->hcan->Init.ReceiveFifoLocked = DISABLE;
  config->hcan->Init.TransmitFifoPriority = DISABLE;

  if (HAL_CAN_Init(config->hcan) != HAL_OK) {
    return HAL_ERROR;
  }

  if (config->rxQueue == NULL) {
    config->rxQueue = osMessageQueueNew(10, sizeof(CAN_Message_t), NULL);
  }

  CAN_FilterTypeDef filter;
  filter.FilterBank = 0;
  filter.FilterMode = CAN_FILTERMODE_IDMASK;
  filter.FilterScale = CAN_FILTERSCALE_32BIT;

  // Магия битов для фильтра (сдвиги для STD и EXT ID)
  filter.FilterIdHigh = (config->filter_id >> 13) & 0xFFFF;
  filter.FilterIdLow = (config->filter_id << 3) & 0xFFF8;
  filter.FilterMaskIdHigh = (config->filter_mask >> 13) & 0xFFFF;
  filter.FilterMaskIdLow = (config->filter_mask << 3) & 0xFFF8;

  filter.FilterFIFOAssignment = CAN_RX_FIFO0;
  filter.FilterActivation = ENABLE;
  filter.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(config->hcan, &filter) != HAL_OK) {
    return HAL_ERROR;
  }

  HAL_CAN_Start(config->hcan);
  HAL_CAN_ActivateNotification(config->hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  return HAL_OK;
}

osStatus_t CAN_Bus_Read(CAN_Config_t *config, CAN_Message_t *msg, uint32_t timeout) {
	return osMessageQueueGet(config->rxQueue, msg, NULL, timeout);
}

// Прерывание: вызывается при получении нового сообщения
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CAN_RxHeaderTypeDef header;
	CAN_Message_t msg;

	if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &header, msg.data) == HAL_OK) {
        // Определяем ID в зависимости от типа
		if (header.IDE == CAN_ID_STD) {
				msg.id = header.StdId;
				msg.id_type = CAN_ID_STD;
		} else {
				msg.id = header.ExtId;
				msg.id_type = CAN_ID_EXT;
		}

		msg.dlc = header.DLC;

    // Кладём в очередь. Из ISR используем NULL для таймаута
		osMessageQueuePut(g_can_config->rxQueue, &msg, 0, 0);
	}
}
