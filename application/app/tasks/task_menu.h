
#ifndef TASK_MENU_H
#define TASK_MENU_H

typedef struct alert_s
{
  const char *title;
  char framebuffer[8][21];
} alert_t;

void menuTask_init(void);
void test_menu_info(void);
void show_alert(alert_t alert);
#endif