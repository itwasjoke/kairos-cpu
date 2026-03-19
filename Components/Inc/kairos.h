/*
 * kairos.h
 *
 *  Created on: Feb 6, 2026
 *      Author: itwasjoke
 */

#ifndef KAIROS_H_
#define KAIROS_H_

#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"

// Core API and Config
#include "api_interface.h"
#include "Config.h"

// Hardware interfaces
#include "i2c.h"
#include "eeprom.h"
#include "led.h"
#include "analog.h"
#include "Discrete.h"
#include "CAN.h"
#include "RS485.h"

// Networking
#include "Ethernet.h"
#include "app_tcp.h"

// Math/Control
#include "universal_pid.h"

#endif /* KAIROS_H_ */