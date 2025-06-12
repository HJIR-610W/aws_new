

#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"
#include "config_app.h"

const char* panelList[] = {"AWS STD", "HJ_STD", "MOOJU", "HANSUNG"};


int32_t print_menu_panel()
{
  int32_t cnt = 0;
  bool enalbe;
  io_printf("%2d.패널 종류:%s\r\n", cnt++, ITEM_LIST(config.panel_model, panelList));

  // 무주인 경우 추가 설정 출력
  if (get_config_app()->panel_model == ePANEL_MUJU)
  {
    enalbe = get_config_app()->panel_snow_use;
    io_printf("%2d.적설 출력:%s\r\n", cnt++, ITEM_LIST((int32_t)enalbe, enableList));

    enalbe = get_config_app()->panel_barometer_use;
    io_printf("%2d.기압 출력:%s\r\n", cnt++, ITEM_LIST((int32_t)enalbe, enableList));
  }
  return cnt;
}
int32_t aws_menu_display_panel()
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList( NULL, print_menu_panel, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
        cnt = select_indexFromList( panelList, NULL, _countof(panelList), true);

        if (cnt > 0)
        {
          cnt--;
          config.panel_model = (ePANEL_MODEL_t)cnt;
          WRITE_CFG(panel_model);
        }
        break;
      case 1:
        if (input_use(&get_config_app()->panel_snow_use))
        {
          WRITE_CFG(panel_snow_use);
        }
        break;
      case 2:
        if (input_use( &get_config_app()->panel_barometer_use))
        {
          WRITE_CFG(panel_barometer_use);
        }
        break;
    }

  } while (1);
}

int aws_menu_panel(void)
{
  int choice, status;

  char buff[3][20];

  char* menu[3];
  int menu_cnt=0;
  bool enalbe;

  for(int i = 0 ; i< 3; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;
    snprintf(buff[menu_cnt],sizeof(buff[menu_cnt]),"패널 종류:%s",ITEM_LIST(config.panel_model, panelList));
    menu_cnt++;

    if (get_config_app()->panel_model == ePANEL_MUJU)
    {
      enalbe = get_config_app()->panel_snow_use;
      snprintf(buff[menu_cnt],sizeof(buff[menu_cnt]),"적설 출력:%s", ITEM_LIST((int32_t)enalbe, enableList));
      menu_cnt++;

      enalbe = get_config_app()->panel_barometer_use;
      snprintf(buff[menu_cnt],sizeof(buff[menu_cnt]),"기압 출력:%s",  ITEM_LIST((int32_t)enalbe, enableList));
      menu_cnt++;
    }


    status = choice_menu(24, "패널(전광판)", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
        status = choice_menu(24, "패널 종류",(char **)panelList, _countof(panelList), &choice);
        if(status != MENU_OK)
        {
          break;
        }
        config.panel_model = (ePANEL_MODEL_t)(choice-1);
        WRITE_CFG(panel_model);
        break;
      case 2:
        if (input_use(&get_config_app()->panel_snow_use))
        {
          WRITE_CFG(panel_snow_use);
        }
        break;
        break;
      case 3:
        if (input_use(&get_config_app()->panel_snow_use))
        {
          WRITE_CFG(panel_snow_use);
        }
        break;
        break;
    }

    if(status != MENU_OK)
    {
      break;
    }
  }

  return status;
}