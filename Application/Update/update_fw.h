#ifndef UPDATE_FW_H
#define UPDATE_FW_H

#include <stdint.h>

#define UPDATE_FW__REMOTE_PATH "0:Firmware/Remote/remoFw.bin"

#define UPDATE_LOCAL 0
#define UPDATE_REMOTE 1
void update_fw(uint8_t local);
#endif