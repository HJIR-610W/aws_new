#ifndef TASK_SYSTEM_H
#define TASK_SYSTEM_H

#include <stdint.h>

#define PARA_RUN_MODE 0
#define PARA_TEST_MODE 1
void systemTask_init(uint32_t para);
int is_door_opened(void);
#endif