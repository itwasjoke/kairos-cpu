#include "can.h"
#include "Config.h"

static CAN_Config_t *g_can_config;

/**
 * @brief Вспомогательная функция для извлечения битов из 8-байтового массива данных CAN.
 * Поддерживает Little Endian упаковку данных.
 */
static uint32_t extract_bits(uint8_t *data, uint16_t start_bit, uint8_t bit_len) {
    uint64_t payload = 0;
    // Собираем 64-битное число из 8 байт (Little Endian)
    for (int i = 0; i < 8; i++) {
        payload |= ((uint64_t)data[i] << (i * 8));
    }
    
    // Сдвигаем и маскируем нужную область
    uint64_t mask = (bit_len >= 64) ? 0xFFFFFFFFFFFFFFFF : (((uint64_t)1 << bit_len) - 1);
    return (uint32_t)((payload >> start_bit) & mask);
}

/**
 * @brief Задача обработки входящих CAN-сообщений.
 * Выполняет разбор данных согласно конфигурации и кладет их в project_vars.
 */
void CAN_Task(void *argument) {
    CANr_Message_t rx_msg;
    
    for (;;) {
        // Ожидание сообщения из очереди
        if (CAN_Bus_Read(g_can_config, &rx_msg, osWaitForever) == osOK) {
            
            // Стартовый индекс переменных CAN в глобальном массиве
            uint16_t current_var_offset = kairos_config.main_config.can_start_var;

            // Перебор всех ожидаемых сообщений в конфигурации
            for (uint8_t i = 0; i < kairos_config.gpio_config.can_count; i++) {
                CanMessage_t *conf_msg = &kairos_config.gpio_config.can_messages[i];

                // Проверка совпадения ID
                if (rx_msg.id == conf_msg->id) {
                    
                    // Проверка IDC (если IDC != 0, считаем его как минимально допустимый DLC)
                    if (conf_msg->IDC != 0 && rx_msg.dlc < conf_msg->IDC) {
                        break; 
                    }

                    // Разбор переменных внутри сообщения
                    for (uint8_t v = 0; v < conf_msg->var_count; v++) {
                        VarDesc_t *v_desc = &conf_msg->vars[v];
                        uint16_t target_idx = current_var_offset + v;
                        
                        if (target_idx >= MAX_VARS) break;

                        switch (v_desc->type) {
                            case VAR_TYPE_BOOL:
                                project_vars.vars[target_idx].as_bool = (uint8_t)extract_bits(rx_msg.data, v_desc->start_bit, 1);
                                break;
                            case VAR_TYPE_INT8:
                                project_vars.vars[target_idx].as_uint8 = (uint8_t)extract_bits(rx_msg.data, v_desc->start_bit, 8);
                                break;
                            case VAR_TYPE_INT16:
                                project_vars.vars[target_idx].as_uint16 = (uint16_t)extract_bits(rx_msg.data, v_desc->start_bit, 16);
                                break;
                            case VAR_TYPE_INT32:
                                project_vars.vars[target_idx].as_uint32 = extract_bits(rx_msg.data, v_desc->start_bit, 32);
                                break;
                            case VAR_TYPE_FLOAT: {
                                uint32_t raw = extract_bits(rx_msg.data, v_desc->start_bit, 32);
                                project_vars.vars[target_idx].as_float = *(float*)&raw;
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    // Сообщение обработано, выходим из цикла поиска ID
                    break;
                }
                
                // Накапливаем смещение: каждая конфигурация сообщения занимает var_count регистров
                current_var_offset += conf_msg->var_count;
            }
        }
    }
}

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
    config->rxQueue = osMessageQueueNew(10, sizeof(CANr_Message_t), NULL);
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

osStatus_t CAN_Bus_Read(CAN_Config_t *config, CANr_Message_t *msg, uint32_t timeout) {
	return osMessageQueueGet(config->rxQueue, msg, NULL, timeout);
}

// Прерывание: вызывается при получении нового сообщения
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan) {
	CAN_RxHeaderTypeDef header;
	CANr_Message_t msg;

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

		Led_Blink(LED_CAN_RX, 100);

    // Кладём в очередь. Из ISR используем NULL для таймаута
		osMessageQueuePut(g_can_config->rxQueue, &msg, 0, 0);
	}
}
