#include "menu_panel.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "common\const_string.h"
#include "menu_handler.h"
#include "Sensors\snow\hj_snow.h"
#include "util_memory.h"
#include "view_driver.h"


#define SCREEN_COLS 20
#define PANEL_WD 8




#define HJ_SNOW_CONFIG_VIEW    0
#define HJ_SNOW_ZERO_CALIB     1
#define HJ_SNOW_SET_DISTANCE   2
#define HJ_SNOW_VIEW_DATA      3


driver_t *hjsnow;

void draw_ctrl_hj_snow_menu(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, HJ_SNOW_CONFIG_VIEW, "View config");
  screen_menu_printf(p_win, HJ_SNOW_ZERO_CALIB, "Zero calibration");
  screen_menu_printf(p_win, HJ_SNOW_SET_DISTANCE, "Setting height");
  screen_menu_printf(p_win, HJ_SNOW_VIEW_DATA, "View data");
  screen_menu_clear(p_win);
}

void draw_snow_config_page(screen_page_t *p_win, hjsnow_read_config_t *cfg)
{
  screen_page_start(p_win);
  screen_page_printf(p_win, "model[0] :%s", cfg->config.xModel[0]);
  screen_page_printf(p_win, "model[1] :%s", cfg->config.xModel[1]);
  screen_page_printf(p_win, "model[2] :%s", cfg->config.xModel[2]);
  screen_page_printf(p_win, "distance :%dmm", cfg->config.snow_stddistance);
  screen_page_printf(p_win, "scantime :%d", cfg->config.snow_scantime);
  screen_page_printf(p_win, "ref d[0] :%d", cfg->config.snow_refdistance[0]);
  screen_page_printf(p_win, "ref d[1] :%d", cfg->config.snow_refdistance[1]);
  screen_page_printf(p_win, "ref d[2] :%d", cfg->config.snow_refdistance[2]);
  screen_page_printf(p_win, "flevel   :%d", cfg->config.snow_filterlevel);
  screen_page_printf(p_win, "filter   :%d", cfg->config.snow_nofiltermode);
  screen_page_printf(p_win, "scan auto:%d", cfg->config.snow_scantempauto);
  screen_page_clear(p_win);

}
int32_t read_snow_config(void)
{
  int32_t status;
  int32_t key;
  screen_page_t lcd_win;
  hjsnow_read_config_t cfg;
  uint8_t err;

  hjsnow_ctrl(hjsnow, eHJSNOW_GET_CONFIG, NULL, &cfg, &err);

  if(err)
  {
    show_popup("Error", "Command failed");
    return MENU_BACK;
  }
  screen_page_create(&lcd_win);
  lcd_win.total_pages = 1;
  lcd_win.chunk_scroll_enable = 1;

  do
  {
    draw_snow_config_page(&lcd_win,&cfg);
    screen_refresh();
    key = get_button_key(1000);

    if (key == KEY_CODE_CTRL_Q)
    {
      status = MENU_ABORT;
      break;
    }
    else if (key == KEY_CODE_CTRL_C)
    {
      status = MENU_BACK;
      break;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_page_handle(&lcd_win, key);
    }
  } while (1);

  return status;
}

int32_t ctrl_hj_snow(void)
{

  int32_t index;
  int32_t key;

  screen_menu_t menu;

  uint8_t err;

  hjsnow = hjsnow_opened();
  if (hjsnow == NULL)
  {
    show_popup("Error", "HJ Snow sensor not configured");
    return MENU_BACK;
  }

  screen_menu_create(&menu, "HJ SNOW");

  while (1)
  {
    draw_ctrl_hj_snow_menu(&menu);
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
      case HJ_SNOW_CONFIG_VIEW:
        {
          read_snow_config();
        }
      break;

      case HJ_SNOW_ZERO_CALIB:
      {
        int ok = 0;
        input_active("Run zero calibration?", &ok);
        if (ok)
        {
          hjsnow_ctrl(hjsnow, eHJSNOW_RUN_ZERO, NULL, NULL, &err);
          if (err)
          {
            show_popup("Error", "Command transmission failed");
          }
          else
          {
            show_popup("Success", "Command sent\r\nCheck laser pointer");
          }
        }
      }
      break;
      case HJ_SNOW_SET_DISTANCE:
      {
        int status;
        int height;
  

        status = input_decimal("Height(mm)",0,3000,&height);
        if(status == MENU_OK)
        {
          hjsnow_write_height(height,&err);
          if(err == 0)
          {
            show_popup("Setting height", "Success");
          }
        }

      }
      
      break; 
      case HJ_SNOW_VIEW_DATA:
      {
        hjsnow_read_system_t system;
        hjsnow_ctrl(hjsnow, eHJSNOW_GET_SYSTEM, NULL, &system, &err);

        if (err)
        {
          char buff[50];
          snprintf(buff, sizeof(buff), "Sensor error: %s", get_drv_err_name(err));
          show_popup("Error", buff);
        }
        else
        {
          char buff[150];
          int len = 0;

          len = make_sreen_row(&buff[len], "S1 dist:%d mm", system.system.CurDistance[0]);
          len += make_sreen_row(&buff[len],"S2 dist:%d mm", system.system.CurDistance[1]);
          len += make_sreen_row(&buff[len],"S3 dist:%d mm", system.system.CurDistance[2]);
          len += make_sreen_row(&buff[len],"Temperature:%d", system.system.innerTemp);
          len += make_sreen_row(&buff[len],"Snow Level:%d mm", system.system.CurSnowLevel);

          show_popup("HJ Snow Data", buff);
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