
#ifndef DRIVER_DIGITALIN_H
#define DRIVER_DIGITALIN_H





#include <stdint.h>

#include "cmsis_os.h"
#include "driver_interface.h"



#define DI_ADC_RDY    0
#define DI_RTC_IRQ    1


//¸í·É¾î
typedef enum intType_e
{
  INT_FALLING,
  INT_RISING
}eINT_TYPE_t;

typedef struct digitalIn_set_interupt_s
{
    eINT_TYPE_t intType;//falling,rising
    uint16_t num;
    uint16_t priority;
    void (*call)(void *);
    void *handle;
}digitalIn_set_interupt_t;

typedef enum digitalSetInt_e
{
  eDIG_IN_SET_INTERRUPT
}eDIGITAL_IN_SET_CMD_t;


typedef struct driver_digitalIn_s
{
    const char *name;
    uint8_t err;
    osSemaphoreId mutex;
    uint16_t num;
    void *api;
    void *apiCfg;
}driver_digitalIn_t;

void driver_digitalIn_init(driver_digitalIn_t *digitalOut,uint32_t num);
uint32_t driver_digitalIn_read(driver_digitalIn_t *digitalOut);
void driver_digitalIn_set(driver_digitalIn_t *digitalIn,uint32_t cmd,void *data);


driver_t *driver_di_open(int num);
uint16_t driver_di_read(driver_t *driver);
#endif