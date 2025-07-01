

#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H




#include "cmsis_os.h"

#include <stdint.h>

#include "driver_interface.h"

#define STM_SPI_1 0
#define STM_SPI_2 1

typedef struct driver_spi_s
{
    const char *name;
    uint8_t err;
    uint32_t num;
    void *api;
    void *apiCfg;
    void *sem;
}driver_spi_t;

typedef struct fm25l_cfg_s
{
  void* handle;
}stm32_spi_cfg_t;




driver_t *driver_spi_open(int num);
void driverex_spi_send_byte(driver_t *spi, uint8_t value);
void driverex_spi_send_bytes(driver_t *spi,uint8_t *data,uint16_t dataLen);
uint8_t driverex_spi_read_byte(driver_t *spi);
uint8_t driverex_spi_read_bytes(driver_t *spi,uint8_t *pBuff,uint16_t rLen);


void driverex_spi_pend_sem(driver_t *spi);
void driverex_spi_post_sem(driver_t *spi);
#endif
