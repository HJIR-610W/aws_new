#include "task_system.h"

#include "bsp.h"

#include "app_charger.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "drv_di.h"
#include "drv_do.h"
#include "drv_rs232.h"
#include "task_isrEvent.h"
#include "os_user_def.h"
#include "system_err.h"
#include "fatfs.h"
#include "app_key.h"
#include "drv_rtc.h"
#include "drv_system.h"
#include "dev_charger.h"

const osThreadAttr_t kSystemTask_attributes = {
    .name = "systemTask",
    .stack_size = TASK_SYSTEM_STACK_SIZE,
    .priority = (osPriority_t)osPriorityBelowNormal ,
};

void userBtnCallBack(int32_t arg)
{ 
  os_send_isrEvent(eUSER_BTN_INT, 0); 
}

void userBtn_init(void)
{
  di_isr_set_cfg_t isr_cfg;
  isr_cfg.call = userBtnCallBack;
  isr_cfg.name = "user_btn";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;

  drv_di_set_interrupt(DRV_DI_USER_BTN, &isr_cfg);
}

extern uint8_t g_sd_diskio_error;
extern void *get_file_sem(void);
extern uint8_t BSP_PlatformIsDetected(void);
extern void hal_sd_init(void);
extern void hal_sd_deinit(void);

static uint8_t pre_sd_inserted;

void check_sd_card(void)
{
  uint8_t now_sd_inserted;

  now_sd_inserted = BSP_PlatformIsDetected();

  // SD 카드 상태가 변경된 경우
  if (now_sd_inserted != pre_sd_inserted)
  {
    pre_sd_inserted = now_sd_inserted;  // 상태 갱신

    if (now_sd_inserted)
    {
      ERROR_PRINTF("SD card inserted\r\n");
      OS_PEND_SEM(get_file_sem(), osWaitForever);
      MX_FATFS_DeInit();
      hal_sd_deinit();
      osDelay(100);
      hal_sd_init();
      MX_FATFS_Init();
      g_sd_diskio_error=0;
      OS_POST_SEM(get_file_sem());
    }
    else
    {
      ERROR_PRINTF("SD card removed\r\n");
      OS_PEND_SEM(get_file_sem(), osWaitForever);
      MX_FATFS_DeInit();
      hal_sd_deinit();
      g_sd_diskio_error = 1;  // 제거 시에도 에러 상태로 전환
      OS_POST_SEM(get_file_sem());
    }
  }

  // 카드가 삽입된 상태에서 오류가 감지된 경우
  if (now_sd_inserted && g_sd_diskio_error)
  {
    ERROR_PRINTF("g_sd_diskio_error %d\r\n", g_sd_diskio_error);
    OS_PEND_SEM(get_file_sem(), osWaitForever);
    MX_FATFS_DeInit();
    hal_sd_deinit();
    osDelay(100);
    hal_sd_init();
    MX_FATFS_Init();

    g_sd_diskio_error = 0;
    OS_POST_SEM(get_file_sem());
  }
}

int is_door_opened(void)
{
  if(drv_di_read(DRV_DI_0))
  {
    return 1;
  }

  return 0;
}

void systemTask(void *arg)
{
  uint8_t err=0;
  uint32_t start_time = osKernelGetTickCount();
  eCHARGER_MODEL_t charger_model;

  pre_sd_inserted = BSP_PlatformIsDetected();

  charger_model = get_config_app()->charger_model;

  while (1)
  {
    scan_key();
    drv_rtc_read(&Date_Time);

    if ((osKernelGetTickCount() - start_time) > 1000)
    {

      if( arg==PARA_RUN_MODE)
      {
          start_time = osKernelGetTickCount();
          System.door_opened = drv_di_read(DRV_DI_0) > 0;
          update_charger(charger_model);
          System.battery_error = read_batteryVoltage1(&err) < 10.0f ? 1 : 0;
          System.ac_status = 1; // 220v
          System.dc_error = drv_system_read(DRV_SYS_BATTERY) < 11.0f ? 1 : 0;
          check_sd_card();
       }
    }
    osDelay(100);
  }
}



 
void systemTask_init(uint32_t para)
{

  if(para==PARA_RUN_MODE)
  {
    
    app_key_init();
    userBtn_init();
    dev_charger_init(get_config_app()->charger_model);
  }

  osThreadNew(systemTask, (void *)para, &kSystemTask_attributes);
}