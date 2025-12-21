
#ifndef CONFIG_MIGRATION_H
#define CONFIG_MIGRATION_H

#include <stdint.h>
/*
버전별 설정 마이그레이션 처리
*/
void config_app_migration(uint32_t current_version,uint32_t target_version,void *old_config);
#endif