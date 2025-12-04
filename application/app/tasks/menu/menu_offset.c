#include "menu_offset.h"


#include "app_key.h"
#include "app_screen.h"
#include "app_sensor.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "config_sensor.h"
#include "console_utile.h"
#include "drv_adc.h"
#include "menu_handler.h"
#include "util_memory.h"
#include "view_driver.h"

#define SCREEN_COLS 16
#define OFFSET_WD 15

static const eSENSOR_TYPE_t g_offset_sensor_list[] = {
    A1_TEMPERATURE,
    A2_WIND_DIRECTION,
    A10_RELATIVE_HUMIDITY,
    A7_PRESSURE,
    B1_SOLAR_RADIATION,
    B5_SOIL_TEMPERATURE_5CM,
    B6_SOIL_TEMPERATURE_10CM,
    B7_SOIL_TEMPERATURE_20CM,
    B8_SOIL_TEMPERATURE_30CM,
    B9_SOIL_TEMPERATURE_50CM,
    B10_SOIL_TEMPERATURE_100CM,
    B11_SOIL_TEMPERATURE_150CM,
    B12_SOIL_TEMPERATURE_300CM,
    B13_SOIL_TEMPERATURE_500CM};

#define OFFSET_SENSOR_COUNT (sizeof(g_offset_sensor_list) / sizeof(g_offset_sensor_list[0]))

void draw_offset_page(screen_menu_t* p_win)
{
  sensor_t* p_sensor;

  screen_menu_start(p_win);

  for (int32_t i = 0; i < OFFSET_SENSOR_COUNT; i++)
  {
    p_sensor = &get_config_app()->sensor[g_offset_sensor_list[i]];

    screen_menu_printf(p_win, i, "%-*s:%4.2f", OFFSET_WD,
                       sensor_name_eng_list[g_offset_sensor_list[i]], p_sensor->offset);
  }

  screen_menu_clear(p_win);
}


int32_t setup_sensor_offset(eSENSOR_TYPE_t sensor_type)
{
  int32_t status;
  sensor_t* p_sensor;

  p_sensor = &get_config_app()->sensor[sensor_type];

  status = input_float_adv("Offset Value", -0.0f, 0.0f, &p_sensor->offset, "%8.3f");
  if (status == MENU_OK)
  {
    WRITE_CFG(sensor[sensor_type].offset);
  }


  return status;
}

int32_t setup_menu_offset(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  eSENSOR_TYPE_t selected_sensor;
  screen_menu_t menu;

  screen_menu_create(&menu,  "Offset");

  while (1)
  {
    draw_offset_page(&menu);
    screen_refresh();
    key = get_menu_key(WAIT_FOREVER);

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

      if (index < OFFSET_SENSOR_COUNT)
      {
        selected_sensor = g_offset_sensor_list[index];
        status = setup_sensor_offset(selected_sensor);
        if(status !=MENU_OK)
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