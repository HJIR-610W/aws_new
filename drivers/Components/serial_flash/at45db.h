
#ifndef AT45DB_H
#define AT45DB_H

#include <stdint.h>

#include "cmsis_os.h"

#include "driver_interface.h"

typedef enum {
    AT45DB_OK = 0,
    AT45DB_ERROR = -1,
    AT45DB_TIMEOUT = -2,
    AT45DB_UNSUPPORTED = -3,
    AT45DB_INIT_FAILED = -4
} at45db_result_t;

typedef struct {
    uint8_t density_code;
    uint32_t capacity_bits;
    uint32_t total_pages;
    uint16_t page_size_standard;
    uint16_t page_size_binary;
    const char* device_name;
} at45db_device_info_t;

typedef struct {
    at45db_device_info_t device_info;
    uint16_t current_page_size;
    uint32_t total_capacity_bytes;
    bool is_binary_mode;
    bool is_initialized;
} at45db_chip_info_t;

void at45db_init(void);
at45db_result_t at45db_get_chip_info( at45db_chip_info_t *info);
int32_t at45db_write(uint32_t offset, uint8_t *pData, uint32_t dataLen);
void at45db_read(uint32_t offset, uint8_t *pBuff, uint32_t buffSize, uint32_t readLen);
#endif