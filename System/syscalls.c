

#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

void __exit (int status)
{

  vTaskSuspendAll();
  __disable_irq();
	while (1)
  {
    
  }
}