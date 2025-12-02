


#include "boot_version.h"
#include "product.h"
#include "util_time.h"


#define BOOT_INFO_START_ADDRESS (0x08000188U) 
/**
 * @brief 부트 버전 읽기
 * a.b.c.d
 */
uint32_t get_boot_version(uint8_t *a,uint8_t *b,uint8_t *c,uint8_t *d)
{
  section_info_t *info = (section_info_t *)BOOT_INFO_START_ADDRESS;

  uint32_t ver;

  ver = info->section_ver;

  *a = (ver>>24)&0xFF;
  *b = (ver>>16)&0xFF;
  *c = (ver>>8)&0xFF;
  *d = ver&0xFF; 

  return ver;
}

#ifndef TIME_ZONE_SOULE
#define TIME_ZONE_SOULE 32400
#endif
/**
 * @brief 부트 빌드 시간 읽기
 */
void get_boot_build(DATE_TIME_BUF *build)
{
  section_info_t *info = (section_info_t *)BOOT_INFO_START_ADDRESS;

  time_cvt_secTotime(info->build_timestamp + TIME_ZONE_SOULE, build);
}

uint32_t get_boot_build_timestamp(void)
{
  section_info_t *info = (section_info_t *)BOOT_INFO_START_ADDRESS;
  return info->build_timestamp;
}

uint32_t get_boot_pcb_version(void)
{
  section_info_t *info = (section_info_t *)BOOT_INFO_START_ADDRESS;

  return info->pcb[0];//부트로더에 PCB 인덱스 0 PCB고유 버전 기록
}