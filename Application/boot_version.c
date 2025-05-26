


#include "boot_version.h"
#include "hj_product_list.h"
#include "utile_time.h"


#define BOOT_INFO_START_ADDRESS (0x08000188U) 
/**
 * @brief 부트 버전 읽기
 * a.b.c.d
 */
uint32_t get_bootVer(uint8_t *a,uint8_t *b,uint8_t *c,uint8_t *d)
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
void get_bootBuild(DATE_TIME_BUF *build)
{
    section_info_t *info = (section_info_t *)BOOT_INFO_START_ADDRESS;

 

    time_cvt_secTotime(info->time+TIME_ZONE_SOULE,build);
    
}