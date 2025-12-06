

#ifndef DRIVER_485_H
#define DRIVER_485_H

#include "drv_uart_def.h"
#include "bsp_rs485.h"
#include "io_interface.h"


#define DRV_RS485_RS232_A BSP_RS485_RS232_A // 이 포트는 하드웨어 점퍼를 RS485로 해야 한다.
#define DRV_RS485_RS232_B BSP_RS485_RS232_B // 이 포트는 하드웨어 점퍼를 RS485로 해야 한다.
#define DRV_RS485_C BSP_RS485_C


typedef enum RS485_PORT_e
{
  eAPP_RS485_RS232_A,
  eAPP_RS485_RS232_B,
  eAPP_RS485_C,
  eAPP_RS485_MAX
} eRS485_PORT_t;



int32_t drv_rs485_init(int32_t num,void *opt,const char *owner);
int32_t drv_rs485_send(int32_t num, uint8_t *data, size_t len);
int32_t drv_rs485_recv(int32_t num, uint8_t *buffer, size_t len, uint32_t timeOutms);
int32_t drv_rs485_recv_opt(int32_t num, uint8_t *buffer, size_t len, uint32_t timeout1_ms, uint32_t timeout2_ms);
void drv_rs485_set(int32_t num, eUART_SET_OPTION_t cmd, void *option);
void drv_rs485_get(int32_t num, eUART_GET_OPTION_t cmd, void *value);
void drv_rs485_flush_rx(int32_t num);


uint16_t drv_rs485_get_portList(const char **list,size_t list_max);
uint16_t rs485_get_port_name_list(const char **list,size_t list_max);
void drv_rs485_inject(int32_t num, const uint8_t *data, size_t dataLen);

int32_t rs485_num_to_driver_num(int32_t num);


int32_t drv_rs485_io_send(io_if_t *io,const uint8_t *data,size_t len );
int32_t drv_rs485_io_recv(io_if_t *io,uint8_t *buffer,size_t len,uint32_t timeout_ms);
void drv_rs485_io_flush(io_if_t *io);

#endif