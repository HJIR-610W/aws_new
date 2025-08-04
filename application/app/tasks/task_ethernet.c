


#include "cmsis_os.h"
#include "config_app.h"
#include "lwip.h"


#include "task_tcpServer.h"
#include "task_client.h"
#include "task_http_server.h"
#include "task_telnet_server.h"


const osThreadAttr_t ethernetTxTask_attributes = {
  .name = "ethernetTask",
  .stack_size = TASK_STACK(TASK_ETHERNET_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_ETHERNET_DEF),
};


/**
 * @brief 이더넷 초기화 해주고 종료
 */
void ethernetTask(void *arg)
{
  uint8_t *ip;
  uint8_t *mask;
  uint8_t *gw;

  ip   = config.eth_ip;
  mask = config.eth_subnet;
  gw   = config.eth_gateway;


  MX_LWIP_Init(ip,mask,gw);

  
  noti_tcpServerTask(0x00000001);
  noti_tcpClientTask(0x00000001);
  noti_httpServerTask(0x00000001);
  noti_telnetServerTask(0x00000001);
  osThreadExit();//종료 시킴


}



    void ethernetTask_init(void)
{
 
  osThreadNew(ethernetTask, NULL, &ethernetTxTask_attributes);

}

void ethernetPowerDownTask(void *arg)
{
  extern void ethernet_power_down(void);

  uint8_t *ip;
  uint8_t *mask;
  uint8_t *gw;

  ip = config.eth_ip;
  mask = config.eth_subnet;
  gw = config.eth_gateway;

  MX_LWIP_Init(ip, mask, gw);

  ethernet_power_down();

  osThreadExit(); // 종료 시킴
}

void ethernet_powerdown(void)
{
  osThreadNew(ethernetPowerDownTask, NULL, &ethernetTxTask_attributes);
}