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

#define M_PRINTF screen_menu_printf_row

static const eSENSOR_TYPE_t g_offset_sensor_list[] = {
    A1_TEMPERATURE,
    A2_WIND_DIRECTION,
    A10_RELATIVE_HUMIDITY,
    A7_PRESSURE,
    B1_SOLAR_RADIATION,
    B2_SUNSHINE_DURATION,
    B3_GROUND_TEMPERATURE,
    B4_SURFACE_TEMPERATURE,
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
  int32_t row_count = 0;
  sensor_t* p_sensor;

  p_win->current_row = 0;

  for (int32_t i = 0; i < OFFSET_SENSOR_COUNT && row_count < p_win->view_row; i++)
  {
    p_sensor = &get_config_app()->sensor[g_offset_sensor_list[i]];

    screen_update_list(p_win, row_count, i);
    M_PRINTF(p_win, row_count++, "%-*s:%4.2f", OFFSET_WD,
             sensor_name_eng_list[g_offset_sensor_list[i]], p_sensor->offset);
  }

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }

  screen_refresh();
}

//ADC 자체의 오프셋을 수정하려면 이함수 추후 사용
int32_t setup_pressure_offset(eSENSOR_TYPE_t sensor_type)
{

  uint8_t error;
  int32_t choice;
  int32_t status = MENU_OK;

  float measured_value;
  float new_offset;
  float reference_value;
  float voltage;
  adc_config_t* cfg;
  sensor_t* p_sensor;

  p_sensor = &get_config_app()->sensor[sensor_type];

  if (p_sensor->type != S_T_ADC)
  {
    status = input_float("Offset Value", -1000.0f, 1000.0f, &p_sensor->offset, "%8.3f");
    if (status == MENU_OK)
    {
      WRITE_CFG(sensor[sensor_type].offset);
    }
    return status;
  }

  cfg = get_sensor_config(p_sensor);
  if (cfg == NULL)
  {
    return MENU_ERROR;
  }

  voltage = drv_adc_single_read_voltage(cfg->single_channel, 10,&error);
  measured_value = voltage;

  screen_clear();
  
  screen_printf(0, 0, "Current: %.3f", measured_value);
  screen_printf(1, 0, "ADC Ch%d: %.3fV", cfg->single_channel, voltage);
  screen_refresh();

  status = input_float("Reference Value", -1000.0f, 1000.0f, &reference_value, "%8.3f");
  if (status != MENU_OK)
  {
    return status;
  }

  new_offset = reference_value - measured_value;



  choice = 0;

  status = input_active("Apply?",  &choice);

  if (status == MENU_OK && choice == 1)
  {
    p_sensor->offset = new_offset;
    WRITE_CFG(sensor[sensor_type].offset);

    drv_adc_set_offset( cfg->single_channel, new_offset);
  }

  return status;
}

int32_t setup_sensor_offset(eSENSOR_TYPE_t sensor_type)
{
  int32_t status;
  sensor_t* p_sensor;

  p_sensor = &get_config_app()->sensor[sensor_type];

  status = input_float("Offset Value", -1000.0f, 1000.0f, &p_sensor->offset, "%8.3f");
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