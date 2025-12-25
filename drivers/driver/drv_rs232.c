
#include "drv_rs232.h"

#include "bsp_uart.h"
#include "util_memory.h"


typedef struct app_rs232_s
{
  uint8_t num;
  const char *name;
}app_rs232_t;

const app_rs232_t rs232_define[] = {{.num = DRV_UART_2_EXT_A, .name = "A"},
                                    {.num = DRV_UART_3_EXT_B, .name = "B"},
                                    {.num = DRV_UART_4_EXT_C, .name = "C"}};


const char *rs232_port_name_list[3] = {"A","B","C"};


#define RS232_OWNER_SIZE 20
char rs232_owner_table[16][RS232_OWNER_SIZE]={{"A"},
{"B"},
{"C"}};

const char *g_rs232_owner_list[3] = {
    rs232_owner_table[0],
    rs232_owner_table[1],
    rs232_owner_table[2]};

void update_rs232_owner(eRS232_PORT_t port,const char *owner)
{

  switch(port)
  {
    case eRS232_RS485_A:
        snprintf(rs232_owner_table[port],RS232_OWNER_SIZE,"A %s",owner);
    break;
    case eRS232_RS485_B:
        snprintf(rs232_owner_table[port],RS232_OWNER_SIZE,"B %s",owner);
    break;
    case eRS232_C:
        snprintf(rs232_owner_table[port],RS232_OWNER_SIZE,"C %s",owner);
    break;
  }

}




int32_t uart_num_to_driver_num(int32_t app_uart_num)
{

  if (app_uart_num > eRS232_MAX)
    return rs232_define[0].num;

  return rs232_define[app_uart_num].num;

  
}


uint16_t rs232_get_portList(const char **list,size_t listMax)
{
  uint32_t i=0;


  for( i = 0; i <_countof(rs232_define);i++)
  {
    if(i<listMax)
    {
      list[i] = rs232_owner_table[i];//rs232_define[i].name;
    }
    else
    {
      list[i] = 0;
    }
  }
  return i;
}

uint16_t rs232_get_port_name_list(const char **list,size_t listMax)
{
  uint32_t i=0;


  for( i = 0; i <_countof(rs232_define);i++)
  {
    if(i<listMax)
    {
      list[i] = rs232_port_name_list[i];//rs232_define[i].name;
    }
    else
    {
      list[i] = 0;
    }
  }
  return i;
}


int32_t drv_uart_init(int32_t num, void *opt,const char *owner)
{
  int32_t status=0;

  switch(num)
  {
case DRV_UART_0_VHF:     // VHF
break;
case DRV_UART_2_EXT_A:  // 사용자0
   update_rs232_owner(eRS232_RS485_A,owner);
   break;
case DRV_UART_3_EXT_B:  // 사용자1
   update_rs232_owner(eRS232_RS485_B,owner);
   break;
case DRV_UART_4_EXT_C:  // 사용자2
   update_rs232_owner(eRS232_C,owner);
   break;

case DRV_UART_5_EXT_D:  // 사용자3
case DRV_UART_8_CDMA  :  // CDMA
case DRV_UART_10_CDC :  // USB 디버깅

    break;
  }

  status =  bsp_uart_init(num, opt);

  return status;
}


void drv_uart_deinit(int32_t num)
{
  bsp_uart_deinit(num);
}


int32_t drv_uart_send(int32_t num, const uint8_t *data, size_t dataLen)
{
  return bsp_uart_send(num, data, dataLen);
}

int32_t drv_uart_recv(int32_t num, uint8_t *buffer, size_t len,uint32_t timeOutMs)
{
  return bsp_uart_recv(num, buffer, len, timeOutMs);
}

int32_t drv_uart_recv_crlf(int32_t num, char *buffer, size_t len,uint32_t tout_ms)
{
  return bsp_uart_recv_crlf(num, buffer, len, tout_ms);
}
void drv_uart_set(int32_t num, eUART_SET_OPTION_t cmd, void *para){
  bsp_uart_set(num, cmd, para);
}

// 밑에 두함수는 대체 필요
int32_t drv_uart_get_char(int32_t num, uint8_t *buffer)
{
  return bsp_uart_get_char(num, buffer);
}

int32_t drv_uart_get_charNonBlocking(int32_t num, uint8_t *buffer)
{
  return bsp_uart_get_charNonBlocking(num, buffer);
}

void drv_uart_flush_rx(int32_t num)
{
  bsp_uart_flush_rx(num);
}

int32_t drv_uart_recv_opt(int32_t num, uint8_t *buffer, size_t len,
                          uint32_t timeout1_ms, uint32_t timeout2_ms)
                          {
  return bsp_uart_recv_opt(num, buffer, len, timeout1_ms, timeout2_ms);

}

void drv_uart_get(int32_t num, eUART_GET_OPTION_t cmd, void *para)
{
  bsp_uart_get(num, cmd, para);
}

int32_t drv_uart_recv_ll(int32_t num, uint8_t *buffer, size_t len, uint32_t timeOutMs)
{
  return bsp_uart_recv_ll(num, buffer, len, timeOutMs);
}

int32_t drv_uart_inject(int32_t num, const uint8_t *data, size_t dataLen)
{
  return bsp_uart_inject(num, data, dataLen);
}






/*
io_interface 사용을 위해 wrapper함수 
*/


int drv_uart_io_send(io_if_t *io,const uint8_t *data,size_t len )
{
  return bsp_uart_send(io->dev_num, data, len);
}

int drv_uart_io_recv(io_if_t *io,uint8_t *buffer,size_t len,uint32_t timeout_ms)
{
  return drv_uart_recv(io->dev_num, buffer, len, timeout_ms);
}

void drv_uart_io_flush(io_if_t *io)
{
    bsp_uart_flush_rx(io->dev_num);
}

void drv_uart_io_inject(io_if_t *io,uint8_t *data,size_t len)
{
   bsp_uart_inject( io->dev_num, data, len);
}