

#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include <stdint.h>

#include "cmsis_os.h"
#include "driver_interface.h"

#define STM_SPI_1 0
#define STM_SPI_2 1


driver_t *driver_spi_open(int num);
void driver_spi_send_byte(driver_t *spi, uint8_t value);
void driver_spi_send_bytes(driver_t *spi,uint8_t *data,uint16_t dataLen);
uint8_t driver_spi_read_byte(driver_t *spi);
uint8_t driver_spi_read_bytes(driver_t *spi,uint8_t *pBuff,uint16_t rLen);


void driver_spi_pend_sem(driver_t *spi);
void driver_spi_post_sem(driver_t *spi);
#endif
