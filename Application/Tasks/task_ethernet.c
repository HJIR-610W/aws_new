


#include "cmsis_os.h"
#include "config_app.h"
#include "lwip.h"


#include "task_tcpServer.h"


const osThreadAttr_t ethernetTxTask_attributes = {
  .name = "ethernetTask",
  .stack_size = 1024*3,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityRealtime,
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

  osThreadExit();//종료 시킴


}





void ethernetTask_init(void)
{
 
  osThreadNew(ethernetTask, NULL, &ethernetTxTask_attributes);

}