#ifndef AT_PARSER_H
#define AT_PARSER_H

#include <stddef.h>

typedef void (*at_parser_task_fn_t)(void *arg);


int at_parser_start(void);
void at_parser_stop(void);


void at_parser_set_ring_task(at_parser_task_fn_t fn);
void at_parser_set_sms_task(at_parser_task_fn_t fn);

#endif
