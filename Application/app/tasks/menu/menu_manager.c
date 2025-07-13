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
#define CONFIG_MENU_LOG_RESET   3

#define BACKUP_MENU_SAVE        0
#define BACKUP_MENU_RESTORE     1

void draw_setup_menu_manager_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, MANAGER_MENU_VERSION);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "VERSION");

  screen_update_list(p_win, row_count, MANAGER_MENU_RESET);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "RESET");

  screen_update_list(p_win, row_count, MANAGER_MENU_CONFIG);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "CONFIG");

  screen_update_list(p_win, row_count, MANAGER_MENU_UPDATE);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "UPDATE");

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
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "HJ RESET");

  screen_update_list(p_win, row_count, CONFIG_MENU_INIT);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "INIT");

  screen_update_list(p_win, row_count, CONFIG_MENU_BACKUP);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "BACKUP");

  screen_update_list(p_win, row_count, CONFIG_MENU_LOG_RESET);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "LOG RST");

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
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "SAVE");

  screen_update_list(p_win, row_count, BACKUP_MENU_RESTORE);
  MENU_PRINTF(p_win, row_count++, "%-*s", MANAGER_WD, "RESTORE");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

int32_t setup_menu_version(void)
{
  const char* confirm_menu[] = {"OK"};
  uint8_t fix;
  uint8_t major;
  uint8_t minor;
  uint8_t rel;
  int32_t choice = 0;
  int32_t status;
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

  status = print_menu_list(confirm_menu, 1, &choice);

  return status;
}

int32_t setup_menu_reset(void)
{
  const char* confirm_menu[] = {"No", "Yes"};
  int32_t choice = 0;
  int32_t status;

  screen_clear();
  screen_printf(0, 0, "Device Reset?");
  screen_refresh();

  status = print_menu_list(confirm_menu, 2, &choice);

  if (status == MENU_OK && choice == 1)
  {
    reset_system("user reset");
  }

  return status;
}

int32_t setup_menu_update(void)
{
  const char* confirm_menu[] = {"No", "Yes"};
  int32_t choice = 0;
  int32_t status;

  screen_clear();
  screen_printf(0, 0, "Firmware Update?");
  screen_refresh();

  status = print_menu_list(confirm_menu, 2, &choice);

  if (status == MENU_OK && choice == 1)
  {
    if (check_firmware(UPDATE_LOCAL) == 0)
    {
      screen_clear();
      screen_printf(0, 0, "Updating...");
      screen_refresh();

      set_magic_value(MAGIC_UPDATE_FW_LACAL);
      reset_system("USER update");
    }
  }

  return status;
}


int32_t setup_menu_hj_reset(void)
{
  const char* confirm_menu[] = {"No", "Yes"};
  int32_t choice = 0;
  int32_t status;

  screen_clear();
  screen_printf(0, 0, "HJ Config Reset?");
  screen_refresh();

  status = print_menu_list(confirm_menu, 2, &choice);

  if (status == MENU_OK && choice == 1)
  {
    config_hj_reset();
    
    screen_clear();
    screen_printf(0, 0, "Reset Complete");
    screen_refresh();
    
    const char* ok_menu[] = {"OK"};
    choice = 0;
    print_menu_list(ok_menu, 1, &choice);
  }

  return status;
}

int32_t setup_menu_init(void)
{
  const char* confirm_menu[] = {"No", "Yes"};
  int32_t choice = 0;
  int32_t status;

  screen_clear();
  screen_printf(0, 0, "Config Init?");
  screen_refresh();

  status = print_menu_list(confirm_menu, 2, &choice);

  if (status == MENU_OK && choice == 1)
  {
    config_app_reset();
    save_config_app();
    config_sensor_reset();
    save_config_sensor();
    
    screen_clear();
    screen_printf(0, 0, "Init Complete");
    screen_refresh();
    
    const char* ok_menu[] = {"OK"};
    choice = 0;
    print_menu_list(ok_menu, 1, &choice);
  }

  return status;
}

int32_t setup_menu_backup(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

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
          
          screen_clear();
          screen_printf(0, 0, "Backup Complete");
          screen_refresh();
          
          const char* ok_menu[] = {"OK"};
          int32_t choice = 0;
          print_menu_list(ok_menu, 1, &choice);
        }
        break;

        case BACKUP_MENU_RESTORE:
        {
          const char* confirm_menu[] = {"No", "Yes"};
          int32_t choice = 0;
          
          screen_clear();
          screen_printf(0, 0, "Restore Config?");
          screen_refresh();
          
          status = print_menu_list(confirm_menu, 2, &choice);
          
          if (status == MENU_OK && choice == 1)
          {
            restore_config();
            
            screen_clear();
            screen_printf(0, 0, "Restore Complete");
            screen_refresh();
            
            const char* ok_menu[] = {"OK"};
            choice = 0;
            print_menu_list(ok_menu, 1, &choice);
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

  screen_clear();
  screen_printf(0, 0, "Log Cnt:%d", get_config_nvm()->log_q_cnt);
  screen_printf(1, 0, "New Count:");
  screen_refresh();

  status = input_decimal("Log Count", 0, LOG_COUNT_MAX, &log_cnt);
  
  if (status == MENU_OK)
  {
    nvm_set_log_cnt(log_cnt);
    
    screen_clear();
    screen_printf(0, 0, "Log Reset");
    screen_printf(1, 0, "Count:%d", log_cnt);
    screen_refresh();
    
    const char* ok_menu[] = {"OK"};
    int32_t choice = 0;
    print_menu_list(ok_menu, 1, &choice);
  }

  return status;
}

int32_t setup_menu_config(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

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
          status = setup_menu_hj_reset();
          break;

        case CONFIG_MENU_INIT:
          status = setup_menu_init();
          break;

        case CONFIG_MENU_BACKUP:
          status = setup_menu_backup();
          break;

        case CONFIG_MENU_LOG_RESET:
          status = setup_menu_log_reset();
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

int32_t setup_menu_manager(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

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
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}