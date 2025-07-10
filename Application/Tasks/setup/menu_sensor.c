

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
#include "app_rs232.h"
#include "app_rs485.h"
#include "config_sensor.h"
#include "util_memory.h"

#include "app_sensor.h"
#define SCREEN_COLS 20
#define SYSTEM_WD 8

#define M_PRINTF screen_menu_printf_row



#define SYSTEM_MENU_TEMP 0
#define SYSTEM_MENU_WIND_SPEED 1

extern int32_t setup_sensor_set(sensor_t* p_sensor, uint8_t choice);
extern uint16_t get_sensor_model_list(const char **model_list, const uint8_t *idxList, uint8_t listCnt);
extern void set_type(sensor_t* sensor);
extern uint16_t get_sensor_model_eng_list(const char **model_list, const uint8_t *idxList, uint8_t listCnt);
extern const char* adcChModeList[2];
extern const char* physical_list[2];


#define E_L_W 8


#define ADC_PAGE_MODE 0



void draw_adc_page(screen_menu_t* p_win,adc_config_t *adc_config)
{
  int row_count = 1;


  screen_update_list(p_win, row_count, ADC_PAGE_MODE);
  M_PRINTF(p_win, row_count++, "%-*s:%s",E_L_W, "Adc Mode",ITEM_LIST(adc_config->mode, (char*)adcChModeList));
  M_PRINTF(p_win, row_count++, "%-*s:%s",E_L_W,  "Adc Mode", "%s",  ITEM_LIST(adc_config->mode, (char *)adcChModeList));
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "Channel", "%d", adc_config->channel);
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "High Value", "%d", adc_config->highScale);
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "Low Value", "%d", adc_config->lowScale);
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "Scale", "%d", adc_config->scale);
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "Max mV", "%d", adc_config->outMaxV);
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "Min mV", "%d", adc_config->outMinV);

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
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

#define HJTEMP_PAGE_PHYSICAL 0
#define HJTEMP_PAGE_PORT 1
#define HJTEMP_PAGE_MODBUS_ID 2
#define HJTEMP_PAGE_TEMP_MENU 3
void draw_hjtemp_page(screen_menu_t* p_win, hjtemp_config_t* hjtemp_config)
{
  int row_count = 1;
  const char *name_table[10];
  int list_cnt;
  uint8_t port_number;


  screen_update_list(p_win, row_count, HJTEMP_PAGE_PHYSICAL);
  M_PRINTF(p_win, row_count++, "%-*s:%s", E_L_W, "Physical", 
           ITEM_LIST(hjtemp_config->physical_layer, physical_list));

  if (hjtemp_config->physical_layer == ePHYSICAL_RS232)
  {
    list_cnt = rs232_get_portList(name_table, _countof(name_table));
    port_number = hjtemp_config->rs232_port;
  }
  else
  {
    list_cnt = rs485_get_portList(name_table, _countof(name_table));
    port_number = hjtemp_config->rs485_port;
  }

  screen_update_list(p_win, row_count, HJTEMP_PAGE_PORT);
  M_PRINTF(p_win, row_count++, "%-*s:%s", E_L_W, "Port", safe_name(name_table, list_cnt, port_number));

  screen_update_list(p_win, row_count, HJTEMP_PAGE_MODBUS_ID);
  M_PRINTF(p_win, row_count++, "%-*s:%d", E_L_W, "Modbus ID", hjtemp_config->modbus_id);
  
  screen_update_list(p_win, row_count, HJTEMP_PAGE_TEMP_MENU);
  M_PRINTF(p_win, row_count++, "TempMenu");

  p_win->total_items = row_count;

  while (p_win->current_row < p_win->view_row)
  {
    screen_menu_clear_row(p_win, row_count++);
  }
}

void draw_sensor_page(screen_menu_t* p_win, sensor_t *p_sensor)
{
  int row_count=0;
  p_win->current_row = 0;
  M_PRINTF(p_win, row_count++, "%-*s:%s", E_L_W, "TYPE", g_sensor_model_eng_table[p_sensor->type]);
  switch (p_sensor->type)
  {
    case S_T_ADC:
      draw_adc_page(p_win, get_sensor_config(p_sensor));
      break;
    case S_T_TEMPERATURE_HJ:
      draw_hjtemp_page(p_win, get_sensor_config(p_sensor));
      break;
    default :
      p_win->total_items = row_count;

      screen_clear_unsued_line(p_win);
      break;
  }

}

int32_t setup_select_menu_index(sensor_t* p_sensor, int* choice)
{

  int32_t status;
  int32_t key;
  screen_menu_t menu;
  int32_t index;

  screen_menu_create(&menu, 8, 20);

  
  while (1)
  {
    draw_sensor_page(&menu,p_sensor);
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
      *choice = menu.selected_index;
      return MENU_OK;
    }
    else if (key != KEY_CODE_NONE)
    {
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

  status = print_menu_list(model_list_string,  model_list_count, &choice);

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

}
int32_t general_freq_setup( sensor_t *sensor, uint8_t menu_index)
{

}
int32_t hjwinddir_setup( sensor_t *sensor, uint8_t menu_index)
{

}
int32_t hjwind_setup( sensor_t *sensor, uint8_t menu_index)
{

}
int32_t hjsnow_setup( sensor_t *sensor, uint8_t menu_index)
{

}

int32_t hjtemp_setup(sensor_t* sensor, uint8_t menu_index)

{
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  hjtemp_config_t* hjtemp;
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
      status = print_menu_list(physical_list, _countof(physical_list), &choice);
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
        status = print_menu_list(portList, portListCnt, &choice);

        if (status != MENU_OK)
          break;
        hjtemp->rs232_port = choice;
        save_config_sensor();
      }
      else
      {
        portListCnt = rs485_get_portList(portList, _countof(portList));
        choice = hjtemp->rs485_port;
        status = print_menu_list(portList,  portListCnt,  &choice);
        if (status != MENU_OK)
          break;

        hjtemp->rs485_port = choice;
        save_config_sensor();

        break;
        case HJTEMP_PAGE_MODBUS_ID:
          status = input_decimal("ID", 0, 247, &dec);
          if (status != MENU_OK)
            break;

          hjtemp->modbus_id = dec;
          save_config_sensor();
          break;
        case HJTEMP_PAGE_TEMP_MENU:

       //   status = hjtemperature_menu();

          break;
      }
  }

  return status;
}

int32_t hjhumi_setup(sensor_t* sensor, uint8_t menu_index) {}
int32_t ott_smp3_setup(sensor_t* sensor, uint8_t menu_index) {}
int32_t rain_present_setup(sensor_t* sensor, uint8_t menu_index) {}

const sensor_setup_entry_t g_sensor_setup_table[] = {
    {.sensor_type = S_T_ADC, .config_set = general_adc_setup},
    {.sensor_type = S_T_FREQ, .config_set = general_freq_setup},
    {.sensor_type = S_T_WIND_DIRECTION_HJ_485, .config_set = hjwinddir_setup},
    {.sensor_type = S_T_WIND_SPEED_HJ_485, .config_set = hjwind_setup},
    {.sensor_type = S_T_SNOW_HJ, .config_set = hjsnow_setup},
    {.sensor_type = S_T_TEMPERATURE_HJ, .config_set = hjtemp_setup},
    {.sensor_type = S_T_HUMINITY_HJ, .config_set = hjhumi_setup},
    {.sensor_type = S_T_SOLAR_RADIATION_OTT_SMP3, .config_set = ott_smp3_setup},
    {.sensor_type = S_T_RAIN_PRESENT_DI, .config_set = rain_present_setup}};



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
    status = setup_select_menu_index(sensor, &choice);
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

void draw_menu_sensor_page(screen_menu_t* p_win)
{
  int row_count = 0;

  p_win->current_row = 0;

  for(int i = 0;i< SENSOR_LIST_MAX;i++)
  {
    M_PRINTF(p_win, row_count++, "%s", sensor_name_eng_list[i]);
  }

  p_win->total_items = row_count;

  screen_clear_unsued_line(p_win);
}

int32_t setup_menu_sensor(void)
{
  int32_t choice;
  int32_t status;
  int32_t key;
  int32_t index;
  screen_menu_t menu;

  screen_menu_create(&menu, 8, 20);

  while (1)
  {
    draw_menu_sensor_page(&menu);
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
      screen_clear(menu.view_row,menu.view_col);
      setup_sensor((eSENSOR_TYPE_t)menu.selected_index);
    }
    else if (key != KEY_CODE_NONE)
    {
      screen_menu_handle(&menu, key);
    }
  }

  return convert_key_to_status(key);
}
