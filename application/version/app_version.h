
#ifndef APP_VERSION_H
#define APP_VERSION_H


#include <stdint.h>
#include "util_time.h"
#include "product.h"

#define MAKE_FW_VERSION(major, minor, bugfix,rel) (((major) << 24) | ((minor) << 16) | (bugfix<<8) |(rel))

#define APP_VERSION (MAKE_FW_VERSION(0, 34, 0, 0))
#define PCB_VERSION (MAKE_FW_VERSION(1, 0, 0, 0))


/*버전
1.x.x.x  호환 여부
x.1.x.x  기능 추가
x.x.1.x  버그 수정
x.x.x.1  개발 버전
*/

uint32_t get_app_version(uint8_t *major,uint8_t *minor,uint8_t *patch,uint8_t *release);
void get_app_build(DATE_TIME_BUF *build);
uint32_t get_app_alias(void);
const char *get_alias_name(void);
uint32_t get_product_code(void);
uint32_t get_app_area_code(void);
uint32_t get_app_build_timestamp(void);
#endif