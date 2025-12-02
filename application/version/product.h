
#ifndef PRODUCT_H
#define PRODUCT_H
#include <stdint.h>


#define PRODUCT_LIST               \
  X(PRODUCT_NEW_DIVAS, "HJIR-540")\
  X(PRODUCT_NEW_RECORDER, "HJIR-310")\
  X(PRODUCT_NEW_MINILOGGER, "-")\
  X(PRODUCT_NEW_AWS, "HJIR-610W")\
  X(PRODUCT_GATE_CONTROL, "HJBC-400")

#define ALIAS_LIST \
  X(ALIAS_HJ, "HJ") \
  X(ALIAS_NEW_ASW_HJ, "HJ")

#define SECTION_LIST \
  X(SECTION_BOOT, "Boot") \
  X(SECTION_APP, "App") \
  X(SECTION_CONST, "Const") \
  X(SECTION_LIB, "Lib")

typedef enum {
#define X(code, name) code,
  PRODUCT_LIST
#undef X
  PRODUCT_COUNT
} product_code_e;

typedef enum {
#define X(code, name) code,
  ALIAS_LIST
#undef X
  ALIAS_COUNT
} alias_code_e;


#define INFO_VER     (0U)



typedef struct section_info_s
{
  char signature[4];//0
  uint32_t ver;     //4 섹션 버전
  uint32_t section;//8 파일이 저장되는 영역
  uint32_t product_code;//12 제품 구분
  uint32_t alias_code;//16 별칭
  uint32_t offset;//20 펌웨어시작주소
  uint32_t section_ver;//24
  uint32_t build_timestamp;//28 빌드 시간
  uint32_t area;//32 지역
  uint32_t pcb_n;//36 적용가능한 PCB버전 목록
  uint32_t pcb[4];//52
}section_info_t;




typedef enum {
#define X(code, name) code,
  SECTION_LIST
#undef X
  SECTION_COUNT
} section_code_e;

extern const char* product_name_list[];
extern const char* alias_name_list[];
extern const char* section_name_list[];


#endif