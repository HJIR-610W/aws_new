#include "task_system.h"

#include "bsp.h"

#include "app_charger.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "driver_di.h"
#include "driver_do.h"
#include "driver_uart.h"
#include "task_isrEvent.h"
#include "os_user_def.h"
#include "system_err.h"
#include "fatfs.h"

const osThreadAttr_t kSystemTask_attributes = {
    .name = "systemTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityBelowNormal,
};

void userBtnCallBack(void *arg)
{ 
  os_send_isrEvent(eUSER_BTN_INT, 0); 
}

void userBtn_init(void)
{
  driver_t *user_btn;
  di_isr_set_cfg_t isr_cfg;

  user_btn = driver_di_open(DI_USER_BTN, 0);

  isr_cfg.call = userBtnCallBack;
  isr_cfg.name = "user_btn";
  isr_cfg.trigger = eDI_FALLING;
  isr_cfg.prio = 5;

  driver_di_set(user_btn, DI_SET_INTERRUPT, &isr_cfg);
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
      ERROR_PRINTF("SD카드 삽입됨\r\n");
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
      ERROR_PRINTF("SD카드 제거됨\r\n");
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

void systemTask(void *arg)
{
  uint8_t err=0;
  uint32_t start_time = osKernelGetTickCount();

  pre_sd_inserted = BSP_PlatformIsDetected();

  while (1)
  {
    bsp_rtc_update();

    if ((osKernelGetTickCount() - start_time)>1000)
    {

      start_time = osKernelGetTickCount();
      System.door_opened = bsp_door_opened();
      update_charger();
      System.battery_error = read_batteryVoltage1(&err) < 10.0f?1:0;
      System.ac_status = 1;//220v
      System.dc_error = bsp_read_battery()<11.0f?1:0;

      check_sd_card();
    }

    osDelay(500);
  }
}




void systemTask_init(uint32_t para)
{
  if(para==PARA_RUN_MODE)
  {
    userBtn_init();
    
    charger_init(get_config_app()->charger_model);

  }

  osThreadNew(systemTask, NULL, &kSystemTask_attributes);
}