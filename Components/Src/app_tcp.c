#include "app_tcp.h"
#include "lwip/sockets.h"
#include "main.h" // Для HAL_FLASH...
#include <string.h>

const osThreadAttr_t configTask_attributes = {
  .name = "ConfigTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t modbusTask_attributes = {
  .name = "ModbusTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

// --- Работа с Flash (Твой код) ---
void Flash_Save_User_Code(uint8_t *data, uint32_t len) {
  HAL_FLASH_Unlock();

  // 1. Стираем сектор 7
  FLASH_EraseInitTypeDef EraseInitStruct;
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
  EraseInitStruct.Sector = FLASH_SECTOR_7;
  EraseInitStruct.NbSectors = 1;
  EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;

  uint32_t SectorError;
  HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);

  // 2. Пишем Заголовок
  uint32_t write_addr = USER_CODE_ADDR;

  // Magic Key
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr, MAGIC_KEY);
  write_addr += 4;

  // Размер кода
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr, len);
  write_addr += 4;

  // 3. Пишем тело кода
  for (uint32_t i = 0; i < len; i++) {
    HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_addr + i, data[i]);
  }

  HAL_FLASH_Lock();

  // 4. Перезагрузка
  HAL_NVIC_SystemReset();
}

void convert_to_be_fast(uint32_t *src, uint8_t *dest, uint16_t count32) {
  uint32_t *dest32 = (uint32_t *)dest;
  for (uint16_t i = 0; i < count32; i++) {
    dest32[i] = __REV(src[i]);
  }
}

// --- Задача Modbus TCP ---
uint8_t tx[1080] __attribute__((aligned(4)));
uint8_t rx[260];

void ModbusTask(void *argument) {
  int server_sock, client_sock;
  struct sockaddr_in srv_addr;

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_addr.s_addr = INADDR_ANY;
  srv_addr.sin_port = htons(502);

  bind(server_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr));
  listen(server_sock, 3);

  for (;;) {
    client_sock = accept(server_sock, NULL, NULL);
    if (client_sock < 0) continue;

    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while (1) {
      int len = recv(client_sock, rx, sizeof(rx), 0);
      if (len < 12) break;

      uint8_t func = rx[7];
      uint16_t start_reg = (rx[8] << 8) | rx[9];
      uint16_t reg_count = (rx[10] << 8) | rx[11];

      // Проверка границ: 1 переменная = 2 регистра
      uint16_t max_allowed_reg = project_vars.var_count * 2;

      // Проверка на четность и выход за диапазон
      if (start_reg % 2 != 0 || reg_count % 2 != 0 || (start_reg + reg_count) > max_allowed_reg) {
        memcpy(tx, rx, 8);
        tx[7] |= 0x80; tx[8] = 0x02; // Illegal Data Address
        tx[4] = 0; tx[5] = 3;
        send(client_sock, tx, 9, 0);
        continue;
      }

      memcpy(tx, rx, 8);
      int tx_total = 0;

      if (func == 0x03) {
        tx[8] = reg_count * 2;

        // Используем твою функцию.
        // reg_count / 2 — это количество 32-битных переменных
        convert_to_be_fast(&project_vars.vars[start_reg / 2].as_int32, &tx[9], reg_count / 2);

        tx_total = 6 + 3 + (reg_count * 2);
      }
      else if (func == 0x10) {
        // Для записи используем ту же логику __REV
        uint32_t *src32 = (uint32_t *)&rx[13];
        uint32_t *dst32 = (uint32_t *)(((void*)&project_vars.vars[start_reg / 2].as_int32));

        for (int i = 0; i < (reg_count / 2); i++) {
          dst32[i] = __REV(src32[i]);
        }

        memcpy(&tx[8], &rx[8], 4);
        tx_total = 12;
      }

      if (tx_total > 0) {
        uint16_t mb_len = tx_total - 6;
        tx[4] = (mb_len >> 8); tx[5] = (mb_len & 0xFF);
        send(client_sock, tx, tx_total, 0);
      }
    }
    close(client_sock);
  }
}

// Вспомогательная функция гарантированного приема
int recv_full(int sock, void *ptr, int len) {
  int total = 0;
  char *p = (char *)ptr;
  while (total < len) {
    int n = recv(sock, p + total, len - total, 0);
    if (n <= 0) return -1;
    total += n;
  }
  return total;
}

void ConfigTask(void *argument) {
  int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(5000),
    .sin_addr.s_addr = INADDR_ANY
  };

  bind(server_sock, (struct sockaddr *)&addr, sizeof(addr));
  listen(server_sock, 1);

  // Буфер для приема чанков прошивки (выровнен для HAL)
  uint8_t chunk[CHUNK_SIZE] __attribute__((aligned(4)));

  for (;;) {
    int conn = accept(server_sock, NULL, NULL);
    if (conn < 0) {
      osDelay(10);
      continue;
    }

    // Устанавливаем таймаут на чтение (чтобы сокет не висел вечно при сбое связи)
    struct timeval tv = {.tv_sec = 5, .tv_usec = 0};
    setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    uint8_t cmd;
    if (recv(conn, &cmd, 1, 0) == 1) {
      uint8_t ack = 0xAA;

      switch (cmd) {
        // --- 1. ПРИЕМ ОСНОВНОЙ КОНФИГУРАЦИИ ---
        case CMD_SET_CONFIG:
          if (recv_full(conn, &kairos_config, sizeof(kairos_config)) > 0) {
            send(conn, &ack, 1, 0);
          }
          break;

        // --- 2. ПРИЕМ ПЕРЕМЕННЫХ ПРОЕКТА ---
        case CMD_SET_VARS:
          if (recv_full(conn, &project_vars, sizeof(project_vars)) > 0) {
            send(conn, &ack, 1, 0);
          }
          break;

        // --- 3. ПРОШИВКА USER LOGIC (.bin) ---
        case CMD_FLASH_BIN:
          uint32_t bin_len = 0;
          if (recv_full(conn, &bin_len, 4) > 0) {
            HAL_FLASH_Unlock();

            FLASH_EraseInitTypeDef EraseInitStruct = {
              .TypeErase = FLASH_TYPEERASE_SECTORS,
              .Sector = FLASH_SECTOR_7,
              .NbSectors = 1,
              .VoltageRange = FLASH_VOLTAGE_RANGE_3
            };
            uint32_t SectorError;

            if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK) {
              uint32_t write_addr = USER_CODE_ADDR;

              // Заголовок
              HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr, MAGIC_KEY);
              HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr + 4, bin_len);
              write_addr += 8;

              uint32_t remaining = bin_len;
              uint8_t success = 1;

              while (remaining > 0) {
                uint32_t to_recv = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
                if (recv_full(conn, chunk, to_recv) > 0) {
                  for (uint32_t i = 0; i < to_recv; i++) {
                    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_addr + i, chunk[i]) != HAL_OK) {
                      success = 0;
                      break;
                    }
                  }
                  write_addr += to_recv;
                  remaining -= to_recv;
                } else {
                  success = 0;
                  break;
                }
              }

              if (success) {
                send(conn, &ack, 1, 0);
                osDelay(500);
                HAL_FLASH_Lock();
                HAL_NVIC_SystemReset();
              }
            }
            HAL_FLASH_Lock();
          }
          break;

        default:
          // Неизвестная команда
          break;
      }
    }
    close(conn);
    osDelay(10);
  }
}

void StartNetworkTasks(void) {
  osThreadNew(ConfigTask, NULL, &configTask_attributes);
  osThreadNew(ModbusTask, NULL, &modbusTask_attributes);
}
