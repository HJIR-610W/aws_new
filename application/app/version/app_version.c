


#include "app_version.h"
#include "util_time.h"
#include "system_err.h"

#define MCU_SRAM_START_ADDR 0x20000000   // MCU SRAM 시작 주소

#define SYSTEM_SHARE_VAR_ADDR MCU_SRAM_START_ADDR
#pragma location = SYSTEM_SHARE_VAR_ADDR
__no_init volatile uint32_t _shareData;


#define APP_INFO_START_ADDRESS (0x08000188 + 0x00010000) // 벡터가 끝나는 곳
#pragma location = APP_INFO_START_ADDRESS
__root const section_info_t g_kappInfo = {.signature ={'A','P','P',' '},\
                                            .ver = INFO_VER,

#if USE_DEBUG
                                       .section = SECTION_TEST,
#else
                                       .section = SECTION_APP,
#endif
                                       .hw_code = HW_NEW_ASW,\
                                       .alias_code =ALIAS_NEW_ASW_HJ,\
                                       .offset = 0x08010000,\
                                       .section_ver = APP_VERSION,\
                                       .build_timestamp = 1763116666,\
                                       .pcb_n =1,
                                       .pcb[0]=0x01000000};




void get_nickCode(uint32_t *nickCode)
{
  *nickCode = g_kappInfo.alias_code;
}

void get_hwCode(uint32_t *hwCode)
{

    *hwCode = g_kappInfo.hw_code;
}

uint32_t get_app_version(uint8_t *major, uint8_t *minor, uint8_t *patch, uint8_t *release)
{
    uint32_t ver;

    ver = g_kappInfo.section_ver;

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




#ifndef TIME_ZONE_SOULE
#define TIME_ZONE_SOULE 32400
#endif
/**
 * @brief 부트 빌드 시간 읽기
 */
void get_app_build(DATE_TIME_BUF *build)
{
    uint32_t data;

    data = g_kappInfo.build_timestamp;
    time_cvt_secTotime(data,build);
}

uint32_t get_app_build_timestamp(void)
{
  uint32_t data;

  data = g_kappInfo.build_timestamp ;

  return data;
 
}

uint32_t get_hardware_code(void)
{ 
  return g_kappInfo.hw_code;

}

uint32_t get_app_alias(void) 
{
  return g_kappInfo.alias_code;
}

/**
 * 


INFO:test 프로그램을 사용하지 않으려면 없어도 되는 함수
*/
void set_testKey(uint32_t key) 
{
  _shareData = key;
}

const char *mfgList[] = {"HJ"};


const char *get_mfg_name(void)
{
  uint32_t mfg_ver;

  mfg_ver = get_app_alias();

  if(mfg_ver < (sizeof(mfgList)/sizeof(mfgList[0])))
  {
    return mfgList[mfg_ver];
  }
  else
  {
    return "UNKNOWN";
  }
}

uint32_t get_app_area_code(void)
{
    return g_kappInfo.area;
}