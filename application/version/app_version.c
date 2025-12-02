


#include "app_version.h"
#include "product.h"
#include "util_time.h"
#include "util_memory.h"

#define MCU_SRAM_START_ADDR 0x20000000   // MCU SRAM 시작 주소

#define SYSTEM_SHARE_VAR_ADDR MCU_SRAM_START_ADDR
#pragma location = SYSTEM_SHARE_VAR_ADDR
__no_init volatile uint32_t _shareData;


#define APP_INFO_START_ADDRESS (0x08000188 + 0x00010000) // 벡터가 끝나는 곳
#pragma location = APP_INFO_START_ADDRESS
//2025년 11월 27일 목요일 오전 11:24:56 GMT+09:00
__root const section_info_t k_app_section = {.signature ={'A','P','P',' '},\
                                          .ver = INFO_VER,
                                          .section = SECTION_APP,
                                          .product_code = PRODUCT_NEW_AWS,\
                                          .alias_code =ALIAS_NEW_ASW_HJ,\
                                          .offset = 0x08010000,\
                                          .section_ver = APP_VERSION,\
                                          .build_timestamp = 1764210296,\
                                          .pcb_n =1,
                                          .pcb[0]=0x01000000};
 


uint32_t get_app_version(uint8_t *major, uint8_t *minor, uint8_t *patch, uint8_t *release)
{
    uint32_t ver;

    ver = k_app_section.section_ver;

    if(major)
    {
      *major = (ver>>24)&0xFF;
    }
    if(minor)
    {
      *minor = (ver >> 16) & 0xFF;
    }

    if(patch)
    {
      *patch = (ver >> 8) & 0xFF;
    }

    if(release)
    {
      *release = ver & 0xFF;
    }


  return ver;
}



/**
 * @brief 부트 빌드 시간 읽기
 */
void get_app_build(DATE_TIME_BUF *build)
{
    uint32_t data;

    data = k_app_section.build_timestamp;
    time_cvt_secTotime(data,build);
}

uint32_t get_app_build_timestamp(void)
{
  uint32_t data;

  data = k_app_section.build_timestamp ;

  return data;
 
}

uint32_t get_product_code(void)
{ 
  return k_app_section.product_code;

}

uint32_t get_app_alias(void) 
{
  return k_app_section.alias_code;
}

const char *get_alias_name(void)
{
  uint32_t alias_code;

  alias_code = get_app_alias();
  if(alias_code < ALIAS_COUNT)
  return alias_name_list[alias_code];
  else
  return "Unknown";
}

uint32_t get_app_area_code(void)
{
    return k_app_section.area;
}