

#include "bsp.h"
#include "cmsis_os2.h"
#include "task_start.h"


int main(void)
{

  bsp_init();
 
  osKernelInitialize();

  startTask_init();

  osKernelStart();

  while(1);
  
}



