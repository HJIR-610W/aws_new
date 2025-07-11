
#include "aws_menu_panel.h"

#include "config_app.h"
#include "console_define.h"
#include "console_utile.h"
#include "dev_io.h"
#include "util_memory.h"

const char* panelList[] = {"AWS STD", "HJ STD", "MOOJU", "HANSUNG"};

#define AWS_MENU_PANEL_CNT 3
int32_t aws_menu_panel(void)
{
  int32_t choice, status;
  char buff[AWS_MENU_PANEL_CNT][20];
  char* menu[AWS_MENU_PANEL_CNT];
  int32_t menu_cnt = 0;
  bool enalbe;

  for (int32_t i = 0; i < AWS_MENU_PANEL_CNT; i++)
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
      enalbe = get_config_app()->panel_snow_active;
      snprintf(buff[menu_cnt],sizeof(buff[menu_cnt]),"적설 출력:%s", ITEM_LIST((int32_t)enalbe, enableList));
      menu_cnt++;

      enalbe = get_config_app()->panel_barometer_active;
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
          break;

        config.panel_model = (ePANEL_MODEL_t)(choice-1);
        WRITE_CFG(panel_model);
        break;
      case 2:
        status = choice_enable(&get_config_app()->panel_snow_active);
        if(status != MENU_OK)
          break;
          WRITE_CFG(panel_snow_active);
        break;
      case 3:
        status = choice_enable(&get_config_app()->panel_barometer_active);
        if(status != MENU_OK)
        break;
        WRITE_CFG(panel_barometer_active);
        break;
    }

    if(status != MENU_OK)
    {
      break;
    }
  }

  return status;
}