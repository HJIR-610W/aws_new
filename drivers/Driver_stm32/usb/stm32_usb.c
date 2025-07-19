#include "cmsis_os2.h"

#include "pcb_define.h"
#include "usbd_desc.h"
#include "usbd_desc.h"
#include "usbd_cdc.h" 
#include "usbd_cdc_interface.h"
#include "system_err.h"

extern USBD_CDC_ItfTypeDef USBD_CDC_fops;
                                
USBD_HandleTypeDef  USBD_Device;


void usb_start(void)
{
  
    /* Init Device Library */
  if(USBD_Init(&USBD_Device, &FS_Desc, 0)!= USBD_OK)
  {
    ERROR_PRINTF("usb");
  }
  
  /* Add Supported Class */
  if(USBD_RegisterClass(&USBD_Device, USBD_CDC_CLASS)!= USBD_OK)
  {
    ERROR_PRINTF("usb");
  }
  
  /* Add CDC Interface Class */
  if(USBD_CDC_RegisterInterface(&USBD_Device, &USBD_CDC_fops)!= USBD_OK)
  {
    ERROR_PRINTF("usb");
  }
  
  /* Start Device Process */
  if(USBD_Start(&USBD_Device) != USBD_OK)
  {
    ERROR_PRINTF("usb");
  }
}


int32_t cdc_send(const uint8_t *p_data,uint16_t dataLen)
{
  int32_t retVal=dataLen;

  USBD_CDC_SetTxBuffer(&USBD_Device, (uint8_t*)p_data, dataLen);
  if(USBD_CDC_TransmitPacket(&USBD_Device) != USBD_OK)
  {
    retVal = -1;
  }
  return retVal;
}






void usbTask_init(void)
{


  usb_start();




}
