

#include "app_key.h"
#include "app_screen.h"
#include "bsp_rtc.h"
#include "cli_key_code.h"
#include "config_app.h"
#include "console_utile.h"
#include "menu_handler.h"
#include "menu_system.h"
#include "util_time.h"
#include "view_driver.h"
#include "drv_rs232.h"
#include "drv_rs485.h"
#include "drv_freqInput.h"
#include "config_sensor.h"
#include "util_memory.h"

#include "app_sensor.h"
#include "const_string.h"
#include "menu/devices/hj_temperature_menu.h"
#include "menu/devices/hj_snowfall_menu.h"
#include "menu/devices/hj_wind_speed.h"
#include "Sensors\general\general_adc.h"

#define SCREEN_COLS 20
#define SYSTEM_WD 8




#define SYSTEM_MENU_TEMP 0
#define SYSTEM_MENU_WIND_SPEED 1

extern int32_t setup_sensor_set(sensor_t* p_sensor, uint8_t choice);
extern uint16_t get_sensor_model_list(const char **model_list, const uint8_t *idxList, uint8_t listCnt);
extern void set_type(sensor_t* sensor);
extern uint16_t get_sensor_model_eng_list(const char **model_list, const uint8_t *idxList, uint8_t listCnt);
extern const char* adcChModeList[2];
extern const char* physical_list[2];


#define ADC_L_W 10
#define E_L_W 8
#define TYPE_LABEL_W 8

#define ADC_PAGE_MODE 0
#define ADC_PAGE_CHANNEL 1
#define ADC_PAGE_HIGH_VALUE 2
#define ADC_PAGE_LOW_VALUE 3
#define ADC_PAGE_SCALE 4
#define ADC_PAGE_MAX_MV 5
#define ADC_PAGE_MIN_MV 6



void draw_adc_page(screen_menu_t* p_win,adc_config_t *adc_config)
{

  screen_menu_printf(p_win, ADC_PAGE_MODE, "%-*s:%s",ADC_L_W, "Adc Mode",ITEM_LIST(adc_config->mode, adcChModeList));

  if(adc_config->mode == ADC_CFG_MODE_SE)
    screen_menu_printf(p_win, ADC_PAGE_CHANNEL, "%-*s:%d", ADC_L_W, "Channel", adc_config->single_channel);
  else
    screen_menu_printf(p_win, ADC_PAGE_CHANNEL, "%-*s:%d", ADC_L_W, "Channel", adc_config->diff_channel);

  screen_menu_printf(p_win, ADC_PAGE_HIGH_VALUE, "%-*s:%d", ADC_L_W, "High Value", adc_config->high_scale);
  screen_menu_printf(p_win, ADC_PAGE_LOW_VALUE, "%-*s:%d", ADC_L_W, "Low Value", adc_config->low_scale);
  screen_menu_printf(p_win, ADC_PAGE_SCALE, "%-*s:%d", ADC_L_W, "Scale", adc_config->scale);
  screen_menu_printf(p_win, ADC_PAGE_MAX_MV, "%-*s:%d", ADC_L_W, "Max mV", adc_config->out_max_mv);
  screen_menu_printf(p_win, ADC_PAGE_MIN_MV, "%-*s:%d", ADC_L_W, "Min mV", adc_config->out_min_mv);

}

const char *safe_name(const char **names,int name_count,int index)
{
  const char *str="unknown";

  if(index>=name_count)
  {
    return str;
  }

  if(names[index]==0)
  {
    return str;
  }

  return names[index];

}

#define HJSNOW_PAGE_PHYSICAL  0
#define HJSNOW_PAGE_PORT      1
#define HJSNOW_PAGE_DEFAULT_MENU 2
#define HJSNOW_PAGE_SNOW_MENU 3


void draw_hjsnow_page(screen_menu_t* p_win, snow_hj_config_t* hjsnow_config)
{
  const char *name_table[10];
  int list_cnt;
  uint8_t port_number;


  screen_menu_printf(p_win, HJSNOW_PAGE_PHYSICAL, "%-*s:%s", E_L_W, "Com Type", 
           ITEM_LIST(hjsnow_config->physical_layer, physical_list));

  if (hjsnow_config->physical_layer == ePHYSICAL_RS232)
  {
    list_cnt = rs232_get_port_name_list(name_table, _countof(name_table));
    port_number = hjsnow_config->rs232_port;
  }
  else
  {
    list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));
    port_number = hjsnow_config->rs485_port;
  }

  screen_menu_printf(p_win, HJSNOW_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, port_number));
  screen_menu_printf(p_win, HJSNOW_PAGE_DEFAULT_MENU, "Default");
  screen_menu_printf(p_win, HJSNOW_PAGE_SNOW_MENU, "Settings");
  screen_menu_clear(p_win);
}

#define HJWIND_PAGE_FULLSET 0
#define HJWIND_PAGE_OFFSET 1
#define HJWIND_PAGE_PORT 2
#define HJWIND_PAGE_DEFAULT 3
void draw_hjwind_page(screen_menu_t* p_win, wind_speed_hj_pulse_config_t* hjwind_config)
{
  const char *name_table[10];
  int list_cnt;

  list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));


  screen_menu_printf(p_win, HJWIND_PAGE_FULLSET, "%-*s:%d", E_L_W, "Fullset", hjwind_config->full);
  screen_menu_printf(p_win, HJWIND_PAGE_OFFSET, "%-*s:%d", E_L_W, "Offset", hjwind_config->offset);
  screen_menu_printf(p_win, HJWIND_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, hjwind_config->rs485_port));
  screen_menu_printf(p_win, HJWIND_PAGE_DEFAULT, "%-*s", E_L_W, "Default");
}

#define HJWINDDIR_PAGE_PORT 0
#define HJWINDDIR_PAGE_DEFAULT 1

void draw_hjwindDir_page(screen_menu_t* p_win, wind_direction_hj_pulse_config_t* hjwindDir_config)
{
  const char *name_table[10];
  int list_cnt;

  list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));

  screen_menu_printf(p_win, HJWINDDIR_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, hjwindDir_config->rs485_port));
  screen_menu_printf(p_win, HJWINDDIR_PAGE_DEFAULT, "%-*s", E_L_W, "Default");
}

#define OTT_SMP3_PAGE_PORT 0
#define OTT_SMP3_PAGE_MODBUS_ID 1
#define OTT_SMP3_PAGE_DEFAULT 2
void draw_solar_radiation_ott_smp3_page(screen_menu_t* p_win, solar_r_ott_smp3_config_t* ott_smp3_config)
{
  const char *name_table[10];
  int list_cnt;

  list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));


  screen_menu_printf(p_win, OTT_SMP3_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, ott_smp3_config->rs485_port));
  screen_menu_printf(p_win, OTT_SMP3_PAGE_MODBUS_ID, "%-*s:%d", E_L_W, "MODBUS ID", ott_smp3_config->modbus_id);
  screen_menu_printf(p_win, OTT_SMP3_PAGE_DEFAULT, "Default");
}



#define RAIN_PRESENT_PAGE_DELAY 0
void draw_rain_present_page(screen_menu_t* p_win, rain_present_config_t* rain_present_config)
{
  screen_menu_printf(p_win, RAIN_PRESENT_PAGE_DELAY, "%-*s:%d", E_L_W, "Off Delay sec", rain_present_config->off_delay_sec);
}

#define FREQ_PAGE_CHANNEL 0
#define FREQ_PAGE_SCALE_FACTOR 1
void draw_freq_page(screen_menu_t* p_win, frequency_config_t* freq_config)
{

  screen_menu_printf(p_win, FREQ_PAGE_CHANNEL, "%-*s:%s", E_L_W, "Channel", ITEM_LIST(freq_config->channel, freq_ch_list));
  screen_menu_printf(p_win, FREQ_PAGE_SCALE_FACTOR, "%-*s:%.4f", E_L_W, "Scale Factor", freq_config->scale_factor);

}

#define WIND_SPD_RMYOUNG_05103V_CHANNEL 0
#define WIND_SPD_RMYOUNG_05103V_DEFAULT 1
void draw_wind_speed_rmyoung_05103V_page(screen_menu_t *p_win, wind_speed_rmyoung_05103v_config_t *freq_config)
{
  screen_menu_printf(p_win, WIND_SPD_RMYOUNG_05103V_CHANNEL, "%-*s:%s", E_L_W, "Channel", ITEM_LIST(freq_config->frequency_channel, freq_ch_list));
  screen_menu_printf(p_win, WIND_SPD_RMYOUNG_05103V_DEFAULT, "Default");
}


#define HJTEMP_PAGE_PHYSICAL  0
#define HJTEMP_PAGE_PORT      1
#define HJTEMP_PAGE_MODBUS_ID 2
#define HJTEMP_PAGE_DEFAULT   3
#define HJTEMP_PAGE_TEMP_MENU 4

void draw_hjtemp_page(screen_menu_t* p_win, temp_hj_config_t* hjtemp_config)
{
  const char *name_table[10];
  int list_cnt;
  uint8_t port_number;

  screen_menu_printf(p_win, HJTEMP_PAGE_PHYSICAL, "%-*s:%s", E_L_W, "Com Type",
                         ITEM_LIST(hjtemp_config->physical_layer, physical_list));

  if (hjtemp_config->physical_layer == ePHYSICAL_RS232)
  {
    list_cnt = rs232_get_port_name_list(name_table, _countof(name_table));
    port_number = hjtemp_config->rs232_port;
  }
  else
  {
    list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));
    port_number = hjtemp_config->rs485_port;
  }

  screen_menu_printf(p_win, HJTEMP_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, port_number));
  screen_menu_printf(p_win, HJTEMP_PAGE_MODBUS_ID, "%-*s:%d", E_L_W, "M bus ID", hjtemp_config->modbus_id);
  screen_menu_printf(p_win, HJTEMP_PAGE_DEFAULT, "Default");
  screen_menu_printf(p_win, HJTEMP_PAGE_TEMP_MENU, "Settings");

}

#define JINSUNG_BARO_PAGE_PORT 0
void draw_barometer_jinsung_page(screen_menu_t *p_win, barometer_jinsung_sjgp215_config_t *jinsung_config)
{
  const char *name_table[10];
  int list_cnt;
  uint8_t port_number;

  list_cnt = rs232_get_port_name_list(name_table, _countof(name_table));
  port_number = jinsung_config->rs232_port;
  screen_menu_printf(p_win, HJTEMP_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, port_number));
}


#define BAROMETER_RMYOUNG_61402V_CH  0
#define BAROMETER_RMYOUNG_61402V_DEFAULT 1
void draw_barometer_rmyoung_61402V_page(screen_menu_t *p_win, barometer_rmyoung_61402v_config_t *adc_config)
{
  screen_menu_printf(p_win, BAROMETER_RMYOUNG_61402V_CH, "%-*s:%d", E_L_W, "ADC CH", adc_config->adc_channel);
  screen_menu_printf(p_win, BAROMETER_RMYOUNG_61402V_DEFAULT, "Default");
}

#define WDIN_DIRECTION_RMYOUNG_05103V_CH 0
#define WDIN_DIRECTION_RMYOUNG_05103V_DEFAULT 1
void draw_wind_direction_rmyoung_05103V_page(screen_menu_t *p_win, wind_direction_rmyoung_05103v_config_t *adc_config)
{
  screen_menu_printf(p_win, WDIN_DIRECTION_RMYOUNG_05103V_CH, "%-*s:%d", E_L_W, "ADC CH", adc_config->adc_channel);
  screen_menu_printf(p_win, WDIN_DIRECTION_RMYOUNG_05103V_DEFAULT, "Default");
}




#define SOLAR_DURATION_CSD3_CH 0
#define SOLAR_DURATION_CSD3_DEFAULT 1
void draw_solar_duration_csd3_page(screen_menu_t *p_win, solar_duration_csd3_t *p_csd3)
{
  screen_menu_printf(p_win, SOLAR_DURATION_CSD3_CH, "%-*s:%d", E_L_W, "ADC CH", p_csd3->adc_channel);
  screen_menu_printf(p_win, SOLAR_DURATION_CSD3_DEFAULT, "Default");
}




#define HJ_WIND_SPD_MODBUS_PAGE_PORT 0
#define HJ_WIND_SPD_MODBUS_PAGE_ID 1
#define HJ_WIND_SPD_MODBUS_PAGE_DEFAULT 2
#define HJ_WIND_SPD_MODBUS_PAGE_SETTINGS 3
void draw_wind_speed_hj_modbus_page(screen_menu_t* p_win, wind_speed_hj_config_t* p_wind)
{
  const char *name_table[10];
  int list_cnt;

  list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));

  screen_menu_printf(p_win, HJ_WIND_SPD_MODBUS_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, p_wind->rs485_port));
  screen_menu_printf(p_win, HJ_WIND_SPD_MODBUS_PAGE_ID, "%-*s:%d", E_L_W, "MODBUS ID", p_wind->modbus_id);
  screen_menu_printf(p_win, HJ_WIND_SPD_MODBUS_PAGE_DEFAULT, "Default");
  screen_menu_printf(p_win, HJ_WIND_SPD_MODBUS_PAGE_SETTINGS, "Settings");
}


#define HJ_WIND_DIR_MODBUS_PAGE_PORT 0
#define HJ_WIND_DIR_MODBUS_PAGE_ID 1
#define HJ_WIND_DIR_MODBUS_PAGE_DEFAULT 2
void draw_wind_dir_hj_modbus_page(screen_menu_t* p_win, wind_direction_hj_config_t* p_wind)
{
  const char *name_table[10];
  int list_cnt;

  list_cnt = rs485_get_port_name_list(name_table, _countof(name_table));

  screen_menu_printf(p_win, HJ_WIND_DIR_MODBUS_PAGE_PORT, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, p_wind->rs485_port));
  screen_menu_printf(p_win, HJ_WIND_DIR_MODBUS_PAGE_ID, "%-*s:%d", E_L_W, "MODBUS ID", p_wind->modbus_id);
  screen_menu_printf(p_win, HJ_WIND_DIR_MODBUS_PAGE_DEFAULT, "Default");
}





  void draw_sensor_page(screen_menu_t * p_win, sensor_t * p_sensor)
  {
    int32_t label_width = TYPE_LABEL_W;
    if (p_sensor->type == S_T_ADC)
    {
      label_width = ADC_L_W;
    }
    screen_menu_start(p_win);
    screen_menu_printf(p_win, 0, "%-*s:%s", label_width, "TYPE", g_sensor_model_eng_table[p_sensor->type]);
    switch (p_sensor->type)
    {
    case S_T_ADC:
      draw_adc_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_SNOW_HJ:
      draw_hjsnow_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_WIND_DIRECTION_HJ_485:
      draw_hjwindDir_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_WIND_SPEED_HJ_485:
      draw_hjwind_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_TEMPERATURE_HJ:
      draw_hjtemp_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_HUMINITY_HJ:
      draw_hjtemp_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
      draw_solar_radiation_ott_smp3_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_RAIN_PRESENT_DI:
    case S_T_RAIN_PRESENT_ANALOG:
      draw_rain_present_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_FREQ:
      draw_freq_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_BARO_JINSUNG_SJGP215:
      draw_barometer_jinsung_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_BARO_RMYOUNG_61402V:
      draw_barometer_rmyoung_61402V_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_WIND_DIRECTION_RMYOUNG_05103V:
      draw_wind_direction_rmyoung_05103V_page(p_win,get_sensor_config(p_sensor));
       break;
    case S_T_WIND_SPEED_RMYOUNG_05103V:
      draw_wind_speed_rmyoung_05103V_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_SOLAR_DURATION_CSD3:
    draw_solar_duration_csd3_page(p_win, get_sensor_config(p_sensor));
    break;
      case S_T_WIND_SPEED_HJ_MODBUS:
      draw_wind_speed_hj_modbus_page(p_win, get_sensor_config(p_sensor));
      break;
      case S_T_WIND_DIRECTION_HJ_MODBUS:
            draw_wind_dir_hj_modbus_page(p_win, get_sensor_config(p_sensor));
      break;
    

          default : break;
  }
  screen_menu_clear(p_win);
}

int32_t setup_select_menu_index(sensor_t *p_sensor, int *choice, eSENSOR_TYPE_t type)
{
  int32_t key;
  static  uint8_t selected_index=0; //이전 선택 행 유지
  static eSENSOR_TYPE_t sensor_type = (eSENSOR_TYPE_t)-1;
  screen_menu_t menu;

  screen_menu_create(&menu, sensor_name_eng_list[type]);

  if (type != sensor_type)
  {
    menu.selected_index = 0;
  }
  else
  {
    menu.selected_index = selected_index;
  }
  sensor_type = type;

  while (1)
  {
    draw_sensor_page(&menu,p_sensor);
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
      *choice = menu.selected_index;
      selected_index = menu.selected_index;
      return MENU_OK;
    }
    else if (key != KEY_CODE_NONE)
    {
      selected_index=0;
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}

int32_t find_index_sensor_type(const uint8_t* idxList, int8_t list_cnt, eSENSOR_TYPE_MODEL_t type)
{
  for(int i = 0 ; i < list_cnt;i++)
  {
    if(idxList[i]==type)
    {
      return i;
    }
  }

  return 0;
}

int32_t setup_sensor_model_set(sensor_t* sensor, const uint8_t* model_list, uint8_t list_cnt)
{
  uint8_t model_list_count;
  int32_t status = 0;
  int32_t choice;
  const char* model_list_string[10];

  model_list_count = get_sensor_model_eng_list(model_list_string, model_list, list_cnt);

  choice = find_index_sensor_type(model_list,list_cnt,sensor->type);

  status = input_combobox("Sensor Model",model_list_string,  model_list_count, &choice);

  if (status != MENU_OK)
  {
    return status;
  }
  sensor->type = (eSENSOR_TYPE_MODEL_t)model_list[choice];
  set_type(sensor);

  return MENU_OK;
}

typedef struct
{
  uint8_t sensor_type;
  int32_t (*config_set)(sensor_t*, uint8_t);
} sensor_setup_entry_t;






int32_t general_adc_setup( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  int32_t dec;
  adc_config_t* adc;

  adc = get_sensor_config(sensor);
  if (adc == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case ADC_PAGE_MODE:
      choice = adc->mode;
      status = input_combobox("ADC Mode",adcChModeList, _countof(adcChModeList), &choice);
      if (status != MENU_OK)
        break;
      adc->mode = choice;
      save_config_sensor();
      break;
    case ADC_PAGE_CHANNEL:
     if (adc->mode == ADC_CFG_MODE_SE)
      {
          choice = adc->single_channel;
          status = input_combobox("SE Channel", g_adc_single_owner_list, _countof(g_adc_single_owner_list), &choice);
          if (status != MENU_OK)
            break;
          adc->single_channel = choice;
      }
      else
      {
        choice = adc->diff_channel;
        status = input_combobox("Diff Channel", g_adc_diff_owner_list, _countof(g_adc_diff_owner_list), &choice);
        if (status != MENU_OK)
          break;
        adc->diff_channel = choice;
      }

      save_config_sensor();
      break;
    case ADC_PAGE_HIGH_VALUE:
      dec = adc->high_scale;
      status = input_decimal("High Value", -1000000, 1000000, &dec);
      if (status != MENU_OK)
        break;
      adc->high_scale = dec;
      save_config_sensor();
      break;
    case ADC_PAGE_LOW_VALUE:
    dec = adc->low_scale;
      status = input_decimal("Low Value", -1000000, 1000000, &dec);
      if (status != MENU_OK)
        break;
      adc->low_scale = dec;
      save_config_sensor();
      break;
    case ADC_PAGE_SCALE:
    dec = adc->scale;
      status = input_decimal("Scale", 0, 100, &dec);
      if (status != MENU_OK)
        break;
      adc->scale = dec;
      save_config_sensor();
      break;
    case ADC_PAGE_MAX_MV:
    dec = adc->out_max_mv;
      status = input_decimal("Max mV", 0, 5000, &dec);
      if (status != MENU_OK)
        break;
      adc->out_max_mv = dec;
      save_config_sensor();
      break;
    case ADC_PAGE_MIN_MV:
    dec = adc->out_min_mv;
      status = input_decimal("Min mV", 0, 5000, &dec);
      if (status != MENU_OK)
        break;
      adc->out_min_mv = dec;
      save_config_sensor();
      break;
  }

  return status;
}
int32_t general_freq_setup( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t dec;
  float factor;

  frequency_config_t* freq;

  freq = get_sensor_config(sensor);
  if (freq == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case FREQ_PAGE_CHANNEL:
      dec = freq->channel;
       status = input_combobox("Channel", g_freq_owner_list, _countof(g_freq_owner_list), &dec);
      if (status != MENU_OK)
        break;
      freq->channel = dec;
      save_config_sensor();
      break;
    case FREQ_PAGE_SCALE_FACTOR:
      factor = freq->scale_factor;
      status= input_float_adv("Scale Factor",0,0,&factor,"%6.4f");

      if (status != MENU_OK)
        break;
      freq->scale_factor = factor;
      save_config_sensor();
      break;
  }

  return status;
}


int32_t hjwinddir_setup( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  wind_direction_hj_pulse_config_t* hjwindDir;
  const char* portList[10];
  uint16_t portListCnt;

  hjwindDir = get_sensor_config(sensor);
  if (hjwindDir == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJWINDDIR_PAGE_PORT:
      portListCnt = drv_rs485_get_portList(portList, _countof(portList));
      choice = hjwindDir->rs485_port;
      status = input_combobox("RS485 Port",portList, portListCnt, &choice);
      if (status != MENU_OK)
        break;
      hjwindDir->rs485_port = choice;
      save_config_sensor();
      break;
    case HJWINDDIR_PAGE_DEFAULT:
    {
      choice = 0;
      status = input_active("Set as Default?", &choice);
      if (status != MENU_OK || choice == 0)
        break;

      hjwindDir->rs485_port = eAPP_RS485_C;
    }
    break;
  }

  return status;
}
int32_t hjwind_setup( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  int32_t dec;
  wind_speed_hj_pulse_config_t* hjwind;
  const char* portList[10];
  uint16_t portListCnt;

  hjwind = get_sensor_config(sensor);
  if (hjwind == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJWIND_PAGE_FULLSET:
      dec = hjwind->full;
      status = input_decimal("FULLSET", 0, 999999, &dec);
      if (status != MENU_OK)
        break;
      hjwind->full = dec;
      save_config_sensor();
      break;
    case HJWIND_PAGE_OFFSET:
    dec = hjwind->offset;
      status = input_decimal("OFFSET", 0, 999999, &dec);
      if (status != MENU_OK)
        break;
      hjwind->offset = dec;
      save_config_sensor();
      break;
    case HJWIND_PAGE_PORT:
      portListCnt = drv_rs485_get_portList(portList, _countof(portList));
      choice = hjwind->rs485_port;
      status = input_combobox("RS485 Port",portList, portListCnt, &choice);
      if (status != MENU_OK)
        break;
      hjwind->rs485_port = choice;
      save_config_sensor();
      break;
    case HJWIND_PAGE_DEFAULT:
    {
      choice = 0;
      status = input_active("Set as Default?", &choice);
      if (status != MENU_OK || choice == 0)
        break;
      hjwind->full = 3200;
      hjwind->offset = 0;
      hjwind->rs485_port = eAPP_RS485_C;
    }

    break;
  }

  return status;
}
int32_t hjsnow_setup( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  snow_hj_config_t* hjsnow;
  const char* portList[10];
  uint16_t portListCnt;

  hjsnow = get_sensor_config(sensor);
  if (hjsnow == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJSNOW_PAGE_PHYSICAL:
      choice = hjsnow->physical_layer;
      status = input_combobox("Port Type", physical_list, _countof(physical_list), &choice);
      if (status != MENU_OK)
        break;
      hjsnow->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
      save_config_sensor();
      break;
    case HJSNOW_PAGE_PORT:
      if (hjsnow->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        choice = hjsnow->rs232_port;
        status = input_combobox("RS232 Port",portList, portListCnt, &choice);
        if (status != MENU_OK)
          break;
        hjsnow->rs232_port = choice;
        save_config_sensor();
      }
      else
      {
        portListCnt = drv_rs485_get_portList(portList, _countof(portList));
        choice = hjsnow->rs485_port;
        status = input_combobox("RS485 Port",portList, portListCnt, &choice);
        if (status != MENU_OK)
          break;
        hjsnow->rs485_port = choice;
        save_config_sensor();
      }
      break;
    case HJSNOW_PAGE_DEFAULT_MENU:
      {
        choice = 0;
        status = input_active("Set as Default?",&choice);
        if (status != MENU_OK || choice == 0)
          break;
        hjsnow->physical_layer = ePHYSICAL_RS232;
        hjsnow->rs232_port = eRS232_C;
      }
      break;
     case HJSNOW_PAGE_SNOW_MENU:
      status = ctrl_hj_snow();
      break;
  }

  return status;
}

int32_t hjtemp_setup(sensor_t* sensor, uint8_t menu_index)

{
  int32_t status;
   int32_t choice;
  int32_t dec = 0;
  temp_hj_config_t* hjtemp;
  const char* portList[10];
  uint16_t portListCnt;

  hjtemp = get_sensor_config(sensor);
  if (hjtemp == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJTEMP_PAGE_PHYSICAL:
      choice = hjtemp->physical_layer;
      status = input_combobox("Port Type",physical_list, _countof(physical_list), &choice);
      if (status != MENU_OK)
        break;
      hjtemp->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
      save_config_sensor();
      break;
    case HJTEMP_PAGE_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        choice = hjtemp->rs232_port;
        status = input_combobox("RS232 Port",portList, portListCnt, &choice);

        if (status != MENU_OK)
          break;
        hjtemp->rs232_port = choice;
        save_config_sensor();
      }
      else
      {
        portListCnt = drv_rs485_get_portList(portList, _countof(portList));
        choice = hjtemp->rs485_port;
        status = input_combobox("RS485 Port",portList, portListCnt, &choice);
        if (status != MENU_OK)
          break;

        hjtemp->rs485_port = choice;
        save_config_sensor();

        break;
        case HJTEMP_PAGE_MODBUS_ID:
          dec = hjtemp->modbus_id;
                    status = input_decimal("ID", 0, 247, &dec);
          if (status != MENU_OK)
            break;

          hjtemp->modbus_id = dec;
          save_config_sensor();
          break;
          case HJTEMP_PAGE_DEFAULT:
          {
            choice = 0;
            status = input_active("Set as Default?", &choice);
            if (status != MENU_OK || choice == 0)
              break;
            hjtemp->physical_layer = ePHYSICAL_RS485;
            hjtemp->rs485_port = eAPP_RS485_RS232_B;
            hjtemp->modbus_id = 1;
                      save_config_sensor();
          }
          break;
        case HJTEMP_PAGE_TEMP_MENU:

            status = ctrl_hj_temp();

          break;
      }
  }

  return status;
}

int32_t hjhumi_setup(sensor_t* sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  int32_t dec = 0;
  humi_hj_config_t* hjhumi;
  const char* portList[10];
  uint16_t portListCnt;

  hjhumi = get_sensor_config(sensor);
  if (hjhumi == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJTEMP_PAGE_PHYSICAL:
      choice = hjhumi->physical_layer;
      status = input_combobox("Port Type",physical_list, _countof(physical_list), &choice);
      if (status != MENU_OK)
        break;
      hjhumi->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
      save_config_sensor();
      break;
    case HJTEMP_PAGE_PORT:
      if (hjhumi->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        choice = hjhumi->rs232_port;
        status = input_combobox("RS232 Port",portList, portListCnt, &choice);
        if (status != MENU_OK)
          break;
        hjhumi->rs232_port = choice;
        save_config_sensor();
      }
      else
      {
        portListCnt = drv_rs485_get_portList(portList, _countof(portList));
        choice = hjhumi->rs485_port;
        status = input_combobox("RS485 Port",portList, portListCnt, &choice);
        if (status != MENU_OK)
          break;
        hjhumi->rs485_port = choice;
        save_config_sensor();
      }
      break;
    case HJTEMP_PAGE_MODBUS_ID:
      dec = hjhumi->modbus_id;
      status = input_decimal("ID", 0, 247, &dec);
      if (status != MENU_OK)
        break;
      hjhumi->modbus_id = dec;
      save_config_sensor();
      break;
    case HJTEMP_PAGE_DEFAULT:
    {
      choice = 0;
      status = input_active("Set as Default?", &choice);
      if (status != MENU_OK || choice == 0)
        break;
      hjhumi->physical_layer = ePHYSICAL_RS485;
      hjhumi->rs485_port = eAPP_RS485_RS232_B;

      hjhumi->modbus_id = 1;
                save_config_sensor();
    }
    break;
    case HJTEMP_PAGE_TEMP_MENU:
       status = ctrl_hj_temp();
      break;
  }

  return status;
}
int32_t ott_smp3_setup(sensor_t* sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  int32_t dec;
  solar_r_ott_smp3_config_t* ott_smp3;
  const char* portList[10];
  uint16_t portListCnt;

  ott_smp3 = get_sensor_config(sensor);
  if (ott_smp3 == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case OTT_SMP3_PAGE_PORT:
      portListCnt = drv_rs485_get_portList(portList, _countof(portList));
      choice = ott_smp3->rs485_port;
      status = input_combobox("RS485 Port",portList, portListCnt, &choice);
      if (status != MENU_OK)
        break;
      ott_smp3->rs485_port = choice;
      save_config_sensor();
      break;
    case OTT_SMP3_PAGE_MODBUS_ID:
    dec = ott_smp3->modbus_id;
      status = input_decimal("MODBUS ID", 0, 255, &dec);
      if (status != MENU_OK)
        break;
      ott_smp3->modbus_id = dec;
      save_config_sensor();
      break;
    case OTT_SMP3_PAGE_DEFAULT:
      choice = 0;
      status = input_active("Set as Default?", &choice);
      if (status != MENU_OK || choice == 0)
        break;
      ott_smp3->rs485_port = eAPP_RS485_RS232_A;
      ott_smp3->modbus_id = 1;
      save_config_sensor();
      break;
    }

  return status;
}

int32_t rain_present_setup(sensor_t* sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t dec;
  rain_present_config_t* rain_present;

  rain_present = get_sensor_config(sensor);
  if (rain_present == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case RAIN_PRESENT_PAGE_DELAY:
      dec = rain_present->off_delay_sec;
      
       status = input_decimal("Off Delay Time(sec)", 1, 300, &dec);
      if (status != MENU_OK)
        break;
      rain_present->off_delay_sec = dec;
      save_config_sensor();
      break;
  }

  return status;
}

int32_t barometer_jinsung_setup(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;


  barometer_jinsung_sjgp215_config_t *jinsung_baro;
  const char *portList[10];
  uint16_t portListCnt;
  int choice;

  jinsung_baro = get_sensor_config(sensor);
  if (jinsung_baro == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
  case JINSUNG_BARO_PAGE_PORT:
    portListCnt = rs232_get_portList(portList, _countof(portList));
    choice = jinsung_baro->rs232_port;
    status = input_combobox("RS232 Port", portList, portListCnt, &choice);
    if (status != MENU_OK)
      break;
    jinsung_baro->rs232_port = choice;
    save_config_sensor();
    break;
  }

  return status;
}

int32_t barometer_rmyoun_61402V_setup(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  barometer_rmyoung_61402v_config_t *p_cfg;
  int choice;
  int active;

  p_cfg = get_sensor_config(sensor);
  if(p_cfg == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
  case BAROMETER_RMYOUNG_61402V_CH:
    choice = p_cfg->adc_channel;
    status = input_combobox("SE Channel", g_adc_single_owner_list, _countof(g_adc_single_owner_list), &choice);
    if (status != MENU_OK)
      break;
        p_cfg->adc_channel = choice;
        save_config_sensor();
    break;
  case BAROMETER_RMYOUNG_61402V_DEFAULT:
    choice = 0;
    status = input_active("Set as Default?", &choice);
    if (status != MENU_OK || choice == 0)
      break;
    p_cfg->adc_channel = 0;
    save_config_sensor();
    break;
  }

  return status;
}

int32_t wind_direction_rmyoung_05103V_setup(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  wind_direction_rmyoung_05103v_config_t *p_cfg;
  int active;
  int choice;

  p_cfg = get_sensor_config(sensor);
  if (p_cfg == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
  case WDIN_DIRECTION_RMYOUNG_05103V_CH:
    choice = p_cfg->adc_channel;
    status = input_combobox("SE Channel", g_adc_single_owner_list, _countof(g_adc_single_owner_list), &choice);
    if (status != MENU_OK)
      break;
        p_cfg->adc_channel = choice;
        save_config_sensor();

    break;
  case WDIN_DIRECTION_RMYOUNG_05103V_DEFAULT:
    choice = 0;
    status = input_active("Set as Default?", &choice);
    if (status != MENU_OK || choice == 0)
      break;
    p_cfg->adc_channel = 15;
    save_config_sensor();
    break;
  }


  return status;
}



int32_t wind_speed_rmyoung_05103V_setup(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t dec;
  int32_t choice;
  float factor;
  wind_speed_rmyoung_05103v_config_t *p_cfg;

  p_cfg = get_sensor_config(sensor);
  if (p_cfg == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
  case WIND_SPD_RMYOUNG_05103V_CHANNEL:
    dec = p_cfg->frequency_channel;
    status = input_combobox("Channel", freq_ch_list, _countof(freq_ch_list), &dec);
    if (status != MENU_OK)
      break;
    p_cfg->frequency_channel = dec;

    save_config_sensor();
    break;
  case WIND_SPD_RMYOUNG_05103V_DEFAULT:
    choice = 0;
    status = input_active("Set as Default?", &choice);
    if (status != MENU_OK || choice == 0)
      break;
    p_cfg->frequency_channel = 0;
    save_config_sensor();
    break;

  }

  return status;
}



int32_t solar_duration_csd3_setup(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  solar_duration_csd3_t *p_cfg;
  int active;
  int choice;

  p_cfg = get_sensor_config(sensor);
  if (p_cfg == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
  case SOLAR_DURATION_CSD3_CH:
    choice = p_cfg->adc_channel;
    status = input_combobox("SE Channel", g_adc_single_owner_list, _countof(g_adc_single_owner_list), &choice);
    if (status != MENU_OK)
      break;
        p_cfg->adc_channel = choice;
        save_config_sensor();

    break;
  case SOLAR_DURATION_CSD3_DEFAULT:
    choice = 0;
    status = input_active("Set as Default?", &choice);
    if (status != MENU_OK || choice == 0)
      break;
    p_cfg->adc_channel = 14;
    save_config_sensor();
    break;
  }


  return status;
}



int32_t wind_speed_hj_modbus_setup(sensor_t* sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  int32_t dec;
  wind_speed_hj_config_t* p_wind_speed;
  const char* portList[10];
  uint16_t portListCnt;

  p_wind_speed = get_sensor_config(sensor);
  if (p_wind_speed == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJ_WIND_SPD_MODBUS_PAGE_PORT:
      portListCnt = drv_rs485_get_portList(portList, _countof(portList));
      choice = p_wind_speed->rs485_port;
      status = input_combobox("RS485 Port",portList, portListCnt, &choice);
      if (status != MENU_OK)
        break;
      p_wind_speed->rs485_port = choice;
      save_config_sensor();
      break;
    case HJ_WIND_SPD_MODBUS_PAGE_ID:
    dec = p_wind_speed->modbus_id;
      status = input_decimal("MODBUS ID", 0, 255, &dec);
      if (status != MENU_OK)
        break;
      p_wind_speed->modbus_id = dec;
      save_config_sensor();
      break;
    case HJ_WIND_SPD_MODBUS_PAGE_DEFAULT:
      choice = 0;
      status = input_active("Set as Default?", &choice);
      if (status != MENU_OK || choice == 0)
        break;
      p_wind_speed->rs485_port = eAPP_RS485_C;
      p_wind_speed->modbus_id = 1;
      save_config_sensor();
      break;
    case HJ_WIND_SPD_MODBUS_PAGE_SETTINGS:
    status = ctrl_hj_wind_speed();
    break;
    }

  return status;
}


int32_t wind_direction_hj_modbus_setup(sensor_t* sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  int32_t dec;
  wind_direction_hj_config_t* p_wind_speed;
  const char* portList[10];
  uint16_t portListCnt;

  p_wind_speed = get_sensor_config(sensor);
  if (p_wind_speed == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJ_WIND_DIR_MODBUS_PAGE_PORT:
      portListCnt = drv_rs485_get_portList(portList, _countof(portList));
      choice = p_wind_speed->rs485_port;
      status = input_combobox("RS485 Port",portList, portListCnt, &choice);
      if (status != MENU_OK)
        break;
      p_wind_speed->rs485_port = choice;
      save_config_sensor();
      break;
    case HJ_WIND_DIR_MODBUS_PAGE_ID:
    dec = p_wind_speed->modbus_id;
      status = input_decimal("MODBUS ID", 0, 255, &dec);
      if (status != MENU_OK)
        break;
      p_wind_speed->modbus_id = dec;
      save_config_sensor();
      break;
    case HJ_WIND_DIR_MODBUS_PAGE_DEFAULT:
      choice = 0;
      status = input_active("Set as Default?", &choice);
      if (status != MENU_OK || choice == 0)
        break;
      p_wind_speed->rs485_port = eAPP_RS485_C;
      p_wind_speed->modbus_id = 2;
      save_config_sensor();
      break;
    }

  return status;
}


const sensor_setup_entry_t g_sensor_setup_table[] = {
    {.sensor_type = S_T_ADC, .config_set = general_adc_setup},
    {.sensor_type = S_T_FREQ, .config_set = general_freq_setup},
    {.sensor_type = S_T_WIND_DIRECTION_HJ_485, .config_set = hjwinddir_setup},
    {.sensor_type = S_T_WIND_SPEED_HJ_485, .config_set = hjwind_setup},
    {.sensor_type = S_T_SNOW_HJ, .config_set = hjsnow_setup},
    {.sensor_type = S_T_TEMPERATURE_HJ, .config_set = hjtemp_setup},
    {.sensor_type = S_T_HUMINITY_HJ, .config_set = hjhumi_setup},
    {.sensor_type = S_T_SOLAR_RADIATION_OTT_SMP3, .config_set = ott_smp3_setup},
    {.sensor_type = S_T_RAIN_PRESENT_DI, .config_set = rain_present_setup},
    {.sensor_type = S_T_RAIN_PRESENT_ANALOG, .config_set = rain_present_setup},
    {.sensor_type = S_T_BARO_JINSUNG_SJGP215, .config_set = barometer_jinsung_setup},
    {.sensor_type = S_T_BARO_RMYOUNG_61402V, .config_set = barometer_rmyoun_61402V_setup},
    {.sensor_type = S_T_WIND_DIRECTION_RMYOUNG_05103V, .config_set = wind_direction_rmyoung_05103V_setup},
    {.sensor_type = S_T_WIND_SPEED_RMYOUNG_05103V, .config_set = wind_speed_rmyoung_05103V_setup},
    {.sensor_type = S_T_SOLAR_DURATION_CSD3, .config_set = solar_duration_csd3_setup},
    {.sensor_type = S_T_WIND_SPEED_HJ_MODBUS, .config_set = wind_speed_hj_modbus_setup},
    {.sensor_type = S_T_WIND_DIRECTION_HJ_MODBUS, .config_set = wind_direction_hj_modbus_setup}};

int32_t setup_sensor_set(sensor_t* p_sensor, uint8_t choice)
{
  int32_t status = MENU_OK;

  for (int i = 0; i < _countof(g_sensor_setup_table); i++)
  {  // 센서마다 고유의 처리 함수를 사용한다.
    if (g_sensor_setup_table[i].sensor_type == p_sensor->type)
    {
      status = g_sensor_setup_table[i].config_set(p_sensor, choice - 1);
      break;
    }
  }

  return status;
}
int32_t setup_sensor(eSENSOR_TYPE_t list)
{
  int32_t status = 0;
  int32_t choice = 0;


  // 선택된 센서의 설정 정보를 가져온다.
  sensor_t* sensor = &get_config_app()->sensor[(int)list];
  do
  {
    /*
    센서의 현재 정보를 출력하고, 수정을 원하는 항목의 번호를 입력받는다.
    0.type       :화진 RS485 9600
    1.port        :EX1 RS485 A
    이런 화면이 나타남남
    */
    status = setup_select_menu_index(sensor, &choice,list);
    if (status != MENU_OK)
      break;

    switch (choice)
    {
      case 0:  // 센서가 사용하고자하는 센서 타입을 설정한다.
               // 센서마다 지원가능한 목록을 넘겨지고 출력하여 선택하도록 한다.
        status = setup_sensor_model_set(sensor, sensor_table[list].list, sensor_table[list].cnt);
        break;
      default:  // 센서 타입이 아닌 센서 고유 속성들은 이 함수 에서 처리한다.
        // 현재의 센서 정보와 사용자가 수정하고자한 항목 번호를 넘긴다.
        status = setup_sensor_set(sensor, choice);  // 센서별 설정값 변경
        break;
    }

    if (status == MENU_ABORT)
      break;
  } while (1);

  return status;
}

eSENSOR_TYPE_t supported_sensor_menu_num[SENSOR_LIST_MAX];

void draw_menu_sensor_page(screen_menu_t* p_win)
{
  sensor_t *p_sensor = get_config_app()->sensor;
  int menu_num=0;
  screen_menu_start(p_win);

  for(int i = 0;i< SENSOR_LIST_MAX;i++)
  {
    if(supported_sensors[i].supported==true)
    {
      screen_menu_printf(p_win, i, "%-15s%s", sensor_name_eng_list[i],p_sensor[i].type>0?"[E]":"[D]");
      supported_sensor_menu_num[menu_num++] =(eSENSOR_TYPE_t)i;
    }
  }

  screen_menu_clear(p_win);
}

int32_t setup_menu_sensor(void)
{
  int32_t key;
  int32_t status;
  
  eSENSOR_TYPE_t sensor_type;

  screen_menu_t menu;

  screen_menu_create(&menu, "Sensor");

  while (1)
  {
    draw_menu_sensor_page(&menu);
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
      screen_clear();
      sensor_type = supported_sensor_menu_num[menu.selected_index];
     status =  setup_sensor(sensor_type);
     if(status == MENU_ABORT)
       return MENU_ABORT;
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}
