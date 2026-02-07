#ifndef CAN_BUS_H
#define CAN_BUS_H

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

typedef struct {
  uint32_t id;
  uint32_t id_type;
  uint8_t dlc;
  uint8_t data[8];
} CAN_Message_t;

typedef struct {
  CAN_HandleTypeDef *hcan;
  CAN_TypeDef *instance;
  uint32_t baud_rate;
  uint32_t filter_id;
  uint32_t filter_mask;
  osMessageQueueId_t rxQueue;
} CAN_Config_t;

HAL_StatusTypeDef CAN_Bus_Init(CAN_Config_t *config);
osStatus_t CAN_Bus_Read(CAN_Config_t *config, CAN_Message_t *msg, uint32_t timeout);

#endif
