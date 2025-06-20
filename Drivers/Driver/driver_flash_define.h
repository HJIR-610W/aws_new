
#ifndef FLASH_DEFINE_H
#define FLASH_DEFINE_H


#include "driver_interface.h"


typedef enum flash_ctrl_e
{
  eREAD_SIZE
}eFLASH_CTRL_t;

typedef struct flash_read_size_s
{
  int size;
} flash_read_size_t;

typedef struct flash_api_s
{
void (*read)(driver_t *drv, uint32_t offset, uint8_t *pBuff, uint32_t buffSize,
                 uint32_t readLen) ;
int32_t (*write)(driver_t *drv, uint32_t offset, uint8_t *pData, uint32_t dataLen);
  void (*read_page)(driver_t *driver,uint32_t page_num,unsigned char *pBuff);
  void (*write_page)(driver_t *driver,uint32_t page_num,unsigned char *pBuff);
  void (*ctrl)(driver_t *driver, eFLASH_CTRL_t ctrl, void *w_opt, void *r_opt, uint8_t *err);
} flash_api_t;

#endif