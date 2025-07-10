#include "menu_calibration.h"

#include "adc_calibration.h"
#include "app_adc.h"
#include "app_key.h"
#include "app_screen.h"
#include "cli_key_code.h"
#include "config_adc.h"
#include "console_utile.h"
#include "driver_adc.h"
#include "menu_handler.h"
#include "util_filter.h"
#include "util_memory.h"
#include "view_driver.h"
#include "app_adc.h"

extern config_adc_adv_t g_adc_config_ads1220;
extern config_adc_adv_t g_adc_config_stm32;
extern float g_current_temp;

#define SCREEN_COLS 20
#define CALI_WD 8

#define MENU_PRINTF screen_menu_printf_row

#define CALI_MENU_FACTORY       0
#define CALI_MENU_VIEW          1
#define CALI_MENU_INIT          2

#define FACTORY_MENU_SINGLE     0
#define FACTORY_MENU_DIFF       1

#define VIEW_MENU_SINGLE        0
#define VIEW_MENU_DIFF          1
#define VIEW_MENU_SUMMARY       2

void draw_setup_menu_calibration_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, CALI_MENU_FACTORY);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "FACTORY");

  screen_update_list(p_win, row_count, CALI_MENU_VIEW);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "VIEW");

  screen_update_list(p_win, row_count, CALI_MENU_INIT);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "INIT");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

void draw_cali_setup_menu_factory_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, FACTORY_MENU_SINGLE);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "SINGLE");

  screen_update_list(p_win, row_count, FACTORY_MENU_DIFF);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "DIFF");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

void draw_cali_setup_menu_view_page(screen_menu_t* p_win)
{
  int32_t row_count = 0;

  p_win->current_row = 0;

  screen_update_list(p_win, row_count, VIEW_MENU_SINGLE);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "SINGLE");

  screen_update_list(p_win, row_count, VIEW_MENU_DIFF);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "DIFF");

  screen_update_list(p_win, row_count, VIEW_MENU_SUMMARY);
  MENU_PRINTF(p_win, row_count++, "%-*s", CALI_WD, "SUMMARY");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

int32_t cali_setup_menu_factory_calibration(adc_channel_type_t type)
{
  const char* confirm_menu[] = {"No", "Yes"};
  uint8_t err;
  int32_t adc_raw;
  int32_t avg_cnt;
  int32_t channel;
  int32_t choice = 0;
  int32_t status;
  float avg;
  float cal_temp;
  adc_cal_params_t* cal_params_ptr;
  adc_cal_point_t p1;
  adc_cal_point_t p2;
  config_adc_adv_t* p_adc;

  p_adc = &g_adc_config_ads1220;

  screen_clear(8, 20);
  screen_printf(0, 0, "Channel (0-%d):", 
                (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? 17 : 7);
  screen_refresh();

  status = input_decimal("Channel", 0, 
                        (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? 17 : 7, 
                        &channel);
  if (status != MENU_OK)
    return status;

  cal_params_ptr = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                       ? &p_adc->single_ended_cal[channel]
                       : &p_adc->differential_cal[channel];

  screen_clear(8, 20);
  screen_printf(0, 0, "Low Ref Ready?");
  screen_refresh();

  status = print_menu_list(confirm_menu, 2, &choice);
  if (status != MENU_OK || choice == 0)
    return status;

  avg = 0;
  avg_cnt = 0;

  screen_clear(8, 20);
  screen_printf(0, 0, "Reading Low...");
  screen_printf(1, 0, "Press any key");
  screen_refresh();

  while (1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      adc_raw = (int32_t)adc_read_single_raw(channel, &err);
    }
    else
    {
      adc_raw = (int32_t)adc_read_diff_raw(channel, &err);
    }

    avg_cnt++;
    avg = recursive_avg_i(avg, adc_raw, avg_cnt);

    screen_clear(8, 20);
    screen_printf(0, 0, "RAW:%d", adc_raw);
    screen_printf(1, 0, "AVG:%.0f", avg);
    screen_printf(2, 0, "Press any key");
    screen_refresh();

    if (get_button_key(100) != -1)
      break;
  }

  screen_clear(8, 20);
  screen_printf(0, 0, "Low RAW:");
  screen_refresh();

  status = input_decimal("Low RAW", p_adc->bits->min_raw_value, 
                        p_adc->bits->max_raw_value, (int32_t*)&p1.raw_value);
  if (status != MENU_OK)
    return status;

  status = input_float("Low Value", -1000.0f, 1000.0f, &p1.reference_value, "%8.3f");
  if (status != MENU_OK)
    return status;

  screen_clear(8, 20);
  screen_printf(0, 0, "High Ref Ready?");
  screen_refresh();

  choice = 0;
  status = print_menu_list(confirm_menu, 2, &choice);
  if (status != MENU_OK || choice == 0)
    return status;

  avg = 0;
  avg_cnt = 0;

  screen_clear(8, 20);
  screen_printf(0, 0, "Reading High...");
  screen_printf(1, 0, "Press any key");
  screen_refresh();

  while (1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      adc_raw = (int32_t)adc_read_single_raw(channel, &err);
    }
    else
    {
      adc_raw = (int32_t)adc_read_diff_raw(channel, &err);
    }

    avg_cnt++;
    avg = recursive_avg_i(avg, adc_raw, avg_cnt);

    screen_clear(8, 20);
    screen_printf(0, 0, "RAW:%d", adc_raw);
    screen_printf(1, 0, "AVG:%.0f", avg);
    screen_printf(2, 0, "Press any key");
    screen_refresh();

    if (get_button_key(100) != -1)
      break;
  }

  screen_clear(8, 20);
  screen_printf(0, 0, "High RAW:");
  screen_refresh();

  status = input_decimal("High RAW", p_adc->bits->min_raw_value, 
                        p_adc->bits->max_raw_value, (int32_t*)&p2.raw_value);
  if (status != MENU_OK)
    return status;

  status = input_float("High Value", -1000.0f, 1000.0f, &p2.reference_value, "%8.3f");
  if (status != MENU_OK)
    return status;

  cal_temp = 25.0f;

  if (adc_perform_factory_calibration(p_adc, cal_params_ptr, p1, p2, cal_temp))
  {
    save_adc_cali();
    
    screen_clear(8, 20);
    screen_printf(0, 0, "Calibration OK");
    screen_refresh();
    
    const char* ok_menu[] = {"OK"};
    choice = 0;
    print_menu_list(ok_menu, 1, &choice);
  }
  else
  {
    screen_clear(8, 20);
    screen_printf(0, 0, "Calibration");
    screen_printf(1, 0, "Failed");
    screen_refresh();
    
    const char* ok_menu[] = {"OK"};
    choice = 0;
    print_menu_list(ok_menu, 1, &choice);
  }

  return status;
}

int32_t cali_setup_menu_factory(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

  while (1)
  {
    draw_cali_setup_menu_factory_page(&menu);
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
        case FACTORY_MENU_SINGLE:
          status = cali_setup_menu_factory_calibration(ADC_CHANNEL_TYPE_SINGLE_ENDED);
          break;

        case FACTORY_MENU_DIFF:
          status = cali_setup_menu_factory_calibration(ADC_CHANNEL_TYPE_DIFFERENTIAL);
          break;

        default:
          break;
      }
    }
    else if (key != -1)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t cali_setup_menu_view_channel(adc_channel_type_t type)
{
  const adc_cal_params_t* params;
  uint8_t err;
  int32_t channel;
  int32_t raw;
  int32_t status;
  float voltage;
  config_adc_adv_t* p_adc;

  p_adc = &g_adc_config_ads1220;

  screen_clear(8, 20);
  screen_printf(0, 0, "Channel (0-%d):", 
                (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? 17 : 7);
  screen_refresh();

  status = input_decimal("Channel", 0, 
                        (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? 17 : 7, 
                        &channel);
  if (status != MENU_OK)
    return status;

  params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
               ? &p_adc->single_ended_cal[channel]
               : &p_adc->differential_cal[channel];

  screen_clear(8, 20);
  screen_printf(0, 0, "Ch%d %s", channel, 
                (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? "SE" : "DIFF");
  screen_printf(1, 0, "Cal:%s", params->is_calibrated ? "YES" : "NO");
  screen_printf(2, 0, "Press any key");
  screen_refresh();

  while (1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      raw = (int32_t)adc_read_single_raw(channel, &err);
    }
    else
    {
      raw = adc_read_diff_raw(channel, &err);
    }

    g_current_temp = read_current_temperature();
    voltage = adc_get_compensated_value(raw, params, g_current_temp);

    screen_clear(8, 20);
    screen_printf(0, 0, "Ch%d RAW:%d", channel, raw);
    if (isnan(voltage))
    {
      screen_printf(1, 0, "Need Cal");
    }
    else
    {
      screen_printf(1, 0, "V:%.6f", voltage);
    }
    screen_printf(2, 0, "Temp:%.1f", g_current_temp);
    screen_refresh();

    if (get_button_key(500) != -1)
      break;
  }

  return MENU_OK;
}


void draw_cali_menu_view_summary(screen_page_t* p_win)
{
  int row_count = 0;
  int page = p_win->current_page;
  char buff[SCREEN_COLS + 1];
  const char* message;
  const adc_cal_params_t* params;
  int32_t raw;
  float voltage;
  uint8_t err;
  config_adc_adv_t* p_adc;


  p_adc = &g_adc_config_ads1220;

  p_win->current_row = 0;

  make_centered(buff, sizeof(buff), "SE Channels(V)", SCREEN_COLS);
  screen_printf_row(p_win, row_count++, "%s", buff);


  g_current_temp = read_current_temperature();

  for (int32_t channel = 0; channel < 18; channel++)
  {
    params = &p_adc->single_ended_cal[channel];
    raw = (int32_t)adc_read_single_raw(channel, &err);
    voltage = adc_get_compensated_value(raw, params, g_current_temp);

    if (isnan(voltage))
    {
      screen_printf_row(p_win, row_count++, "%d:NC", channel);
    }
    else
    {
      screen_printf_row(p_win, row_count++, "%d:%.4f", channel, voltage);
    }
  }

  p_win->total_items[0] = ALIGN_UP(row_count, p_win->view_row);

  while (p_win->current_row < p_win->view_row)
  {
    screen_clear_row(p_win, row_count++);
  }
}

int32_t cali_setup_menu_view_summary(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_page_t lcd_win;
  
  screen_page_create(&lcd_win,8,20);
    
  lcd_win.total_pages =1;
   while (1)
  {
    draw_cali_menu_view_summary(&lcd_win);

    screen_refresh();

    key = get_button_key(100);
    
    if (key == KEY_CODE_CTRL_Q)
    {
      break;
    }
    else if(key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if (key != -1)
    {
      screen_handle_scroll(&lcd_win, key);
    }
  }

  return convert_key_to_status(key);
}







int32_t cali_setup_menu_view(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

  while (1)
  {
    draw_cali_setup_menu_view_page(&menu);
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
        case VIEW_MENU_SINGLE:
          status = cali_setup_menu_view_channel(ADC_CHANNEL_TYPE_SINGLE_ENDED);
          break;

        case VIEW_MENU_DIFF:
          status = cali_setup_menu_view_channel(ADC_CHANNEL_TYPE_DIFFERENTIAL);
          break;

        case VIEW_MENU_SUMMARY:
          status = cali_setup_menu_view_summary();
          break;

        default:
          break;
      }
    }
    else if (key != -1)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t cali_setup_menu_init(void)
{
  const char* confirm_menu[] = {"No", "Yes"};
  int32_t choice = 0;
  int32_t status;

  screen_clear(8, 20);
  screen_printf(0, 0, "Init Calibration?");
  screen_refresh();

  status = print_menu_list(confirm_menu, 2, &choice);

  if (status == MENU_OK && choice == 1)
  {
    adc_config_init(&g_adc_config_ads1220, 24, 5.0f);

    for (int32_t channel = 0; channel < g_adc_config_ads1220.params_se_cnt; channel++)
    {
      g_adc_config_ads1220.single_ended_cal[channel].comp_method = TEMP_COMP_NONE;
      g_adc_config_ads1220.single_ended_cal[channel].factory_cal_temp = 25.0f;
      g_adc_config_ads1220.single_ended_cal[channel].is_calibrated = true;
      g_adc_config_ads1220.single_ended_cal[channel].factory_offset = 4.928633e-03f;
      g_adc_config_ads1220.single_ended_cal[channel].factory_slope = 5.958932e-07f;
      g_adc_config_ads1220.single_ended_cal[channel].offset_temp_coeff = 1.0f;
      g_adc_config_ads1220.single_ended_cal[channel].slope_temp_coeff = 1.0f;
    }

    for (int32_t channel = 0; channel < g_adc_config_ads1220.params_di_cnt; channel++)
    {
      g_adc_config_ads1220.differential_cal[channel].comp_method = TEMP_COMP_NONE;
      g_adc_config_ads1220.differential_cal[channel].factory_cal_temp = 25.0f;
      g_adc_config_ads1220.differential_cal[channel].is_calibrated = true;
      g_adc_config_ads1220.differential_cal[channel].factory_offset = 4.928633e-03f;
      g_adc_config_ads1220.differential_cal[channel].factory_slope = 5.958932e-07f;
      g_adc_config_ads1220.differential_cal[channel].factory_offset_trim = 0.0f;
      g_adc_config_ads1220.differential_cal[channel].offset_temp_coeff = 1.0f;
      g_adc_config_ads1220.differential_cal[channel].slope_temp_coeff = 1.0f;
    }

    adc_config_init(&g_adc_config_stm32, 12, 3.3f);

    for (int32_t channel = 0; channel < STM32_NUM_SINGLE_ENDED_CHANNELS; channel++)
    {
      g_adc_config_stm32.single_ended_cal[channel].comp_method = TEMP_COMP_NONE;
      g_adc_config_stm32.single_ended_cal[channel].factory_cal_temp = 25.0f;
      g_adc_config_stm32.single_ended_cal[channel].is_calibrated = true;
      g_adc_config_stm32.single_ended_cal[channel].factory_offset = 0.0f;
      g_adc_config_stm32.single_ended_cal[channel].factory_slope = 8.05e-04f;
      g_adc_config_stm32.single_ended_cal[channel].factory_offset_trim = 0.0f;
      g_adc_config_stm32.single_ended_cal[channel].offset_temp_coeff = 1.0f;
      g_adc_config_stm32.single_ended_cal[channel].slope_temp_coeff = 1.0f;
    }

    save_adc_cali();

    screen_clear(8, 20);
    screen_printf(0, 0, "Init Complete");
    screen_refresh();

    const char* ok_menu[] = {"OK"};
    choice = 0;
    print_menu_list(ok_menu, 1, &choice);
  }

  return status;
}

int32_t setup_menu_calibration(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  screen_menu_t menu;

  adc_init();

  screen_menu_create(&menu, 8, 20);

  while (1)
  {
    draw_setup_menu_calibration_page(&menu);
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
        case CALI_MENU_FACTORY:
          status = cali_setup_menu_factory();
          break;

        case CALI_MENU_VIEW:
          status = cali_setup_menu_view();
          break;

        case CALI_MENU_INIT:
          status = cali_setup_menu_init();
          break;

        default:
          break;
      }
    }
    else if (key != -1)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}