
#include "driver_i2c.h"
#include "stm32f4xx_hal.h"
#include "driver_stm32_i2c.h"
#include "cmsis_os2.h"
#include "main.h"




driver_t driver_i2c[2];

driver_t *driver_i2c_open(uint32_t num,void *opt)
{
    switch(num)
    {
    case DRIVER_STM32_I2C_1:
    
    break;
    case DRIVER_STM32_I2C_2:
    
    break;
    }
}