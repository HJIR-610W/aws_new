


#include "cmsis_os.h"

#include "lwip.h"




const osThreadAttr_t ethernetTxTask_attributes = {
  .name = "ethernetTask",
  .stack_size = 1024*3,//2048바이트가 할당됨 하지만 4바이트 단위로 스택은 구성됨
  .priority = (osPriority_t) osPriorityNormal1,
};








void ethernetTask(void *arg)
{
uint8_t ip[4]={192,168,1,173};
uint8_t mask[4]={255,255,255,0};
uint8_t gw[4]={192,168,1,1};

  MX_LWIP_Init(ip,mask,gw);
  while(1)
  {
    osDelay(1000);
  }
}













void ethernetTask_init(void)
{
 
  osThreadNew(ethernetTask, NULL, &ethernetTxTask_attributes);

}