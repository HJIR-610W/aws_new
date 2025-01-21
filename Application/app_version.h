
#ifndef APP_VERSION_H
#define APP_VERSION_H
/**
app 파일들에 공통적으로 적용되는 define
*/




#include <stdint.h>


#include "driver_rtc.h"

#include "hj_product_list.h"


#define MAKE_FW_VERSION(major, minor, bugfix,rel) (((major) << 24) | ((minor) << 16) | (bugfix<<9) |(rel))


#define APP_VERSION (MAKE_FW_VERSION(0, 1, 0,0))







#define MODEM_BG96_USE 1

/*버전
1.x.x.x  호환 여부
x.1.x.x  기능 추가
x.x.1.x  버그 수정
x.x.x.1  개발 버전
*/


uint32_t get_appVer(uint8_t *a,uint8_t *b,uint8_t *c,uint8_t *d);
void get_appBuild(DATE_TIME_BUF *build);
uint32_t get_appNick(void);
void get_nickCode(uint32_t *nickCode);
void get_hwCode(uint32_t *nickCode);
void set_testKey(uint32_t key); 

#endif