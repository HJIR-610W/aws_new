#include "hj_wind_speed.h"


#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "common\const_string.h"
#include "menu_handler.h"
#include "Sensors\temperature\hj_temperature.h"
#include "Sensors\humidity\hj_huminity.h"
#include "Sensors\wind_speed\hj_wind_speed_modbus.h"

#include "util_memory.h"
#include "view_driver.h"


#define SCREEN_COLS 20
#define PANEL_WD 8



#define HJ_WIND_SPEED_INFO            0
#define HJ_WIND_SPEED_SETTING_ID      1
#define HJ_WIND_SPEED_SETTING_FULLSET 2
#define HJ_WIND_SPEED_SETTING_OFFSET  3
#define HJ_WIND_SPEED_SATUS           4
void draw_ctrl_hj_wind_speed_page(screen_menu_t *p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, HJ_WIND_SPEED_INFO, "Information");
  screen_menu_printf(p_win, HJ_WIND_SPEED_SETTING_ID, "Setting id");
  screen_menu_printf(p_win, HJ_WIND_SPEED_SETTING_FULLSET, "Setting fullset");
  screen_menu_printf(p_win, HJ_WIND_SPEED_SETTING_OFFSET, "Setting offset");
  screen_menu_printf(p_win, HJ_WIND_SPEED_SATUS, "Status");
  screen_menu_clear(p_win);
}

int32_t ctrl_hj_wind_speed(void)
{

  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, "HJ WIND SPEED");

  while (1)
  {
    draw_ctrl_hj_wind_speed_page(&menu);
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
      case HJ_WIND_SPEED_INFO:
      {
        char buff[100]={"Time Out"};
        modbus_h_t modbus_h;
        uint16_t regs[3];
        int32_t len = 0;
        int32_t ret;
         modbus_h_t *p_modbus;
            
        p_modbus = hj_wind_speed_get_bus_io();
        if(p_modbus)
        {
          modbus_h = *p_modbus;
          modbus_h.id = 0xFE;


          ret = modbus_read_hold_reg(&modbus_h, REG_RW_ID, (uint16_t *)regs, _countof(regs));
          if(ret ==0)
          {
            len += make_sreen_row(&buff[0],  "ID     :%d",regs[0]);
            len += make_sreen_row(&buff[len],"Fullset:%d",regs[1]);
            len += make_sreen_row(&buff[len],"Offset :%d",regs[2]);

          }
           
          show_popup("HJ Wind Speed",buff);
        }
      }
        break;
      case HJ_WIND_SPEED_SETTING_ID:
        {
            char buff[40];
            modbus_h_t modbus_h;
            modbus_h_t *p_modbus;
            uint16_t regs[1];
            int ok;
            int32_t ret=-1;


            p_modbus = hj_wind_speed_get_bus_io();
            
            if(p_modbus)
            {
              modbus_h = *p_modbus;
              modbus_h.id = 0xFE;
             ret =  modbus_read_hold_reg(&modbus_h, REG_RW_ID, (uint16_t *)regs, _countof(regs));
            }

            if(ret == 0)
            {
              snprintf(buff, sizeof(buff), "ID:%d,Change?\r\n",regs[0]);
              ok = 0;
              input_active(buff, &ok);
              if (ok)
              {
               int32_t val = regs[0];
                status = input_decimal("ID", 0, 247, &val);
                if(status == MENU_OK)
                {
                  modbus_write_holding_reg(&modbus_h,REG_RW_ID,val);
               }
              }
       
          }
        }
      break;

      case HJ_WIND_SPEED_SETTING_FULLSET:
     {
            char buff[40];
            modbus_h_t modbus_h;
            modbus_h_t *p_modbus;
            uint16_t regs[1];
            int ok;
            int32_t ret=-1;


            p_modbus = hj_wind_speed_get_bus_io();
            
            if(p_modbus)
            {
              modbus_h = *p_modbus;
              modbus_h.id = 0xFE;
             ret = modbus_read_hold_reg(&modbus_h, REG_RW_FULLSET, (uint16_t *)regs, _countof(regs));
            }

            if(ret == 0)
            {
              snprintf(buff, sizeof(buff), "Fullset:%d,Change?\r\n",regs[0]);
              ok = 0;
              input_active(buff, &ok);
              if (ok)
              {
               int32_t val = regs[0];
                status = input_decimal("Fullset", 0, 65535, &val);
                if(status == MENU_OK)
                {
                  modbus_write_holding_reg(&modbus_h,REG_RW_FULLSET,val);
               }
              }
       
          }
        }
      break;
      case HJ_WIND_SPEED_SETTING_OFFSET:
         {
            char buff[40];
            modbus_h_t modbus_h;
            modbus_h_t *p_modbus;
            uint16_t regs[1];
            int ok;
            int32_t ret=-1;

            p_modbus = hj_wind_speed_get_bus_io();
            
            if(p_modbus)
            {
              modbus_h = *p_modbus;
              modbus_h.id = 0xFE;
              ret = modbus_read_hold_reg(&modbus_h, REG_RW_OFFSET, (uint16_t *)regs, _countof(regs));
            }

            if(ret == 0)
            {
              snprintf(buff, sizeof(buff), "Offset:%d,Change?\r\n",regs[0]);
              ok = 0;
              input_active(buff, &ok);
              if (ok)
              {
               int32_t val = regs[0];
                status = input_decimal("Offset", 0, 65535, &val);
                if(status == MENU_OK)
                {
                  modbus_write_holding_reg(&modbus_h,REG_RW_OFFSET,val);
               }
              }
 
          }
        }
      break;
      case HJ_WIND_SPEED_SATUS:
      {
        char buff[100]={"Time Out"};
        modbus_h_t *modbus_h;
        uint16_t regs[3];
        int32_t len = 0;
        int32_t ret;
        int8_t major,minor,fix,release;
        
        modbus_h = hj_wind_speed_get_bus_io();
        if(modbus_h)
        {
          modbus_h->id = 0xFE;
          /*
#define		AS_OCF			0x20	// 데이터 유효(1일때 정상)
#define		AS_COF			0x10	// cordic 범위 오류(1일때 오류)
#define		AS_LIN			0x08	// 자기장 강도 부적절 (선형성 오류)(0일때 정상)
#define		AS_MAGINC		0x04	// 자기장 강도 강함	(0일때 정상)
#define		AS_MAGDEC		0x02	// 자기장 강도 약함 (0일때 정상)
#define		AS_EVPARITY 	0x01	// 페리티 검출
*/
         ret =  modbus_read_hold_reg(modbus_h, REG_R_VERSION, (uint16_t *)regs, _countof(regs));
         if(ret==0)
         {
           major   = (regs[0]>>12)&0x000F;
           minor   = (regs[0]>>8)&0x000F;
           fix     = (regs[0]>>4)&0x000F;
           release = regs[0]&0x000F;           
          len += make_sreen_row(&buff[len],"Ver    :%d.%d.%d.%d",major,minor,fix,release);
          len += make_sreen_row(&buff[len],"Status :0x%04X",regs[1]);
          len += make_sreen_row(&buff[len],"Type   :%s",regs[2]==1?"Speed":"Direction");

         }
                   show_popup("HJ Wind Speed",buff);
        }
      }
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