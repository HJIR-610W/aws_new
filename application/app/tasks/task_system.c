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
#include "task_logging.h"

const osThreadAttr_t kSystemTask_attributes = {
    .name = "systemTask",
    .stack_size = TASK_STACK(TASK_SYSTEM_DEF),
    .priority = (osPriority_t)TASK_PRIO(TASK_SYSTEM_DEF),
};

void userBtnCallBack(int32_t arg)
{
  isr_event_cmd_t event;

  event.cmd = eUSER_BTN_INT;
  os_send_event(&event, 0);


}

void user_button_init(void)
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

static uint8_t pre_sd_inserted = SD_NOT_PRESENT;

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
      log_printf(L_ERROR,"SD card inserted");
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
      log_printf(L_ERROR,"SD card removed");
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
int32_t charger_model = eCHARGER_NONE;

void systemTask(void *arg)
{
  uint8_t err=0;
  uint32_t start_time = osKernelGetTickCount();

  DEBUG_PRINTF("system task start\r\n");

  pre_sd_inserted = BSP_PlatformIsDetected();


  while (1)
  {
    drv_rtc_read(&Date_Time);

    if ((osKernelGetTickCount() - start_time) > 1000)
    {

      if( arg==PARA_RUN_MODE)
      {
          start_time = osKernelGetTickCount();

          if (charger_model != eCHARGER_NONE)
          {
            update_charger(charger_model);
            System.battery_error = read_batteryVoltage1(&err) < 10.0f ? 1 : 0;
            System.charger_battery1_voltage = read_batteryVoltage1(&err);
            System.charger_battery2_voltage = read_batteryVoltage2(&err);
            System.charger_solar1_currnet = read_solarCurrent1(&err);
            System.charger_solar2_currnet = read_solarCurrent2(&err);
            System.charger_load1_currnet = read_loadCurrent1(&err);
            System.charger_load2_currnet = read_loadCurrent2(&err);
            System.charger_solar1_voltage = read_solarVoltage1(&err);
            System.charger_solar2_voltage = read_solarVoltage2(&err);
          }

          System.door_opened = drv_di_read(DRV_DI_0) > 0;
          System.battery_voltage = drv_system_read(DRV_SYS_BATTERY);
          System.dc_error = System.battery_voltage < 11.0f ? 1 : 0;
          System.ac_status = drv_di_read(DRV_DI_1) > 0 ? eAC_OFF:eAC_220V;
          System.sdcard_inserted = BSP_PlatformIsDetected();
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

    user_button_init();

    switch (get_config_app()->charger_model)
    {
    case eCHARGER_SMART:
      charger_model = DEV_CHARGER_HJ_SMART;
        dev_charger_init(DEV_CHARGER_HJ_SMART);
      break;
    case eCHARGER_LS:
      charger_model = DEV_CHARGER_LS1024;
      dev_charger_init(DEV_CHARGER_LS1024);
      break;

        default:
      break;
    }
  }

  osThreadNew(systemTask, (void *)para, &kSystemTask_attributes);
}