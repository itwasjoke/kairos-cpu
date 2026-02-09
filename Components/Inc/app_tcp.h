/*
 * app_tcp.h
 *
 *  Created on: Feb 9, 2026
 *      Author: itwasjoke
 */

#ifndef APP_TCP_H_
#define APP_TCP_H_

#include <api_interface.h>
#include "Config.h"
#include <stdint.h>
#include "cmsis_os.h" // CMSIS RTOS V2

#define CMD_SET_CONFIG  0x01
#define CMD_SET_VARS    0x02
#define CMD_FLASH_BIN   0x03

#define CHUNK_SIZE 1024

// Функция запуска задач
void StartNetworkTasks(void);

#endif /* APP_TCP_H_ */
