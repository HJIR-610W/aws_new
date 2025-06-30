
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
#include "Sensors\snow\hj_snow.h"
#define AWS_MENU_WIDTH 30




int hj_snow_menu(void)
{
  int choice, status;
  driver_t* hjsnow;

  uint8_t err;
  char* menu[] = {"설정값 확인", "0점 재조정 실행","적설 확인"};

  hjsnow = hjsnow_opened();
  if (hjsnow == NULL)
  {
    io_printf("화진 적설 센서를 설정해주세요\r\n");
    return MENU_BACK;
  }

  while (1)
  {
    status = choice_menu(AWS_MENU_WIDTH, "화진 적설", menu, _countof(menu), &choice);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 1:
  {
    hjsnow_read_config_t hjsnow_config;
    hjsnow_ctrl(hjsnow, eHJSNOW_GET_CONFIG, NULL, &hjsnow_config, &err);

    if (err)
    {
      io_printf("화진 적설센서 에러 %s\r\n", get_drv_err_name(err));
      break;
      ;
    }

        io_printf("블르투스 1     :%s\r\n", hjsnow_config.config.xModel[0]);
        io_printf("블르투스 2     :%s\r\n", hjsnow_config.config.xModel[1]);
        io_printf("블르투스 3     :%s\r\n", hjsnow_config.config.xModel[2]);
        io_printf("스캔 주기      :%d\r\n", hjsnow_config.config.snow_scantime);
        io_printf("레퍼런스 길이 1:%d\r\n", hjsnow_config.config.snow_refdistance[0]);
        io_printf("레퍼런스 길이 2:%d\r\n", hjsnow_config.config.snow_refdistance[1]);
        io_printf("레퍼런스 길이 3:%d\r\n", hjsnow_config.config.snow_refdistance[2]);
        io_printf("높이           :%d\r\n", hjsnow_config.config.snow_stddistance);
        io_printf("스캔온도       :%d\r\n", hjsnow_config.config.snow_scantemp);
        io_printf("온도 스캔 모드 :%s\r\n",
                  hjsnow_config.config.snow_scantempauto == 0 ? "수동" : "자동");
        io_printf("필터 레벨      :%d\r\n", hjsnow_config.config.snow_filterlevel);
        io_printf("필터 동작      :%d\r\n", hjsnow_config.config.snow_nofiltermode);
        }
        break;
      case 2:
        hjsnow_ctrl(hjsnow, eHJSNOW_RUN_ZERO, NULL, NULL, &err);
        if (err)
        {
          io_printf("명령어가 전송 실패\r\n");
        }
        else
        {
          io_printf("명령어가 전송되었습니다\r\n");
          io_printf("레이저 포인터를 확인해주세요\r\n");
        }

        break;
      case 3:
        {
          hjsnow_read_system_t system;
          hjsnow_ctrl(hjsnow, eHJSNOW_GET_SYSTEM, NULL, &system, &err);

          if (err)
          {
            io_printf("화진 적설센서 에러 %s\r\n", get_drv_err_name(err));
            break;
            ;
          }

          io_printf("센서 1 측정:%d\r\n", system.system.CurDistance[0]);
          io_printf("센서 2 측정:%d\r\n", system.system.CurDistance[1]);
          io_printf("센서 3 측정:%d\r\n", system.system.CurDistance[2]);
          io_printf("온도:%d\r\n", system.system.innerTemp);
          io_printf("현재 적설:%d\r\n", system.system.CurSnowLevel);
      }
        break;
    }
  }

  return 0;
}