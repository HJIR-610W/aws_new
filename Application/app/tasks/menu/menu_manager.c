#include "menu_manager.h"

#include "app_key.h"
#include "app_screen.h"
#include "app_version.h"
#include "boot_version.h"
#include "bsp_rtc.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "config_manager.h"
#include "config_nvm.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "system_err.h"
#include "update_fw.h"
#include "util_memory.h"
#include "util_time.h"
#include "view_driver.h"
#include "app_logging.h"
#include "aws_data_handler.h"

extern void config_hj_reset(void);

#define SCREEN_COLS 20
#define MANAGER_WD 8

#define MENU_PRINTF screen_menu_printf_row

#define MANAGER_MENU_VERSION    0
#define MANAGER_MENU_RESET      1
#define MANAGER_MENU_CONFIG     2
#define MANAGER_MENU_UPDATE     3

#define CONFIG_MENU_HJ_RESET    0
#define CONFIG_MENU_INIT        1
#define CONFIG_MENU_BACKUP      2
#define CONFIG_RAIN_INIT        3
#define CONFIG_MENU_LOG_RESET   4

#define BACKUP_MENU_SAVE        0
#define BACKUP_MENU_RESTORE     1

void draw_setup_menu_manager_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, MANAGER_MENU_VERSION);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Version");

  screen_update_list(p_win, row_count, MANAGER_MENU_RESET);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Device Reset");

  screen_update_list(p_win, row_count, MANAGER_MENU_CONFIG);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Settings Change");

  screen_update_list(p_win, row_count, MANAGER_MENU_UPDATE);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Firmware update");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

void draw_setup_menu_config_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, CONFIG_MENU_HJ_RESET);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "HJ Reset");

  screen_update_list(p_win, row_count, CONFIG_MENU_INIT);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Factory Reset");

  screen_update_list(p_win, row_count, CONFIG_MENU_BACKUP);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Backup Config");

  screen_update_list(p_win, row_count, CONFIG_RAIN_INIT);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Rain,Sun Reset");

  screen_update_list(p_win, row_count, CONFIG_MENU_LOG_RESET);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Log Count Reset");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

void draw_setup_menu_backup_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, BACKUP_MENU_SAVE);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Save");

  screen_update_list(p_win, row_count, BACKUP_MENU_RESTORE);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Restore");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

#define DATA_RESET_MENU_RAIN 0
#define DATA_RESET_MENU_SUN  1

void draw_rain_reset_page(screen_menu_t *p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, DATA_RESET_MENU_RAIN);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Reset Rain Data to 0");

  screen_update_list(p_win, row_count, DATA_RESET_MENU_SUN);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "Reset Sun Data to 0");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}
int32_t setup_menu_version(void)
{

  uint8_t fix;
  uint8_t major;
  uint8_t minor;
  uint8_t rel;


  char buff[30];
  DATE_TIME_BUF ct;

  screen_clear();

  get_app_version(&major, &minor, &fix, &rel);
  get_app_build(&ct);
  make_timeToStr(&ct, buff, sizeof(buff));

  screen_printf(0, 0, "App:%d.%d.%d.%d", major, minor, fix, rel);
  screen_printf(1, 0, "Build:%s", buff);

  get_boot_version(&major, &minor, &fix, &rel);
  get_boot_build(&ct);
  make_timeToStr(&ct, buff, sizeof(buff));

  screen_printf(2, 0, "Boot:%d.%d.%d.%d", major, minor, fix, rel);
  screen_printf(3, 0, "Build:%s", buff);

  screen_refresh();

  get_button_key(0xFFFFFFFF);

  return MENU_OK;
}

int32_t setup_menu_reset(void)
{
  int32_t choice = 0;
  int32_t status;

  screen_clear();
  screen_printf(0, 0, "Device Reset?");
  screen_refresh();

  
  status = input_active("Reset device?",  &choice);

  if (status == MENU_OK && choice == 1)
  {
    reset_system("user reset");
  }

  return status;
}

int32_t setup_menu_update(void)
{
  int32_t choice = 0;
  int32_t status;

  status = input_active("Firmware Update?",&choice);

  if (status == MENU_OK && choice == 1)
  {
    if (check_firmware(UPDATE_LOCAL) == 0)
    {
      screen_clear();
      screen_printf(0, 0, "Updating...");
      screen_refresh();
      osDelay(2000);
      set_magic_value(MAGIC_UPDATE_FW_LACAL);
      reset_system("User Update");
    }
  }

  return status;
}


int32_t setup_menu_hj_reset(void)
{
  int32_t choice = 0;
  int32_t status;

  status = input_active("HJ Config Reset?", &choice);

  if (status == MENU_OK && choice == 1)
  {
    config_hj_reset();
    show_popup("Information", "Reset Complete");
  }

  return status;
}

int32_t setup_menu_init(void)
{
  int32_t choice = 0;
  int32_t status;

  status = input_active("Config Init",&choice);

  if (status == MENU_OK && choice == 1)
  {
    config_app_reset();
    save_config_app();
    config_sensor_reset();
    save_config_sensor();

    show_popup("Information", "Init Complete");
  }

  return status;
}

int32_t setup_menu_backup(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu,"Backup");

  while (1)
  {
    draw_setup_menu_backup_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case BACKUP_MENU_SAVE:
        {
          backup_config();
          show_popup("Information", "Backup Complete");
        }
        break;

        case BACKUP_MENU_RESTORE:
        {
          int32_t choice = 0;
          status = input_active("Restore Config?", &choice);

          if (status == MENU_OK && choice == 1)
          {
            restore_config();
            show_popup("Information", "Restore Complete");
          }
        }
        break;

        default:
          break;
      }
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t setup_menu_rain_reset(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, "Rain Sun Reset");

  while (1)
  {
    draw_rain_reset_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
      case DATA_RESET_MENU_RAIN:
      {
        int32_t choice = 0;
        status = input_active("Initialize all to 0?", &choice);
        if (status == MENU_OK && choice == 1)
        {
           if(rain_file_zero(Date_Time.Year)==0)
             show_popup("Information", "Completed");
           else{
             show_popup("Information", "Failed to complete");
           }
        }
      }
      break;

      case DATA_RESET_MENU_SUN:
      {
        int32_t choice = 0;
        status = input_active("Initialize all to 0?", &choice);

        if (status == MENU_OK && choice == 1)
        {
          if(sunshine_file_zero(Date_Time.Year)==0)
            show_popup("Information", "Completed");
          else
            show_popup("Information", "Failed to complete");
        }
      }
      break;

      default:
        break;
      }
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t setup_menu_log_reset(void)
{
  int32_t log_cnt;
  int32_t status;


  log_cnt = get_config_nvm()->log_q_cnt;

  status = input_decimal("Log Count", 0, 2147483647, &log_cnt);
  
  if (status == MENU_OK)
  {
    nvm_set_log_cnt(log_cnt);

    show_popup("Information", "log count:0");
  }

  return status;
}
//설정변경
int32_t setup_menu_config(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, "Manager");

  while (1)
  {
    draw_setup_menu_config_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case CONFIG_MENU_HJ_RESET:
          status = setup_menu_hj_reset();//1.AWS 화진 기본 설정
          break;

        case CONFIG_MENU_INIT:
          status = setup_menu_init();//2.공장 초기화
          break;

        case CONFIG_MENU_BACKUP:
          status = setup_menu_backup();//설정 백업
          break;

        case CONFIG_RAIN_INIT:
          status = setup_menu_rain_reset();
          break;

        case CONFIG_MENU_LOG_RESET:
          status = setup_menu_log_reset();
          break;

        default:
          break;
      }
      if(status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t setup_menu_manager(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu,  "MANAGER");

  while (1)
  {
    draw_setup_menu_manager_page(&menu);
    screen_refresh();

    key = get_button_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case MANAGER_MENU_VERSION:
          status = setup_menu_version();
          break;

        case MANAGER_MENU_RESET:
          status = setup_menu_reset();
          break;

        case MANAGER_MENU_CONFIG:
          status = setup_menu_config();
          break;

        case MANAGER_MENU_UPDATE:
          status = setup_menu_update();
          break;

        default:
          break;
      }
            if(status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}