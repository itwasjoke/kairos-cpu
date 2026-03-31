#include "app_tcp.h"
#include "lwip/sockets.h"
#include <string.h>

const osThreadAttr_t configTask_attributes = {
  .name = "ConfigTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};

const osThreadAttr_t modbusTask_attributes = {
  .name = "ModbusTask",
  .stack_size = 1024 * 3,
  .priority = (osPriority_t) osPriorityNormal,
};

struct timeval tv = {.tv_sec = 0, .tv_usec = 500000};

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

/**
 * @brief Безопасно записывает 32-битные значения в буфер Modbus (Hi-word first)
 * @param src Указатель на массив uint32_t
 * @param dest_bytes Указатель на байтовый буфер (может быть не выровнен)
 * @param count32 Количество 32-битных переменных
 */
void convert_to_modbus_32bit(uint32_t *src, uint8_t *dest_bytes, uint16_t count32) {
    for (uint16_t i = 0; i < count32; i++) {
        uint32_t val = src[i];

        // Разбиваем на 16-битные слова
        uint16_t hi = htons((uint16_t)(val >> 16));
        uint16_t lo = htons((uint16_t)(val & 0xFFFF));

        // Копируем побайтово через memcpy, чтобы избежать Alignment Fault
        memcpy(&dest_bytes[i * 4], &hi, 2);
        memcpy(&dest_bytes[i * 4 + 2], &lo, 2);
    }
}

/**
 * @brief Безопасно читает 32-битные значения из буфера Modbus
 */
void convert_from_modbus_32bit(uint8_t *src_bytes, uint32_t *dest, uint16_t count32) {
    for (uint16_t i = 0; i < count32; i++) {
        uint16_t hi_be, lo_be;

        // Безопасно забираем байты из сетевого пакета
        memcpy(&hi_be, &src_bytes[i * 4], 2);
        memcpy(&lo_be, &src_bytes[i * 4 + 2], 2);

        uint16_t hi = ntohs(hi_be);
        uint16_t lo = ntohs(lo_be);

        dest[i] = ((uint32_t)hi << 16) | lo;
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

static uint8_t modbus_tx[1080] __attribute__((aligned(4)));
static uint8_t modbus_rx[260];
/**
 * @brief Задача Modbus TCP Server
 */
void ModbusTask(void *argument) {


    int server_sock;
    struct sockaddr_in srv_addr;

    // Настройка таймаута (например, 5 секунд)
    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;

    // Создание сокета
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0) return;

    // Настройка адреса
    srv_addr.sin_family = AF_INET;
    srv_addr.sin_addr.s_addr = INADDR_ANY;
    srv_addr.sin_port = htons(502);

    // Разрешаем повторное использование адреса (полезно при перезапусках)
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_sock, (struct sockaddr *)&srv_addr, sizeof(srv_addr)) < 0) {
        close(server_sock);
        return;
    }

    listen(server_sock, 5);

    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t addr_len = sizeof(client_addr);

        // Ожидание нового подключения
        int client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);

        if (client_sock < 0) {
            osDelay(10);
            continue;
        }

        // Устанавливаем таймаут на прием данных
        setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        // Цикл обслуживания КОНКРЕТНОГО клиента (Keep-Alive)
        while (1) {
						int res = recv_full(client_sock, modbus_rx, 7); // Читаем MBAP Header
						if (res <= 0) break;

						uint16_t payload_len = (modbus_rx[4] << 8) | modbus_rx[5];
						if (payload_len > (sizeof(modbus_rx) - 7)) break; // Защита от переполнения

						res = recv_full(client_sock, &modbus_rx[7], payload_len); // Читаем остаток
						if (res <= 0) break;

						int len = 7 + payload_len;

            uint8_t func = modbus_rx[7];
            uint16_t start_reg = (modbus_rx[8] << 8) | modbus_rx[9];
            uint16_t reg_count = (modbus_rx[10] << 8) | modbus_rx[11];
            uint16_t max_allowed_reg = kairos_config.var_count * 2;

            int tx_total = 0;

            // 1. Проверка адресации (кратность 2 и границы)
            if (start_reg % 2 != 0 || reg_count % 2 != 0 || (start_reg + reg_count) > max_allowed_reg) {
                memcpy(modbus_tx, modbus_rx, 8); // MBAP Header
                modbus_tx[7] |= 0x80;     // Error flag
                modbus_tx[8] = 0x02;      // Exception: Illegal Data Address
                modbus_tx[4] = 0; modbus_tx[5] = 3;
                tx_total = 9;
            }
            // 2. Проверка на переполнение выходного буфера
            else if (9 + (reg_count * 2) > sizeof(modbus_tx)) {
                memcpy(modbus_tx, modbus_rx, 8);
                modbus_tx[7] |= 0x80;
                modbus_tx[8] = 0x04;      // Exception: Slave Device Failure
                modbus_tx[4] = 0; modbus_tx[5] = 3;
                tx_total = 9;
            }
            else {
                // Формируем стандартный заголовок ответа (копируем Transaction ID и Unit ID)
                memcpy(modbus_tx, modbus_rx, 8);

                if (func == 0x03) { // Read Holding Registers
                	modbus_tx[8] = (uint8_t)(reg_count * 2); // Byte count

                    // Безопасная конвертация из системы в Modbus
                    convert_to_modbus_32bit(
                        &project_vars.vars[start_reg / 2].as_uint32,
                        &modbus_tx[9],
                        reg_count / 2
                    );

                    tx_total = 6 + 3 + (reg_count * 2);
                }
                else if (func == 0x10) { // Write Multiple Registers
                    // Проверяем, что в пакете реально пришли данные, которые мы ожидаем
                    uint8_t bytes_to_write = modbus_rx[12];
                    if (len >= (13 + bytes_to_write)) {
                        // Безопасная конвертация из Modbus в систему
                        convert_from_modbus_32bit(
                            &modbus_rx[13],
                            &project_vars.vars[start_reg / 2].as_uint32,
                            reg_count / 2
                        );

                        // Ответ для 0x10: функция + адрес + количество (копируем из запроса)
                        memcpy(&modbus_tx[8], &modbus_rx[8], 4);
                        tx_total = 12;
                    }
                }
                else {
                    // Функция не поддерживается
                	modbus_tx[7] |= 0x80;
                	modbus_tx[8] = 0x01; // Illegal Function
                	modbus_tx[4] = 0; modbus_tx[5] = 3;
                    tx_total = 9;
                }
            }

            // Отправка ответа, если он сформирован
            if (tx_total > 0) {
                // Обновляем длину в заголовке MBAP (байты 4-5)
                uint16_t mb_payload_len = tx_total - 6;
                modbus_tx[4] = (uint8_t)(mb_payload_len >> 8);
                modbus_tx[5] = (uint8_t)(mb_payload_len & 0xFF);

                if (send(client_sock, modbus_tx, tx_total, 0) < 0) {
                    break; // Ошибка сети — закрываем этого клиента
                }
            }
        } // Конец while(1) клиента

        // Закрываем сокет и возвращаемся к accept() для нового клиента
        close(client_sock);
        osDelay(10);
    }
}

void ConfigTask(void *argument) {
  int server_sock = socket(AF_INET, SOCK_STREAM, 0);
  struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port = htons(5000),
    .sin_addr.s_addr = INADDR_ANY
  };

  if (server_sock < 0) return;

  // Разрешаем повторное использование адреса (полезно при ребутах)
  int opt = 1;
  setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    close(server_sock);
    return;
  }

  listen(server_sock, 4);

  // Буфер в стеке (8Кб стека позволяют выделить 1Кб под чанк)
//  uint8_t chunk[CHUNK_SIZE] __attribute__((aligned(4)));

  for (;;) {
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int conn = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len);

    if (conn < 0) {
      osDelay(10);
      continue;
    }

    // Стандартный таймаут 1 секунда для команд
    struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};
    setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));

    uint8_t cmd = 0;
    int res = recv(conn, &cmd, 1, 0);

    if (res > 0) {
      uint8_t ack = 0xAA;
      uint8_t nack = 0xEE; // Ошибка

      switch (cmd) {
        case CMD_SET_CONFIG:
          if (recv_full(conn, &kairos_config, sizeof(kairos_config)) > 0) {
						new_config = 1;
						Led_Blink(LED_2, 100);
						send(conn, &ack, 1, 0);
					} else send(conn, &nack, 1, 0);
          break;

//        case CMD_FLASH_BIN: {
//          uint32_t bin_len = 0;
//          if (recv_full(conn, &bin_len, 4) <= 0) break;
//
//          // УВЕЛИЧИВАЕМ ТАЙМАУТ: Стирание Flash долгое!
//          struct timeval long_timeout = {.tv_sec = 5, .tv_usec = 0};
//          setsockopt(conn, SOL_SOCKET, SO_RCVTIMEO, &long_timeout, sizeof(long_timeout));
//
//          HAL_FLASH_Unlock();
//          FLASH_EraseInitTypeDef EraseInitStruct = {
//            .TypeErase = FLASH_TYPEERASE_SECTORS,
//            .Sector = FLASH_SECTOR_7,
//            .NbSectors = 1,
//            .VoltageRange = FLASH_VOLTAGE_RANGE_3
//          };
//          uint32_t SectorError;
//
//          if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) == HAL_OK) {
//            uint32_t write_addr = USER_CODE_ADDR;
//            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr, MAGIC_KEY);
//            HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, write_addr + 4, bin_len);
//            write_addr += 8;
//
//            uint32_t remaining = bin_len;
//            uint8_t success = 1;
//
//            while (remaining > 0) {
//              uint32_t to_recv = (remaining > CHUNK_SIZE) ? CHUNK_SIZE : remaining;
//              if (recv_full(conn, chunk, to_recv) > 0) {
//                for (uint32_t i = 0; i < to_recv; i++) {
//                  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, write_addr + i, chunk[i]) != HAL_OK) {
//                    success = 0; break;
//                  }
//                }
//                if (!success) break;
//                write_addr += to_recv;
//                remaining -= to_recv;
//              } else {
//                success = 0; break;
//              }
//            }
//
//            if (success) {
//              send(conn, &ack, 1, 0);
//              osDelay(200);
//              HAL_FLASH_Lock();
//              close(conn); // Закрываем сокет ПЕРЕД ресетом
//              HAL_NVIC_SystemReset();
//            } else {
//              send(conn, &nack, 1, 0);
//            }
//          }
//          HAL_FLASH_Lock();
//          break;
//        }

        // По умолчанию просто игнорируем
        default: break;
      }
    }

    close(conn);
    Led_Blink(LED_2, 20);
    osDelay(5);
  }
}

void StartNetworkTasks(void) {
  osThreadNew(ConfigTask, NULL, &configTask_attributes);
  osThreadNew(ModbusTask, NULL, &modbusTask_attributes);
}
