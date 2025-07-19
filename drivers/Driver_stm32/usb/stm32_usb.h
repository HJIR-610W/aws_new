

#ifndef USB_START_H

#define USB_START_H

void usbTask_init(void);

int32_t cdc_send(const uint8_t *p_data,uint16_t dataLen);

#endif