

#ifndef PROJECT_DEF_H
#define PROJECT_DEF_H

#include <stdint.h>

#define INFO_VER (0U)

#define SECTION_BOOT (0U)
#define SECTION_APP  (1U)

#define SECTION_TEST  (2U)

#define HW_NEW_DIVAS     (0U)
#define HW_NEW_RECORDER  (1U)
#define HW_MINILOGGER    (2U)


#define NICK_NEW_DIVAS_HJ     (0U)
#define NICK_NEW_DIVAS_VISION (1U)


#define NICK_MINILOGGER_HJ   (0U)

#define APP_VERSION        0x01000000U   // 바이트 단위

typedef struct section_info_s
{
    uint32_t ver;
    uint32_t section;
    uint32_t hw_code;
    uint32_t nick_code;
    uint32_t offset;
    uint32_t section_ver;
    uint32_t time;
    uint32_t pcb_n;
    uint32_t pcb[4];
}section_info_t;





#endif