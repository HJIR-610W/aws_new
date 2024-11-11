

#ifndef  TASK_CMD_H
#define TASK_CMD_H
#include <stdint.h>
typedef struct cmd_s
{
  void *task;
  void *source;//rs232
  uint8_t cmd;
  uint8_t data[512];
  uint16_t len;
}cmd_t;

void cmdTask_init(void);

void put_cmd(cmd_t *cmd);

#endif