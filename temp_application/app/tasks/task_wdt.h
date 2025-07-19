

#ifndef TASK_WDT_H
#define TASK_WDT_H

#include <stdint.h>


void wdt_task_feed(int index);
int wdt_task_register(const char *name, uint32_t timeout_ms);
void wdtTask_init(void);
void wdt_task_unregister(int index);

#endif