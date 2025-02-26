#include "cmsis_os2.h"

#include "pcb_define.h"
#include "usbd_desc.h"
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_interface.h"

#define APP_RX_DATA_SIZE  2048
#define APP_TX_DATA_SIZE  2048



extern USBD_CDC_ItfTypeDef USBD_CDC_fops;
uint8_t UserRxBuffer[APP_RX_DATA_SIZE];/* Received Data over USB are stored in this buffer */
uint8_t UserTxBuffer[APP_TX_DATA_SIZE];/* Received Data over UART (CDC interface) are stored in this buffer */
uint32_t BuffLength;
uint32_t UserTxBufPtrIn = 0;/* Increment this pointer or roll it back to
                               start address when data are received over USART */
uint32_t UserTxBufPtrOut = 0; /* Increment this pointer or roll it back to
                                 start address when data are sent over USB */
USBD_HandleTypeDef  USBD_Device;

const osThreadAttr_t usbTask_attributes = {
  .name = "usbTask",
  .stack_size = 1024,
  .priority = (osPriority_t) osPriorityNormal1,
};


void usb_start(void)
{
  
    /* Init Device Library */
  if(USBD_Init(&USBD_Device, &FS_Desc, 0)!= USBD_OK)
  {
    while(1);
  }
  
  /* Add Supported Class */
  if(USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS)!= USBD_OK)
  {
    while(1);
  }
  
  /* Add CDC Interface Class */
  if(USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops)!= USBD_OK)
  {
    while(1);
  }
  
  /* Start Device Process */
  if(USBD_Start(&USBD_Device) != USBD_OK)
  {
    while(1);
  }
}

void usbTask(void *arg)
{
    uint32_t buffptr;
  uint32_t buffsize;
  

  
  while(1)
  {
    osDelay(100);
      UserTxBufPtrIn = snprintf(UserTxBuffer,sizeof(UserTxBuffer),"test hello");
      
    if(UserTxBufPtrOut != UserTxBufPtrIn)
    {
      if(UserTxBufPtrOut > UserTxBufPtrIn) /* Rollback */
      {
        buffsize = APP_TX_DATA_SIZE - UserTxBufPtrOut;
      }
      else 
      {
        buffsize = UserTxBufPtrIn - UserTxBufPtrOut;
      }
    
    buffptr = UserTxBufPtrOut;
    
    USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)&UserTxBuffer[buffptr], buffsize);
    
    if(USBD_CDC_TransmitPacket(&USBD_Device) == USBD_OK)
    {
        UserTxBufPtrOut = 0;
#if 0
      UserTxBufPtrOut += buffsize;
      if (UserTxBufPtrOut > APP_RX_DATA_SIZE)
      {
        UserTxBufPtrOut = 0;
      }
#endif
    }
    
    
  }
  }
}



void usbTask_init(void)
{
  usb_start();

  osThreadNew(usbTask, NULL, &usbTask_attributes);

}
