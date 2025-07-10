#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#include <stdint.h>
#include "cmsis_os2.h"
#include "cli_key_code.h"



void app_key_init(void);
int32_t get_button_key(uint32_t timeout_ms);
void scan_key(void);


#endif 