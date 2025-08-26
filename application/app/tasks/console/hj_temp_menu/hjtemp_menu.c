

#include "Sensors\snow\hj_snow.h"
#include "aws_develop.h"
#include "aws_menu.h"
#include "aws_menu_cali.h"
#include "aws_menu_data.h"

#include "aws_menu_manager.h"
#include "aws_menu_offset.h"
#include "aws_menu_panel.h"
#include "aws_menu_sensor.h"
#include "aws_network.h"
#include "aws_system.h"
#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"
#include "Sensors\temperature\hj_temperature.h"
#include "Sensors\humidity\hj_huminity.h"
#include "config_app.h"
#include "config_sensor.h"
#include "modbus_master.h"

#define AWS_MENU_WIDTH 30

int hjtemperature_menu(void)
{
  int choice, status;
  driver_t* hjtemp;
  driver_t* hjhumi;

  uint8_t err;
  char* menu[] = {"설정값 확인(구현 예정)","온도 오프셋 변경" ,"습도 오프셋 변경","온습도 확인"};

  hjtemp = hjtemp_opened();
  if (hjtemp == NULL)
  {
    io_printf("화진 온도를 설정해주세요\r\n");
    return MENU_BACK;
  }

  hjhumi = hjHumi_opened();
  if(hjhumi == NULL)
  {
    io_printf("화진 습도를 설정해주세요\r\n");
    return MENU_BACK;
  }

      while (1)
  {
    status = choice_menu(AWS_MENU_WIDTH, "화진 온습도", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 1:
      {
         modbus_h_t *modbus_h;;
        hjtemp_register_map_t map;

        modbus_h = get_hjtemperature_bus_io();

        modbus_read_hold_reg(modbus_h,  0, (uint16_t *)&map, 16);
        io_printf("SW Version:%d\r\n", map.sw_version);
        io_printf("HW Version:%d\r\n", map.hw_version);

      }
      break;
      case 2:
      {



        int ok;
        int data;
  

        hjtemperature_ctrl(hjtemp, eTEMP_GET_OFFSET, NULL, (void*)&data, &err);
        if (err == 0)
        {
          io_printf("현재 온도 오프셋:%.2f\r\n", ((float)data / 100.0f));
          status = confirm_continue("오프셋을 변경하시겠습니까?", &ok);
          if (status != MENU_OK)
            break;

          if (ok)
          {
            float f_offset;

            status = input_float_prompt("오프셋을 입력해주세요",-5,5,&f_offset);
            if(status !=MENU_OK)
            break;
            
            data = (uint16_t)(f_offset * 100);
            hjtemperature_ctrl(hjtemp, eTEMP_SET_OFFSET, (void*)&data, NULL, &err);
            
          }
        }
        else
        {
          io_printf("장치에 접근할 수 없습니다.\r\n");
        }
      }
        break;
      case 3:
        {
          float f_offset;
 

          int ok;
          int data;
      

          hjtemperature_ctrl(hjtemp, eHUMI_GET_OFFSET, NULL, (void*)&data, &err);

          if (err == 0)
          {
            io_printf("현재 습도 오프셋:%.2f\r\n", ((float)data / 100.0f));
            status = confirm_continue("오프셋을 변경하시겠습니까?", &ok);
            if (status != MENU_OK)
              break;
            if (ok)
            {
              status = input_float_prompt("오프셋을 입력해주세요",-5, 5, &f_offset);
              if(status !=MENU_OK)
              break;
                data = (uint16_t)(f_offset * 100);

                hjtemperature_ctrl(hjtemp, eHUMI_SET_OFFSET, (void*)&data,NULL,&err);
         
            }
          }
          else
          {
            io_printf("장치에 접근할 수 없습니다.\r\n");
          }
        }

      break;
      case 4:
      {
        float temp;
        temp = hjTemperature_read(hjtemp,&err);

        if(err == 0)
        {
          io_printf("온도:%.2f\r\n",temp);
        }
        else
        {
          io_printf("온도 통신 실패 %s\r\n", get_drv_err_name(err));
        }
        float humi;
        humi = hjHuminity_read(hjhumi,&err);
        if(err==0)
        {
          io_printf("습도:%.2f\r\n", humi);
        }
        else{
          io_printf("습도 통신 실패 %s\r\n", get_drv_err_name(err));
        }
      }
    }
  }

  return status;
}