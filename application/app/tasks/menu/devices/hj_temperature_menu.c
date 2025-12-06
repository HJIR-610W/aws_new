#include "menu_panel.h"

#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "common\const_string.h"
#include "menu_handler.h"
#include "Sensors\temperature\hj_temperature.h"
#include "Sensors\humidity\hj_huminity.h"
#include "util_memory.h"
#include "view_driver.h"


#define SCREEN_COLS 20
#define PANEL_WD 8



#define HJ_TEMP_INFO           0
#define HJ_TEMP_SETTING_OFFSET 1
#define HJ_HUMI_SETTING_OFFSET 2
#define HJ_TEMP_VEIW_DATA      3

void draw_ctrl_hj_temp_page(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, HJ_TEMP_INFO, "Information");
  screen_menu_printf(p_win, HJ_TEMP_SETTING_OFFSET, "Setting temp offset");
  screen_menu_printf(p_win, HJ_HUMI_SETTING_OFFSET, "Setting humi offset");
  screen_menu_printf(p_win, HJ_TEMP_VEIW_DATA, "View data");
  screen_menu_clear(p_win);
}

int32_t ctrl_hj_temp(void)
{

  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, "HJ temperature");

  while (1)
  {
    draw_ctrl_hj_temp_page(&menu);
    screen_refresh();

    key = get_menu_key(1000);

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
      case HJ_TEMP_INFO:
      {
          modbus_h_t *modbus_h;
          hjtemp_register_map_t map;
          char buff[50];


          modbus_h = get_hjtemperature_bus_io();

          modbus_read_hold_reg(modbus_h, 0, (uint16_t *)&map, 16);
           snprintf(buff, sizeof(buff), "SW Version:%3d       HW Version:%3d", map.sw_version, map.hw_version);
            show_popup("HJ temperature",buff);

        }
        break;
      case HJ_TEMP_SETTING_OFFSET:
        {
          int ok;
          uint16_t offset;
          int32_t ret;
          char buff[40];

          ret = hjtemp_read_temp_offset(&offset);

          if (ret == 0)
          {
            snprintf(buff, sizeof(buff), "offset:%.2f,Change?\r\n", ((float)offset / 100.0f));
            ok = 0;
            input_active(buff, &ok);
            if (ok)
            {
              float offset_val = (float)offset/100.0f;
              status = input_float("Offset", -5.0f, 5.0f, &offset_val,"%3.1f");
              if(status == MENU_OK)
              {
                hjtemp_write_temp_offset((uint16_t)(offset_val*100));
              }
            }
       
          }
        }
      break;

      case HJ_HUMI_SETTING_OFFSET:
      {
        int ok;


        uint16_t offset;
        int32_t ret;
        char buff[40];

        ret = hjtemp_read_humi_offset(&offset);

        if (ret == 0)
        {
          snprintf(buff, sizeof(buff), "offset:%.2f,Change?\r\n", ((float)offset / 100.0f));
          ok = 0;
          input_active(buff, &ok);
          if(ok)
          {
            float offset_val = (float)offset / 100.0f;
            status = input_float("Offset", -5.0f, 5.0f, &offset_val, "%3.1f");
            if (status == MENU_OK)
            {
              hjtemp_write_humi_offset((uint16_t)(offset_val * 100));
            }
          }
        }
      }
      break;
      case HJ_TEMP_VEIW_DATA:
      {
        uint8_t err;
        float temp;
        int len=0;
        
        driver_t *hjtemp, *hjhumi;
        char buff[40];
        hjtemp = hjtemp_opened();
        if(hjtemp)
        {
        temp = hjTemperature_read(hjtemp, &err);
          len = snprintf(buff,sizeof(buff),"temp:%5.2f           ",temp);
        }
        hjhumi = hjHumi_opened();
        float humi;
        if(hjhumi)
        {
        humi = hjHuminity_read(hjhumi, &err);
          snprintf(&buff[len],sizeof(buff)-len,"humi:%.2f",humi);

        }
        
 
        
        show_popup("HJ temperature", buff);


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