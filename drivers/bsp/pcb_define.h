
#ifndef _PCB_DEFINE_H
#define _PCB_DEFINE_H


#include "pin\pcb_5_pin.h"
#include "os_user_def.h"

#define AWS_PCB_0_5 5
#define AWS_PCB_0_6 6

/*
외부 SRAM:IS61WV204816BLL-xxTLI 4MB
MCU SRAM:192KB(112+16+64(CCM))
112:0x20000000 ~ -0x2001BFFF
 16:0x2001C000 ~  0x2001FFFF
*/
#define AWS_PCB_VER AWS_PCB_0_5 

extern const char* pcbPortNameList[9];
extern const char* pcbPinNameList[9][16];

#endif
