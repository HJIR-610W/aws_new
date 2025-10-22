#include "menu_calibration.h"

#include "adc_calibration.h"
#include "app_adc.h"

#include "app_adc.h"
#include "app_key.h"
#include "app_screen.h"
#include "bsp_delay.h"
#include "cli_key_code.h"
#include "const_string.h"
#include "config_adc.h"
#include "console_utile.h"
#include "drv_adc.h"
#include "menu_handler.h"
#include "util_filter.h"
#include "util_memory.h"
#include "task_measure.h"
#include "util_stdio.h"
#include "view_driver.h"

extern config_adc_adv_t g_adc_config_ads1220;
extern config_adc_adv_t g_adc_config_stm32;
extern float g_current_temp;

#define SCREEN_COLS 20
#define CALI_WD 8


#define CALI_MENU_FACTORY       0
#define CALI_MENU_VIEW          1
#define CALI_MENU_ADC_INIT 2


#define FACTORY_MENU_SINGLE     0
#define FACTORY_MENU_DIFF       1
#define FACTORY_MENU_SINGLE_ALL 2

#define VIEW_ADC_SINGLE_SUMMARY  1
#define VIEW_ADC_DIFF_SUMMARY    2
#define VIEW_MENU_SUMMARY_DEFAULT  3


int32_t view_single_channel_details(adc_channel_type_t type,int32_t channel);

void draw_setup_menu_calibration_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, CALI_MENU_FACTORY, "Factory Calibration"); // Factory Calibration
  screen_menu_printf(p_win, CALI_MENU_VIEW, "View Summary");
  screen_menu_printf(p_win, CALI_MENU_ADC_INIT, "Calibration Init");
  screen_menu_clear(p_win);

}

bool g_cali_single_all_active = false;
void draw_cali_setup_menu_factory_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, FACTORY_MENU_SINGLE ,"Single");
  screen_menu_printf(p_win, FACTORY_MENU_DIFF, "Diff");
  if(g_cali_single_all_active)
  {
    screen_menu_printf(p_win, FACTORY_MENU_SINGLE_ALL, "Single All");
  }
  screen_menu_clear(p_win);
}

bool g_summary_default_en = false;

void draw_cali_setup_menu_view_page(screen_menu_t* p_win)
{
  screen_menu_start(p_win);
  screen_menu_printf(p_win, VIEW_ADC_SINGLE_SUMMARY, "Summary Single-Ended");
  screen_menu_printf(p_win, VIEW_ADC_DIFF_SUMMARY, "Summary Differantial");
  if (g_summary_default_en)
    screen_menu_printf(p_win, VIEW_MENU_SUMMARY_DEFAULT, "Summary(Default)");
  screen_menu_clear(p_win);
}

#define CALI_POINT_1 0
#define CALI_POINT_2 1
int32_t cali_point(adc_channel_type_t type, int32_t channel, int32_t point, adc_cal_point_t *cali_p)
{
  const char *point_list[2]={"Low","High"};
  int32_t adc_raw;
  int32_t avg_cnt=0;
  uint8_t err=0;
  config_adc_adv_t *p_adc;
  adc_cal_params_t *p_cal_params;
  int32_t key;
  float avg=0;
  float input_min;
  float input_max;
  int32_t status;
  p_adc = &g_adc_config_ads1220;
  char buffer[21];

  if(type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
  {
    if (channel<DRV_ADS1220_S_CH_16)
      snprintf(buffer, sizeof(buffer), "%s Input %s V", adc_se_list[channel], point_list[point]);
      else
        snprintf(buffer, sizeof(buffer), "%s Input %s R", adc_se_list[channel], point_list[point]);
  }
  else
  {
    snprintf(buffer, sizeof(buffer), "%s Input %s V", adc_diff_list[channel], point_list[point]);
  }
  show_popup("Information", buffer);

  while (1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      adc_raw = (int32_t)drv_adc_single_raw_read(channel, 1, &err);
    }
    else
    {
      adc_raw = (int32_t)drv_adc_diff_raw_read(channel, 1, &err);
    }

    avg_cnt++;
    avg = recursive_avg_i(avg, adc_raw, avg_cnt);

    screen_clear();
    screen_printf(0, 0, "RAW:%d", adc_raw);
    screen_printf(1, 0, "AVG:%.0f", avg);
    screen_printf(2, 0, "Press enter");
    screen_refresh();

    key = get_menu_key(500);
    if (key == KEY_CODE_CTRL_C || key == KEY_CODE_CTRL_Q)
      return convert_key_to_status(key);
    else if (key == KEY_CODE_ENTER)
      break;
  }

  screen_clear();
  cali_p->raw_value = (int32_t)avg;
  status = input_decimal("RAW", p_adc->bits->min_raw_value, p_adc->bits->max_raw_value, (int32_t *)&cali_p->raw_value);
  if (status != MENU_OK)
    return status;

  if (point == CALI_POINT_1)
  {
    if (channel < DRV_ADS1220_S_CH_16)
    {
      cali_p->reference_value = 0.01;
      input_min = -5.0f;
      input_max = 5.0f;
    }
    else //PT100 A,PT100 B
    {
      cali_p->reference_value = 84.27;
      input_min = 0.0f;
      input_max = 200.0f;
    }
  }
  else
  {
    if (channel < DRV_ADS1220_S_CH_16)
    {
      cali_p->reference_value = 4.99;
      input_min = -5.0f;
      input_max = 5.0f;
    }
    else//PT100 A,PT100 B
    {
      cali_p->reference_value = 123.24;
      input_min = 0.0f;
      input_max = 200.0f;
    }
  }

  status = input_float("Reference Voltage", input_min, input_max, &cali_p->reference_value, "%6.4f");
  if (status != MENU_OK)
    return status;


return status;

}




#define CALI_CHANNEL_START 0
void draw_calibraion_select_single_channel(screen_menu_t *p_win)
{
  const char *cali_status;
  adc_cal_params_t *p_cal_params;


  screen_menu_start(p_win);

  for (int channel=0; channel < _countof(adc_se_list); channel++)
  {
    p_cal_params =  &g_adc_config_ads1220.single_ended_cal[channel];
    cali_status = (p_cal_params->is_calibrated) ? "Calibrated" : "Uncalibrated";

    screen_menu_printf(p_win, CALI_CHANNEL_START+channel, "%s %s",adc_se_list[channel], cali_status);
  }
  screen_menu_clear(p_win);
}

void draw_calibraion_select_diff_channel(screen_menu_t *p_win)
{
  const char *cali_status;
  adc_cal_params_t *p_cal_params;

  screen_menu_start(p_win);

  for (int channel = 0; channel < _countof(adc_diff_list); channel++)
  {
    p_cal_params = &g_adc_config_ads1220.differential_cal[channel];
    cali_status = (p_cal_params->is_calibrated) ? "Calibrated" : "Uncalibrated";

    screen_menu_printf(p_win, CALI_CHANNEL_START + channel, "%s %s", adc_diff_list[channel], cali_status);
  }
  screen_menu_clear(p_win);
}



int32_t setup_factory_calibration(adc_channel_type_t type)
{

  const char *point_list[] = {"Point 1(Low)", "Point 2(High)","Manual P1.ADC","Manual P2.ADC","Init"};
  bool p1_calib_done = false;
  bool calib_updated = false;
  char buffer[100];
  int32_t status;
  int32_t key;
  int32_t channel;
  int32_t choice;
  int32_t dec;
  int32_t len = 0;
  adc_cal_params_t *p_cal_params;
  adc_cal_point_t p1;
  adc_cal_point_t p2;
  screen_menu_t menu;

  screen_menu_create(&menu, "Calibraion");
  menu.enter_long_key_active =true;


  while(1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      draw_calibraion_select_single_channel(&menu);
    }
    else
    {
      draw_calibraion_select_diff_channel(&menu);
    }

    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key == KEY_CODE_ENTER_LONG)
    {
      channel = menu.index_list[menu.selected_index];
      status = view_single_channel_details( type,  channel);
      if(status ==MENU_ABORT)
        return status;
    }
    else if (key == KEY_CODE_ENTER)
    {
        channel = menu.index_list[menu.selected_index];
        p_cal_params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                           ? &g_adc_config_ads1220.single_ended_cal[channel]
                           : &g_adc_config_ads1220.differential_cal[channel];

        // 기존 켈리브레이션 데이터를 복사
        // 갱신되지 않는 값은 이전값으로 사용함이 목적
        p1 = p_cal_params->p1_cal_point;
        p2 = p_cal_params->p2_cal_point;

        choice = 0;

        while (1)
        {
          /*
          켈리브레이션은 2포인트 P1(low),P2(high)한다
          레퍼런스 전압이 5v라면 적당한 선형구한 low:0.01v ,high:4.99v
          한번도 켈리브레이션 한적이 없으면 반드시 P1,P2 순으로 하고
          한번이상 켈리브레이션 한 상태에서 특정 포인트만 다시하고 싶으면 해당 포인트를 선택해서 진행한다
          한번이상 켈리브레시션 된 상태에서 특정 포인트를 켈리브레이션 하면 할때마다 slope과 offset이 재계산된다
          */
         if(type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
         {
           snprintf(buffer, sizeof(buffer), "%s Calibration", adc_se_list[channel]);
         }
         else
         {
           snprintf(buffer, sizeof(buffer), "%s Calibration", adc_se_list[channel]);
         }

          status = input_combobox(buffer, point_list, _countof(point_list), &choice);
          if (status == MENU_ABORT)
            return status;

          if (status != MENU_OK)
            break;

          if (p_cal_params->is_calibrated)
          {
            // P1을 안하고 P2를 할경우 P1을 안해도 되는지 판단해야함 켈리브레이션 된 상태라는건
            // P1은 이미 켈리브레이션 된 상태라는걸 알아야함
            p1_calib_done = true;
          }

          switch (choice)
          {
          case 0: // Point 1
            status = cali_point(type, channel, CALI_POINT_1, &p1);
            if (status != MENU_OK)
              break;

            // P1을 했는데 P2가 안한상태라면 무조건 P2를 하게 한다.
            // P1값은 현재 유지중이다.
            if (p_cal_params->is_calibrated == false)
            {
              p1_calib_done = true;
              show_popup("Information", "P2 Calib Required");
              break;
            }
            else
            {
              calib_updated = true;
            }

            break;
          case 1: // Point 2
            if (p1_calib_done == false)
            {
              show_popup("Information", "P1 Calib Required");
              break;
            }
            status = cali_point(type, channel, CALI_POINT_2, &p2);
            if (status != MENU_OK)
              break;
            calib_updated = true;
            break;
          case 2: // P1.ADC
            snprintf(buffer, sizeof(buffer), "ADC(P1.ref %f)", p1.reference_value);
            dec = p1.raw_value;
            status = input_decimal(buffer, g_adc_config_ads1220.bits->min_raw_value, g_adc_config_ads1220.bits->max_raw_value,
                                   &dec);
            if (status != MENU_OK)
              break;
            p1.raw_value = dec;

            calib_updated = true;

            break;
          case 3: // P2.ADC
                  // 기존 P1,P2에서 P2 값만 변경한다.
            snprintf(buffer, sizeof(buffer), "ADC(P2.ref %f)", p2.reference_value);
            dec = p2.raw_value;
            status = input_decimal(buffer, g_adc_config_ads1220.bits->min_raw_value, g_adc_config_ads1220.bits->max_raw_value,
                                   &dec);
            if (status != MENU_OK)
              break;
            p2.raw_value = dec;

            calib_updated = true;
            break;
          case 4://특정 채널만 0으로 초기화
          {
            int32_t   active = 0;
            calib_updated = false;
            status = input_active("Cail Init?", &active);
            if(status != MENU_OK)
            break;
            if (active == 1)
            {
              memset(p_cal_params,0, sizeof(adc_cal_params_t));
              p_cal_params->comp_method = TEMP_COMP_NONE;
              p_cal_params->factory_cal_temp = 25;
              p1.raw_value = 0;
              p1.reference_value =0;
              p2.raw_value = 0;
              p2.reference_value =0;
              save_adc_cali();
              show_popup("Information","Init Ok");
            }
          }
            break;
          }

          if (status == MENU_ABORT)
          {
            return status;
          }


          if (calib_updated)
          {
            adc_perform_factory_calibration(p_cal_params, p1, p2, 25);
            save_adc_cali();
            len = 0;
            len = make_sreen_row(&buffer[len], "Slope:%e", p_cal_params->factory_slope);
            len += make_sreen_row(&buffer[len], "Offset:%e", p_cal_params->factory_offset);
            len = make_sreen_row(&buffer[len], "Success");
            show_popup("Information", buffer);
          }
        }
      }
      else if (key != KEY_CODE_UNKNOWN)
      {
        screen_menu_handle(&menu, key);
      }
}
  return convert_key_to_status(key);
}




int32_t cali_setup_menu_factory_calibration(adc_channel_type_t type)
{
  uint8_t err;
  int32_t adc_raw;
  int32_t avg_cnt;
  int32_t channel;
  int32_t choice = 0;
  int32_t status;
  int32_t key;
  float avg;
  float cal_temp;
  adc_cal_params_t* p_cal_params;
  adc_cal_point_t p1;
  adc_cal_point_t p2;
  config_adc_adv_t* p_adc;

  p_adc = &g_adc_config_ads1220;

  if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
  {
    status = input_combobox("Select SE Ch", adc_se_list, _countof(adc_se_list), &channel);
  }
  else
  {
    status = input_combobox("Select DIFF Ch", adc_diff_list, _countof(adc_diff_list), &channel);
  }

  if (status != MENU_OK)
    return status;

  p_cal_params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
                       ? &p_adc->single_ended_cal[channel]
                       : &p_adc->differential_cal[channel];



  choice = 0;
  status = input_active("Start cali P1?", &choice);
  if (status != MENU_OK || choice == 0)
    return status;

  avg = 0;
  avg_cnt = 0;


  while (1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      adc_raw = (int32_t)drv_adc_single_raw_read(channel, 1,&err);
    }
    else
    {
      adc_raw = (int32_t)drv_adc_diff_raw_read(channel,1, &err);
    }
    
    adc_raw = adc_raw;

    avg_cnt++;
    avg = recursive_avg_i(avg, adc_raw, avg_cnt);

    screen_clear();
    screen_printf(0, 0, "RAW:%d", adc_raw);
    screen_printf(1, 0, "AVG:%.0f", avg);
    screen_printf(2, 0, "Press enter");
    screen_refresh();

    key = get_menu_key(500);
    if (key == KEY_CODE_CTRL_C || key == KEY_CODE_CTRL_Q)
    return convert_key_to_status(key);
    else if(key == KEY_CODE_ENTER)
      break;
  }

  screen_clear();
  p1.raw_value = (int32_t)avg;
  status = input_decimal("Low RAW", p_adc->bits->min_raw_value, 
                        p_adc->bits->max_raw_value, (int32_t*)&p1.raw_value);
  if (status != MENU_OK)
    return status;

  if (channel < DRV_ADS1220_S_CH_16)
    p1.reference_value = 0.01;
  else
   p1.reference_value = 84.27;
  status = input_float("Low Value(V)", -1000.0f, 1000.0f, &p1.reference_value, "%8.3f");
  if (status != MENU_OK)
    return status;

  screen_clear();
  choice = 0;
  status = input_active("Start cali P2?",  &choice);
  if (status != MENU_OK || choice == 0)
    return status;

  avg = 0;
  avg_cnt = 0;

  screen_clear();

  while (1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      adc_raw = (int32_t)drv_adc_single_raw_read(channel, 1,&err);
    }
    else
    {
      adc_raw = (int32_t)drv_adc_diff_raw_read(channel,1, &err);
    }

    avg_cnt++;
    avg = recursive_avg_i(avg, adc_raw, avg_cnt);

    screen_clear();
    screen_printf(0, 0, "RAW:%d", adc_raw);
    screen_printf(1, 0, "AVG:%.0f", avg);
    screen_printf(2, 0, "Press enter");
    screen_refresh();

    key = get_menu_key(500);
    if (key == KEY_CODE_CTRL_C || key == KEY_CODE_CTRL_Q)
      return convert_key_to_status(key);
    else if (key == KEY_CODE_ENTER)
      break;
  }


  p2.raw_value = (int32_t)avg;
  status = input_decimal("High RAW", p_adc->bits->min_raw_value,p_adc->bits->max_raw_value, (int32_t*)&p2.raw_value);
  if (status != MENU_OK)
    return status;

    if (channel < DRV_ADS1220_S_CH_16)
    p2.reference_value = 4.99;
    else
    p2.reference_value = 123.24;

  status = input_float("High Value(V)", -1000.0f, 1000.0f, &p2.reference_value, "%8.3f");
  if (status != MENU_OK)
    return status;

  cal_temp = 25.0f;

  if (adc_perform_factory_calibration( p_cal_params, p1, p2, cal_temp))
  {
    save_adc_cali();
    show_popup("Information", "Success");
  }
  else
  {
    show_popup("Information", "Failed");
  }

  return status;
}

#define ADC_SE_CHANNEL_COUNT 16

void draw_adc_all(screen_page_t *p_win, adc_channel_type_t type,float average[16],uint32_t avg_cnt[16])
{
  char buff[SCREEN_COLS + 1];
  uint8_t err;
  int32_t raw;

  screen_page_start(p_win);

  make_centered(buff, sizeof(buff), "Ch   ADC     AVG", SCREEN_COLS);
  screen_page_printf(p_win, "%s", buff);

  for (int32_t channel = 0; channel < ADC_SE_CHANNEL_COUNT; channel++)
  { //SE 00:1234567 1234567
    raw = (int32_t)drv_adc_single_raw_read(channel, 1, &err);
    avg_cnt[channel]++;
    average[channel] = recursive_avg_i(average[channel], raw, avg_cnt[channel]);
    screen_page_printf(p_win, "%-4s:%7d %7d", adc_single_list[channel], raw, (int32_t)average[channel]);
  }

  screen_page_clear(p_win);
}


int32_t cali_single_all(adc_channel_type_t type)
{
  adc_cal_params_t *p_cal_params;
  adc_cal_point_t p1;
  adc_cal_point_t p2;
  config_adc_adv_t *p_adc;

  screen_page_t lcd_win;
  int32_t choice;
  int32_t status;
  float p1_average[ADC_SE_CHANNEL_COUNT];
  float p2_average[ADC_SE_CHANNEL_COUNT];
  uint32_t average_count[ADC_SE_CHANNEL_COUNT];
  int32_t key;
  float input_votage;
  bool cali_p1_done = false;
  bool cali_p2_done = false;
  const char *point_list[2] = {"Point 1(low)", "Point 2(high)"};
  p_adc = &g_adc_config_ads1220;
  choice = 0;
  status = input_combobox("Select cal point",point_list,_countof(point_list),&choice);
  if(status != MENU_OK)
  return status;

  if(choice == 1)//point 2만 하고 싶다면
  {
    goto CALI_POINT2;
  }


  choice = 0;
  status = input_active("Start cali P1?", &choice);
  if (status != MENU_OK || choice == 0)
    return status;

  show_popup("Information", "Connet P1");

  for (int i = 0; i < ADC_SE_CHANNEL_COUNT; i++)
  {
    p1_average[i] = 0;
    average_count[i] = 0;
  }

  screen_page_create(&lcd_win);

  lcd_win.total_pages = 1;
  lcd_win.chunk_scroll_enable = 1;

  while (1)
  {
    draw_adc_all(&lcd_win, ADC_CHANNEL_TYPE_SINGLE_ENDED, p1_average, average_count);
    screen_refresh();

    key = get_menu_key(10);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if (key == KEY_CODE_ENTER)
    {
      cali_p1_done = true;
      break;
    }
    if (key != KEY_CODE_UNKNOWN)
    {
      screen_page_handle(&lcd_win, key);
    }
  }

  if (cali_p1_done == false)
  {
    return convert_key_to_status(key);
  }
  p1.reference_value = 0.5;
  status = input_float("Low Value(V)", -1000.0f, 1000.0f, &p1.reference_value, "%8.3f");
  if (status != MENU_OK)
    return status;

CALI_POINT2:

  choice = 0;
  status = input_active("Start cali P2?", &choice);
  if (status != MENU_OK || choice == 0)
  {

    for (int channel = 0; channel < ADC_SE_CHANNEL_COUNT; channel++)
    {
      p_cal_params = &p_adc->single_ended_cal[channel];
      if (cali_p2_done == false) // p1을 안하고 p2만 새롭게 한 경우
      {
        p2 = p_cal_params->p2_cal_point;
      }
      p1.raw_value = (int32_t)p1_average[channel];
      adc_perform_factory_calibration( p_cal_params, p1, p2, 25.0f);
    }

    return status;

  }



  show_popup("Information", "Connet P2");

  for (int i = 0; i < ADC_SE_CHANNEL_COUNT; i++)
  {
    p2_average[i] = 0;
    average_count[i] = 0;
  }

  screen_page_create(&lcd_win);

  lcd_win.total_pages = 1;
  lcd_win.chunk_scroll_enable = 1;

  while (1)
  {
    draw_adc_all(&lcd_win, ADC_CHANNEL_TYPE_SINGLE_ENDED, p2_average, average_count);
    screen_refresh();

    key = get_menu_key(100);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if (key == KEY_CODE_ENTER)
    {
      cali_p2_done = true;
      break;
    }
    if (key != KEY_CODE_UNKNOWN)
    {
      screen_page_handle(&lcd_win, key);
    }
  }

  if (cali_p2_done == false)
  {
    return convert_key_to_status(key);
  }


  p2.reference_value = 4.5;
  status = input_float("High Value(V)", -1000.0f, 1000.0f, &p2.reference_value, "%8.3f");
  if (status != MENU_OK)
    return status;

  for (int channel = 0; channel < ADC_SE_CHANNEL_COUNT; channel++)
  {
    p_cal_params =  &p_adc->single_ended_cal[channel];
    if(cali_p1_done==false)//p1을 안하고 p2만 새롭게 한 경우
    {
      p1 = p_cal_params->p1_cal_point;
    }
    else
    {
      p1.raw_value = (int32_t)p1_average[channel];
    }
    p2.raw_value = (int32_t)p2_average[channel];
    adc_perform_factory_calibration( p_cal_params, p1, p2, 25.0f);
  }

  save_adc_cali();

  return MENU_OK;
}



int32_t cali_setup_menu_factory(void)
{
  int32_t index;
  int32_t key;
  int32_t status;
  uint8_t cali_single_all_active_count=0;
  screen_menu_t menu;

  screen_menu_create(&menu,  "Factory Calibration");

  while (1)
  {
    draw_cali_setup_menu_factory_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if(key == KEY_CODE_RIGHT)
    {
      if (menu.selected_index == FACTORY_MENU_DIFF)
      {
        cali_single_all_active_count++;
      }
      if(cali_single_all_active_count == 5)
      {
        g_cali_single_all_active = true;
      }
    }

    if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case FACTORY_MENU_SINGLE:
        status = setup_factory_calibration(ADC_CHANNEL_TYPE_SINGLE_ENDED);
         break;
        case FACTORY_MENU_DIFF:
          status = setup_factory_calibration(ADC_CHANNEL_TYPE_DIFFERENTIAL);
          break;
        case FACTORY_MENU_SINGLE_ALL:
          status = cali_single_all(ADC_CHANNEL_TYPE_SINGLE_ENDED);
        break;
          default:
          break;
      }
      
      if(status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t view_single_channel_details(adc_channel_type_t type,int32_t channel)
{
  const adc_cal_params_t *params;
  uint8_t err;
  int32_t raw;
  int32_t key;
  uint32_t start_time, elapsed_time;
  float voltage;
  config_adc_adv_t *p_adc;

  p_adc = &g_adc_config_ads1220;

  params = (type == ADC_CHANNEL_TYPE_SINGLE_ENDED) ? &p_adc->single_ended_cal[channel] : &p_adc->differential_cal[channel];

  screen_clear();
  while (1)
  {
    start_time = mcu_get_clk();
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      raw = (int32_t)drv_adc_single_raw_read(channel, 1, &err);
    }
    else
    {
      raw = drv_adc_diff_raw_read(channel, 1, &err);
    }

    if (params->is_calibrated == false)
    { //SE 01 RAW:1234567
      //DIFF 01 RAW:1234567  
      if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        screen_printf(0, 0, "%s RAW:%d", adc_se_list[channel], raw);
        else
          screen_printf(0, 0, "DIFF %2d RAW:%d", channel, raw);
      screen_printf(1, 0, "Calib Required");
      screen_refresh();
    }
    else
    {

      elapsed_time = cal_elapsed_us(start_time);

      g_current_temp = read_current_temperature();
      voltage = adc_get_compensated_value(raw, params, g_current_temp);

      if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        screen_printf(0, 0, "%s RAW:%d", adc_se_list[channel], raw);
      else
        screen_printf(0, 0, "DIFF %2d RAW:%d", channel, raw);
        
      screen_printf(1, 0, "elapsed:%5.2fms", (float)elapsed_time / 1000.0f);
      if (isnan(voltage))
      {
        screen_printf(2, 0, "Need Cal");
      }
      else
      {
        if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
        {
          if (channel < DRV_ADS1220_S_CH_16)
          {
            screen_printf(2, 0, "Voltage:%.4fV", voltage);
          }
          else
          {
            screen_printf(2, 0, "OHM:%.3fR", voltage);
          }
        }
        else
        {
          screen_printf(2, 0, "Voltage:%.4fV", voltage);
        }

      }
      screen_printf(3, 0, "slope :%e", params->factory_slope);
      screen_printf(4, 0, "offset:%e", params->factory_offset);
      screen_printf(5, 0, "P1:%7d,%6.4f", params->p1_cal_point.raw_value, params->p1_cal_point.reference_value);
      screen_printf(6, 0, "P2:%7d,%6.4f", params->p2_cal_point.raw_value, params->p2_cal_point.reference_value);
      screen_refresh();
    }
    key = get_menu_key(100);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
      break;
  }

  return convert_key_to_status(key);
}

int32_t cali_setup_menu_view_channel(adc_channel_type_t type)
{
  int32_t channel=0;
  int32_t status;

  while(1)
  {
    if (type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      status = input_combobox("SE Channel", adc_se_list, _countof(adc_se_list), &channel);
    }
    else
    {
      status = input_combobox("DIFF Channel", adc_diff_list, _countof(adc_diff_list), &channel);
    }
    if(status != MENU_OK)
    break;

    status = view_single_channel_details(type,channel);
    if(status != MENU_OK);
    break;
  }

 return status;

}


void draw_cali_single_summary(screen_page_t* p_win)
{
  const char *adc_se_short_list[18] = {"S 0", "S 1", "S 2", "S 3", "S 4", "S 5", "S 6", "S 7",
                                       "S 8", "S 9", "S 10", "S 11", "S 12", "S 13", "S 14", "S 15", "PT A", "PT B"};
  const adc_cal_params_t *params;
  char buff[SCREEN_COLS + 1];
  uint8_t err;


  int32_t raw;
  float voltage;
  config_adc_adv_t* p_adc;


  p_adc = &g_adc_config_ads1220;

  screen_page_start(p_win);

  make_centered(buff, sizeof(buff), "SE Ch Voltage ADC", SCREEN_COLS);
  screen_page_printf(p_win, "%s", buff);

  g_current_temp = read_current_temperature();

  for (int32_t channel = 0; channel < 18; channel++)
  {
    params = &p_adc->single_ended_cal[channel];

    raw = (int32_t)drv_adc_single_raw_read(channel,1, &err);
    voltage = adc_get_compensated_value(raw, params, g_current_temp);
    if (params->is_calibrated == false)
    {
      screen_page_printf(p_win, "%-4s: No Cal %7d", adc_se_short_list[channel], raw);
      continue;
    }
    if (isnan(voltage))
    {
      screen_page_printf(p_win, "%-4s:NaN", adc_se_short_list[channel]);
    }
    else
    { //1234:11.1111 12345678
      screen_page_printf(p_win, "%-4s:%7.4f %7d", adc_se_short_list[channel], voltage, raw);
    }
  }

  screen_page_clear(p_win);
}


void draw_cali_diff_summary(screen_page_t *p_win)
{
  const adc_cal_params_t *params;
  char buff[SCREEN_COLS + 1];
  uint8_t err;

  int32_t raw;
  float voltage;
  config_adc_adv_t *p_adc;

  p_adc = &g_adc_config_ads1220;

  screen_page_start(p_win);

  make_centered(buff, sizeof(buff), "Diff Ch Voltage ADC", SCREEN_COLS);
  screen_page_printf(p_win, "%s", buff);

  g_current_temp = read_current_temperature();

  for (int32_t channel = 0; channel < 8; channel++)
  {
    params = &p_adc->differential_cal[channel];

    raw = (int32_t)drv_adc_diff_raw_read(channel, 1, &err);
    voltage = adc_get_compensated_value(raw, params, g_current_temp);

    if (params->is_calibrated == false)
    {
      screen_page_printf(p_win, "%-4s: No Cal %7d", adc_diff_list[channel], raw);
      continue;
    }

    if (isnan(voltage))
    {
      screen_page_printf(p_win, "%-4s:NaN", adc_diff_list[channel]);
    }
    else
    { //DIFF 0:11.1111 1234567
      screen_page_printf(p_win, "%-4s:%7.4f %7d", adc_diff_list[channel], voltage, raw);
    }
  }

  screen_page_clear(p_win);
}
void draw_cali_single_summary_default(screen_page_t *p_win)
{
  const char *adc_se_short_list[18] = {"S 0", "S 1", "S 2", "S 3", "S 4", "S 5", "S 6", "S 7",
                                       "S 8", "S 9", "S 10", "S 11", "S 12", "S 13", "S 14", "S 15", "PT A", "PT B"};
  const adc_cal_params_t *params;
  char buff[SCREEN_COLS + 1];
  uint8_t err;
  int32_t raw;

  float offset = 4.928633e-03f;
  float slope = 5.958932e-07f;
  float voltage;



  screen_page_start(p_win);

  make_centered(buff, sizeof(buff), "SE Ch Voltage ADC", SCREEN_COLS);
  screen_page_printf(p_win, "%s", buff);

  g_current_temp = read_current_temperature();

  for (int32_t channel = 0; channel < 18; channel++)
  {
    raw = (int32_t)drv_adc_single_raw_read(channel, 1, &err);
    voltage = raw * slope + offset;;

    if (isnan(voltage))
    {
      screen_page_printf(p_win, "%-4s:NaN", adc_se_short_list[channel]);
    }
    else
    { // 1234:11.1111 12345678
      screen_page_printf(p_win, "%-4s:%7.4f %7d", adc_se_short_list[channel], voltage, raw);
    }
  }

  screen_page_clear(p_win);
}

int32_t handle_cali_view_summary(adc_channel_type_t type)
{
  int32_t key;
  screen_page_t lcd_win;
  
  screen_page_create(&lcd_win);
    
  lcd_win.total_pages =1;
  lcd_win.chunk_scroll_enable = 1;
  
  while (1)
  {
    if(type == ADC_CHANNEL_TYPE_SINGLE_ENDED)
    {
      if(g_summary_default_en)
      {
        draw_cali_single_summary_default(&lcd_win);
      }
      else
      {
        draw_cali_single_summary(&lcd_win);
      }
    }
    else
    {
      draw_cali_diff_summary(&lcd_win);
    }

    screen_refresh();

    key = get_menu_key(500);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }

    if (key != KEY_CODE_UNKNOWN)
    {
      screen_page_handle(&lcd_win, key);
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
  uint8_t cali_summary_default_count=0;

  screen_menu_create(&menu, "View Summary");

  while (1)
  {
    draw_cali_setup_menu_view_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
    {
      break;
    }
    else if(key == KEY_CODE_RIGHT)
    {
      if (menu.selected_index == VIEW_ADC_DIFF_SUMMARY)
      {
        cali_summary_default_count++;
      }
      if (cali_summary_default_count>5)
      g_summary_default_en = true;
    }
    else if (key == KEY_CODE_ENTER)
    {
      index = menu.selected_index;

      switch (menu.index_list[index])
      {
        case VIEW_ADC_SINGLE_SUMMARY:
          status = handle_cali_view_summary(ADC_CHANNEL_TYPE_SINGLE_ENDED);
          break;
        case VIEW_ADC_DIFF_SUMMARY:
          status = handle_cali_view_summary(ADC_CHANNEL_TYPE_DIFFERENTIAL);
          break;
        case VIEW_MENU_SUMMARY_DEFAULT:
          status = handle_cali_view_summary(ADC_CHANNEL_TYPE_SINGLE_ENDED);
          break;
        default:
          break;
      }
      
      if(status == MENU_ABORT)
        return status;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

/**
 * @brief ADC 켈리브레이션 값의 평균값으로 초기화 한다.
 */
int32_t cali_setup_menu_system_adc_init(void)
{
  int32_t choice = 0;
  int32_t status;

  
  status = input_active("Init Calibration?", &choice);

  if(status != MENU_OK)
  return status;

  if(choice == 0)
  return MENU_OK;



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
    screen_clear();
    show_popup("Information", "Init Complete");

  return MENU_OK;
}

//ADC 
int32_t cali_setup_menu_adc_init(void)
{
  int32_t choice = 0;
  int32_t status;

  status = input_active("Init Calibration?", &choice);

  if (status != MENU_OK)
    return status;

  if (choice == 0)
    return MENU_OK;

  adc_config_init(&g_adc_config_ads1220, 24, 5.0f);

  for (int32_t channel = 0; channel < g_adc_config_ads1220.params_se_cnt; channel++)
  {
    g_adc_config_ads1220.single_ended_cal[channel].comp_method = TEMP_COMP_NONE;
    g_adc_config_ads1220.single_ended_cal[channel].factory_cal_temp = 25.0f;
    g_adc_config_ads1220.single_ended_cal[channel].is_calibrated = false;
    g_adc_config_ads1220.single_ended_cal[channel].factory_offset = 4.928633e-03f;
    g_adc_config_ads1220.single_ended_cal[channel].factory_slope = 5.958932e-07f;
    g_adc_config_ads1220.single_ended_cal[channel].offset_temp_coeff = 1.0f;
    g_adc_config_ads1220.single_ended_cal[channel].slope_temp_coeff = 1.0f;
  }

  for (int32_t channel = 0; channel < g_adc_config_ads1220.params_di_cnt; channel++)
  {
    g_adc_config_ads1220.differential_cal[channel].comp_method = TEMP_COMP_NONE;
    g_adc_config_ads1220.differential_cal[channel].factory_cal_temp = 25.0f;
    g_adc_config_ads1220.differential_cal[channel].is_calibrated = false;
    g_adc_config_ads1220.differential_cal[channel].factory_offset = 4.928633e-03f;
    g_adc_config_ads1220.differential_cal[channel].factory_slope = 5.958932e-07f;
    g_adc_config_ads1220.differential_cal[channel].factory_offset_trim = 0.0f;
    g_adc_config_ads1220.differential_cal[channel].offset_temp_coeff = 1.0f;
    g_adc_config_ads1220.differential_cal[channel].slope_temp_coeff = 1.0f;
  }

  save_adc_cali();
  screen_clear();
  show_popup("Information", "Init Complete");

  return MENU_OK;
}

int32_t setup_menu_calibration(void)
{
  int32_t index;
  int32_t key;
  int32_t status = MENU_BACK;
  screen_menu_t menu;



  screen_menu_create(&menu, "Calibraion");

  while (1)
  {
    draw_setup_menu_calibration_page(&menu);
    screen_refresh();

    key = get_menu_key(WAIT_FOREVER);

    if (key == KEY_CODE_CTRL_Q || key == KEY_CODE_CTRL_C)
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
        case CALI_MENU_ADC_INIT:
          status = cali_setup_menu_adc_init();
          break;
         default:
          break;
      }
      if(status == MENU_ABORT)
      return  status;
    }
    else if (key != KEY_CODE_UNKNOWN)
    {
      screen_menu_handle(&menu, key);
    }
  }


  return convert_key_to_status(key);
}