

const osThreadAttr_t kLoggingTask_attributes = {
  .name = "loggingTask",
  .stack_size = TASK_STACK(TASK_LOGGING_DEF),
  .priority = (osPriority_t)TASK_PRIO(TASK_LOGGING_DEF),
};

void vhfTask_init(void)
{


  osThreadNew(loggingTask, NULL, &kLoggingTask_attributes);

}