#include "aws_panel.h"

#include "config_app.h"
#include "const_string.h"
#include "console_define.h"
#include "console_utile.h"
#include "debug_io.h"
#include "util_memory.h"


int32_t aws_panel(void)
{
  char buff[4][30];
  const char* menu[4];
  int choice, status;
  int menu_cnt;
  bool enable;

  for (int i = 0; i < 4; i++)
  {
    menu[i] = buff[i];
  }

  while (1)
  {
    menu_cnt = 0;

    snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "패널 종류:%s",
             ITEM_LIST(config.panel_model, panel_list_eng));
    menu_cnt++;

    if (get_config_app()->panel_model == ePANEL_MUJU)
    {
      enable = get_config_app()->panel_snow_active;
      snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "적설 출력:%s",
               ITEM_LIST((int32_t)enable, enable_list_kor));
      menu_cnt++;

      enable = get_config_app()->panel_barometer_active;
      snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "기압 출력:%s",
               ITEM_LIST((int32_t)enable, enable_list_kor));
      menu_cnt++;
    }
    else if (get_config_app()->panel_model == ePANEL_ITEM6)
    {
      snprintf(buff[menu_cnt], sizeof(buff[menu_cnt]), "TYPE:%s",
               ITEM_LIST((int32_t)get_config_app()->panel_item6_type, panel_item6_type_list_eng));
      menu_cnt++;
    }

    status = view_input_combobox("패널(전광판)", menu, menu_cnt, &choice);
    if (status != MENU_OK)
      return status;

    switch (choice)
    {
      case 1:
      {
        int pre_panel_model;

        choice = config.panel_model;
        pre_panel_model = choice;
        status = view_input_combobox("패널 종류", (const char **)panel_list_eng,
                                      _countof(panel_list_eng), &choice);
        if (status != MENU_OK)
          break;

        if (config.panel_model != (choice - 1))
        {
          config.panel_model = (ePANEL_MODEL_t)(choice - 1);
          WRITE_CFG(panel_model);

          if (pre_panel_model == ePANEL_NOT_USED || (choice - 1) == ePANEL_NOT_USED)
          {
            debug_printf_color(IO_COLOR_RED, "리셋 후 적용됩니다\r\n");
          }
        }
      }
      break;

      case 2:
      {
        if (get_config_app()->panel_model == ePANEL_MUJU)
        {
          choice = get_config_app()->panel_snow_active;
          status = view_input_active("적설 사용", &choice);
          if (status != MENU_OK)
            break;
          config.panel_snow_active = choice;
          WRITE_CFG(panel_snow_active);
        }
        else if (get_config_app()->panel_model == ePANEL_ITEM6)
        {
          choice = config.panel_item6_type;
          status = view_input_combobox("Type", panel_item6_type_list_eng,
                                        _countof(panel_item6_type_list_eng), &choice);
          if (status != MENU_OK)
            break;
          config.panel_item6_type = (ePANEL_ITEM6_TYPE_t)(choice - 1);
          WRITE_CFG(panel_item6_type);
        }
      }
      break;

      case 3:
      {
        if (get_config_app()->panel_model == ePANEL_MUJU)
        {
          choice = get_config_app()->panel_barometer_active;
          status = view_input_active("기압 사용", &choice);
          if (status != MENU_OK)
            break;
          config.panel_barometer_active = choice;
          WRITE_CFG(panel_barometer_active);
        }
      }
      break;
    }

    if (status != MENU_OK)
    {
      break;
    }
  }

  return status;
}
