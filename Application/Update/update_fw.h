#ifndef UPDATE_FW_H
#define UPDATE_FW_H

#include <stdint.h>

#define UPDATE_FW__REMOTE_PATH "0:Firmware/Remote/remoFw.bin"

#define UPDATE_LOCAL 0
#define UPDATE_REMOTE 1


#define MAGIC_UPDATE_FW_REMOTE  0xA5A5ABAB
#define MAGIC_UPDATE_FW_LACAL 0xABABA5A5


#define FW_FILE_OPEN_ERR 0x01
#define FW_FILE_MEM_ERR  0x02
#define FW_FILE_CRC_ERR  0x03

#define FW_ERR_PCB 0x10
#define FW_ERR_MFG 0x20
#define FW_ERR_AREA 0x40

uint8_t check_firmware(uint8_t local);
bool get_firmware_update(void);
void set_firmware_update(void);
void set_magic_value(uint32_t value);
#endif