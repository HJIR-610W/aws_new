

#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include <stdint.h>


#define BSP_SPI_1 0
#define BSP_SPI_2 1
#define BSP_SPI_MAX 2

void bsp_spi_init(int num);
void bsp_spi_send_byte(int num, uint8_t value);
void bsp_spi_send_bytes(int num, uint8_t *data, uint16_t dataLen);
uint8_t bsp_spi_read_byte(int num);
uint8_t bsp_spi_read_bytes(int num, uint8_t *pBuff, uint16_t rLen);
void bsp_spi_pend_sem(int num);
void bsp_spi_post_sem(int num);
#endif
