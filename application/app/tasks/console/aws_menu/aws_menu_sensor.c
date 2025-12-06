

#include "cli_input.h"
#include "config_app.h"
#include "config_sensor.h"
#include "console_scanf.h"
#include "console_utile.h"
#include "debug_io.h"
#include "driver_interface.h"
#include "hj_snow_menu.h"
#include "hj_temp_menu\hjtemp_menu.h"
#include "system_err.h"
#include "temperature\hj_temperature.h"
#include "temperature\temperature_define.h"
#include "terminal.h"
#include "util_memory.h"
#include "vt100_command.h"
#include "util_stdio.h"
#include "menu_handler.h"

#define ENTRY_PF(cnt, width, label, format, ...) \
  debug_printf("%2d.%-*s:" format "\r\n", cnt, width, label, ##__VA_ARGS__)

#define ENTRY_LABEL_WIDTH 16
const char *adcChModeList[] = {"Single", "Diff"};
const char *unusedList[] = {"미사용"};
const char *rs232ParityList[] = {"None", "Even", "Odd"};
const char *physical_list[] = {"RS232", "RS485"};

typedef struct
{
  uint8_t sensor_type;
  int32_t (*config_set)(eSENSOR_TYPE_t type,sensor_t *, uint8_t);
} sensor_config_entry_t;

void make_option(eSENSOR_TYPE_t type,sensor_t *sensor, char *out, uint16_t outSize)
{
  void *cfg;
  const char *list[10] = {" "};

  out[0] = 0;

  cfg = get_sensor_config(type,sensor->model);;

  if (cfg == NULL)
  {
    snprintf(out, outSize, "%s", " ");
    return;
  }

  switch (sensor->model)
  {
    case S_T_SNOW_HJ:
    {
      snow_hj_config_t *hjsnow = (snow_hj_config_t *)cfg;
      if (hjsnow->physical_layer == ePHYSICAL_RS232)
      {
        rs232_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s]", list[hjsnow->rs232_port]);
      }
      else
      {
        drv_rs485_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s]", list[hjsnow->rs485_port]);
      }
    }
    break;
    case S_T_ADC:
    {
      adc_config_t *adc_cfg = (adc_config_t *)cfg;
      snprintf(out, outSize, "[%s.%d]", adcChModeList[adc_cfg->mode], adc_cfg->single_channel);
    }
    break;
    case S_T_HUMINITY_HJ:
    case S_T_TEMPERATURE_HJ:
    {
      temp_hj_config_t *hjtemp = (temp_hj_config_t *)cfg;
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        rs232_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s][A.%d]", list[hjtemp->rs232_port], hjtemp->modbus_id);
      }
      else
      {
        drv_rs485_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s][A.%d]", list[hjtemp->rs485_port], hjtemp->modbus_id);
      }
    }
    break;
    case S_T_WIND_DIRECTION_HJ_485:
    {
      wind_direction_hj_pulse_config_t *hjwindDir = (wind_direction_hj_pulse_config_t *)cfg;
      drv_rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[hjwindDir->rs485_port]);
    }
    break;
    case S_T_WIND_SPEED_HJ_485:
    {
      wind_speed_hj_pulse_config_t *hjwind = (wind_speed_hj_pulse_config_t *)cfg;
      drv_rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[hjwind->rs485_port]);
    }
    break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
    {
      solar_r_ott_smp3_config_t *ott = (solar_r_ott_smp3_config_t *)cfg;
      drv_rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s][A.%d]", list[ott->rs485_port], ott->modbus_id);
    }
    break;
    case S_T_FREQ:
    {
      frequency_config_t *freq_cfg = (frequency_config_t *)cfg;
      snprintf(out, outSize, "[%d]", freq_cfg->channel);
    }
    break;
    default:
      out[0] = 0;
      break;
  }
}

#define ADC_SET_CH_MODE 0
#define ADC_SET_CHANNLEL 1
#define ADC_SET_HIGHSCALE 2
#define ADC_SET_LOWSCALE 3
#define ADC_SET_SCALE 4
#define ADC_SET_OUTMAXVOLT 5
#define ADC_SET_OUTMINVOLT 6

#define ADC_LB_W 15
uint8_t print_adc_cfg( adc_config_t *adc_config, uint8_t cnt)
{
  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("Adc Mode",ENTRY_LABEL_WIDTH), ITEM_LIST(adc_config->mode, adcChModeList));
  if(adc_config->mode ==0)
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Channel",ENTRY_LABEL_WIDTH), adc_config->single_channel);
  else
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Channel",ENTRY_LABEL_WIDTH), adc_config->diff_channel);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("High Value",ENTRY_LABEL_WIDTH), adc_config->high_scale);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Low Value",ENTRY_LABEL_WIDTH), adc_config->low_scale);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Scale",ENTRY_LABEL_WIDTH), adc_config->scale);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Max Voltage(mV)",ENTRY_LABEL_WIDTH), adc_config->out_max_mv);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Min Voltage(mV)",ENTRY_LABEL_WIDTH), adc_config->out_min_mv);

  return cnt;
}





#define HJWIND_CFG_FULL 0
#define HJWIND_CFG_OFF 1
#define HJWIND_CFG_PORT 2

#define HJ_WIND_L_W 10
uint8_t print_hjwind_cfg( wind_speed_hj_pulse_config_t *hjwindCfg, uint8_t cnt)
{
  const char *name_table[10];

  drv_rs485_get_portList(name_table, _countof(name_table));
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Fullset",ENTRY_LABEL_WIDTH), hjwindCfg->full);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("Offset",ENTRY_LABEL_WIDTH), hjwindCfg->offset);
  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("Port",ENTRY_LABEL_WIDTH), name_table[hjwindCfg->rs485_port]);

  return cnt;
}

#define HJWIND_DIR_CFG_PORT 0
uint8_t print_hjwindDir_cfg( wind_speed_hj_pulse_config_t *hjwindCfg, uint8_t cnt)
{
  const char *name_table[10];

  drv_rs485_get_portList(name_table, _countof(name_table));

  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("Port",ENTRY_LABEL_WIDTH), name_table[hjwindCfg->rs485_port]);

  return cnt;
}

// 화진 온도

/*nn.통신방식  :RS232|RS485
  nn.포트      :n
*/
#define HJTEMP_CFG_PHYSICAL_LAYER 0
#define HJTEMP_CFG_PORT 1
#define HJTEMP_CFG_MODBUS_ID 2
#define HJTEMP_CTRL_OFFSET 3
uint8_t print_hjtemp_cfg(temp_hj_config_t *hjtempCfg, uint8_t cnt)
{
  const char *name_table[10];
  int port;
  
  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("통신방식",ENTRY_LABEL_WIDTH), physical_list[hjtempCfg->physical_layer]);

  if (hjtempCfg->physical_layer == ePHYSICAL_RS232)
  {
    rs232_get_portList(name_table, _countof(name_table));
    port = hjtempCfg->rs232_port;
  }
  else
  {
    drv_rs485_get_portList(name_table, _countof(name_table));
        port = hjtempCfg->rs485_port;
  }

  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("포트",ENTRY_LABEL_WIDTH), name_table[port]);
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("모드버스 ID",ENTRY_LABEL_WIDTH), hjtempCfg->modbus_id);
  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("온습도 메뉴",ENTRY_LABEL_WIDTH), "[제어]");

  return cnt;
}

#define OTT_SMP3_CFG_PORT 0
#define OTT_SMP3_CFG_ID 1
uint8_t print_ott_smp3_cfg(solar_r_ott_smp3_config_t *ott, uint8_t cnt)
{
   const char *portNameList[10];

   drv_rs485_get_portList(portNameList, _countof(portNameList));
   debug_printf("%2d.%s:%s\r\n",cnt++, m_l("포트",ENTRY_LABEL_WIDTH), portNameList[ott->rs485_port]);
   debug_printf("%2d.%s:%d\r\n",cnt++, m_l("MODBUS ID",ENTRY_LABEL_WIDTH), ott->modbus_id);
   return cnt;
}

#define RAIN_PRESENT_DELAY 0
uint8_t print_rain_present_cfg( rain_present_config_t *rain_present, uint8_t cnt)
{

  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("해제 지연시간",ENTRY_LABEL_WIDTH), rain_present->off_delay_sec);

  return cnt;
}


uint8_t print_freq_cfg(frequency_config_t *freq, uint8_t cnt)
{
  debug_printf("%2d.%s:%d\r\n",cnt++, m_l("채널",ENTRY_LABEL_WIDTH), freq->channel);
  debug_printf("%2d.%s:%f\r\n",cnt++, m_l("보정계수",ENTRY_LABEL_WIDTH), freq->scale_factor);

  return cnt;
}

#define HJSNOW_CFG_MENU_PHY  0
#define HJSNOW_CFG_MENU_PORT 1
#define HJSNOW_CFG_MENU      2
uint8_t print_hjsnow_cfg(snow_hj_config_t *hjsnow, uint8_t cnt)
{
  const char *portNameList[10];

  debug_printf("%2d.%s:%s\r\n",cnt++,  m_l("통신방식",ENTRY_LABEL_WIDTH), physical_list[hjsnow->physical_layer]);

  if (hjsnow->physical_layer == ePHYSICAL_RS232)
  {
    rs232_get_portList(portNameList, _countof(portNameList));
    debug_printf("%2d.%s:%s\r\n",cnt++,  m_l("통신포트",ENTRY_LABEL_WIDTH), portNameList[hjsnow->rs232_port]);
  }
  else
  {
    drv_rs485_get_portList(portNameList, _countof(portNameList));
    debug_printf("%2d.%s:%s\r\n",cnt++,  m_l("통신포트",ENTRY_LABEL_WIDTH), portNameList[hjsnow->rs485_port]);
  }

  debug_printf("%2d.%s:%s\r\n",cnt++,  m_l("화진 적설 메뉴",ENTRY_LABEL_WIDTH), "[제어]");

  return cnt;
}

#define SJGP215_CFG_PORT 0
uint8_t print_barometer_jsgp215_cfg(barometer_jinsung_sjgp215_config_t *ott, uint8_t cnt)
{
  const char *portNameList[10];

  rs232_get_portList(portNameList, _countof(portNameList));
  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("포트",ENTRY_LABEL_WIDTH), portNameList[ott->rs232_port]);
 
  return cnt;
}

/**
 * @brief 센서 개별 설정 가능한 항목 목록을 출력
 * @retval 각 센서설정가능한 항목수
 *
 *
 0.종류            :화진 온습도
 1.통신방식        :RS485
 2.포트            :RS232/RS485 B
 3.모드버스 ID     :1
 4.온습도 메뉴     :[제어]

 이러한 설정메뉴가 나타나며 리턴값은 설정항목들 갯수
 */
int32_t print_common_cfg( eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t c)
{
  int32_t cnt = 0;

  debug_printf("%2d.%s:%s\r\n",cnt++, m_l("종류",ENTRY_LABEL_WIDTH), g_sensor_model_table[sensor->model]);

  switch (sensor->model)
  {
    case S_T_ADC:  // ADC
      cnt = print_adc_cfg( get_sensor_config(type,sensor->model), cnt);
      break;

    case S_T_SNOW_HJ:
      cnt = print_hjsnow_cfg( get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_WIND_DIRECTION_HJ_485:
      cnt = print_hjwindDir_cfg( get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_WIND_SPEED_HJ_485:
      cnt = print_hjwind_cfg( get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_TEMPERATURE_HJ:
      cnt = print_hjtemp_cfg( get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_HUMINITY_HJ:
      cnt = print_hjtemp_cfg( get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
      cnt = print_ott_smp3_cfg( get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_RAIN_PRESENT_DI:
      cnt = print_rain_present_cfg(get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_FREQ:
      cnt = print_freq_cfg(get_sensor_config(type,sensor->model), cnt);
      break;
    case S_T_BARO_JINSUNG_SJGP215:
      cnt = print_barometer_jsgp215_cfg(get_sensor_config(type,sensor->model), cnt);
      break;
  }
  return cnt;
}

int32_t select_menu_index( eSENSOR_TYPE_t type,sensor_t *sensor,int *choice)
{
  int index = 0;
  int entry_count;
  int status;

  debug_printf("\r\n");
  do
  {
    entry_count = print_common_cfg(type,sensor, 0);

    status = input_decimal_prompt("번호를 선택해 주세요", &index, 0, entry_count - 1);

    if(status != MENU_OK)
      break;
    
      *choice = index;
      break;
    

  } while (1);

  return status;
}
/**
 * @brief index로 저장된 센서 목록을 문자열 목록으로 가져오기
 */
uint16_t get_sensor_model_list(const char **model_list, const uint8_t *idxList, uint8_t listCnt)
{
  int i;

  for (i = 0; i < listCnt; i++)
  {
    model_list[i] = g_sensor_model_table[idxList[i]];
  }

  return i;
}

uint16_t get_sensor_model_eng_list(const char **model_list, const uint8_t *idxList, uint8_t listCnt)
{
  int i;

  for (i = 0; i < listCnt; i++)
  {
    model_list[i] = g_sensor_model_eng_table[idxList[i]];
  }

  return i;
}



void set_type(eSENSOR_TYPE_t type,sensor_t *sensor)
{
  WRITE_CFG_MEM(&sensor->model, sizeof(sensor->model));

}
/**
 * @brief 센서 모델 변경
 */
int32_t sensor_model_set(eSENSOR_TYPE_t type, sensor_t *sensor, const uint8_t *model_list, uint8_t list_cnt)
{
  uint8_t model_list_count;
  int32_t status=0;
  int32_t choice;
  const char *model_list_string[10];

  model_list_count = get_sensor_model_list(model_list_string, model_list, list_cnt);
  status = select_index_from_table(model_list_string, NULL, model_list_count, true, &choice);

  if (status !=MENU_OK)
  {
    return status;
  }
  sensor->model = (eSENSOR_TYPE_MODEL_t)model_list[choice];
  set_type(type,sensor);

  return MENU_OK;
}



int32_t hjwind_config_set( eSENSOR_TYPE_t type, sensor_t *sensor, uint8_t munu_index)
{
  int32_t status=0;
  int32_t choice;
  const char *portList[10];
  int32_t dec;
  uint16_t port_cnt;

  wind_speed_hj_pulse_config_t *hjwind;

  hjwind = get_sensor_config(type,sensor->model);;
  if (hjwind == NULL)
  {
    return status;
  }
  switch (munu_index)
  {
    case HJWIND_CFG_FULL:
      status = input_decimal_prompt("FULL SET",&dec,0, 999999);
      if(status != MENU_OK)
      {
        break;
      }
    
        hjwind->full = dec;
        save_config_sensor();
      break;
    case HJWIND_CFG_OFF:
      status = input_decimal_prompt("OFFSET", &dec, 0, 999999);
      if (status != MENU_OK)
      {
        break;
      }
      
      hjwind->offset = dec;
      save_config_sensor();
      
      break;
    case HJWIND_CFG_PORT:
      port_cnt = drv_rs485_get_portList(portList, _countof(portList));
      status = select_index_from_table( portList, NULL, port_cnt, true,&choice);
      if(status !=MENU_OK)
      break;

      hjwind->rs485_port = choice;
      save_config_sensor();
      break;
    default:
      break;
  }

  return status;
}

int32_t hjwinddir_config_set(eSENSOR_TYPE_t type, sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  wind_speed_hj_pulse_config_t *hjwind;
  const char *portList[10];

  uint16_t port_cnt;

  hjwind = get_sensor_config(type,sensor->model);;
  if (hjwind == NULL)
  {
    return 0;
  }
  switch (menu_index)
  {
    case HJWIND_DIR_CFG_PORT:
      port_cnt = drv_rs485_get_portList(portList, _countof(portList));

      status = select_index_from_table( portList, NULL, port_cnt, true,&choice);
      if (status != MENU_OK)
      {
        break;
      }
        hjwind->rs485_port = choice;
        save_config_sensor();
      break;
  }

  return status;
}

/*
0.type:화진 RS485 9600
1.port:EX1 RS485 A
*/

int32_t hjtemp_config_set(eSENSOR_TYPE_t type, sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  temp_hj_config_t *hjtemp;
  const char *portList[10];
  uint16_t portListCnt;

  hjtemp = get_sensor_config(type,sensor->model);;
  if (hjtemp == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJTEMP_CFG_PHYSICAL_LAYER:
      status = select_index_from_table( physical_list, NULL, _countof(physical_list), true,&choice);
      if (status!= MENU_OK)
        break;
        hjtemp->physical_layer = (ePHYSOCAL_LAYER_t)(choice );
        save_config_sensor();
      break;
    case HJTEMP_CFG_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        status = select_index_from_table( portList, NULL, portListCnt, true,&choice);
  
        if(status != MENU_OK)
          break;
          hjtemp->rs232_port = choice;
          save_config_sensor();
      }
      else
      {
        portListCnt = drv_rs485_get_portList(portList, _countof(portList));
        status = select_index_from_table( portList, NULL, portListCnt, true,&choice);
      if (status!= MENU_OK)
        break;

          hjtemp->rs485_port = choice;
          save_config_sensor();

      break;
    case HJTEMP_CFG_MODBUS_ID:
      status = input_decimal_prompt("ID", &dec,0, 247);
      if(status !=MENU_OK)
        break;

        hjtemp->modbus_id = dec;
        save_config_sensor();
      break;
    case HJTEMP_CTRL_OFFSET:

      status = hjtemperature_menu();     

    break;
  }
  }
  
  return status;
  
}

int32_t hjhumi_config_set( eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  const char *portList[10];
  uint16_t portListCnt;
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  temp_hj_config_t *hjtemp;


  hjtemp = get_sensor_config(type,sensor->model);;
  if (hjtemp == NULL)
  {
    ERROR_PRINTF("화진 온습도 설정값 NULL");    
    return MENU_OK;
  }

  switch (menu_index)
  {
    case HJTEMP_CFG_PHYSICAL_LAYER:
      status = select_index_from_table(physical_list, NULL, _countof(physical_list), true,&choice);
      if(status !=MENU_OK)
        break;
        hjtemp->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
        save_config_sensor();
        break;
    case HJTEMP_CFG_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        status = select_index_from_table( portList, NULL, portListCnt, true,&choice);
        
        if(status !=MENU_OK)
        break;

          hjtemp->rs232_port = choice;
          save_config_sensor();
      }
      else
      {
        portListCnt = drv_rs485_get_portList(portList, _countof(portList));
        status = select_index_from_table( portList, NULL, portListCnt, true,&choice);

        if(status !=MENU_OK)
        break;
        hjtemp->rs485_port = choice;
        save_config_sensor();
      }
      break;
    case HJTEMP_CFG_MODBUS_ID:
      status = input_decimal_prompt("ID",&dec, 0, 247);
      
      if(status != MENU_OK)
        break;
      {
        hjtemp->modbus_id = dec;
        save_config_sensor();
      }
      break;
    case HJTEMP_CTRL_OFFSET:
     status =  hjtemperature_menu();
       break;
    default:
      break;

      
  }
  
  return status;
}

int32_t ott_smp3_config_set( eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  solar_r_ott_smp3_config_t *ott;
  const char *portList[10];
  uint16_t portListCnt;

  ott = get_sensor_config(type,sensor->model);;
  if (ott == NULL)
  {
    ERROR_PRINTF("OTT 일사 설정값 NULL");
    return 0;
  }

  switch (menu_index)
  {
    case OTT_SMP3_CFG_PORT:
      portListCnt = drv_rs485_get_portList(portList, _countof(portList));
      status = select_index_from_table( portList, NULL, portListCnt, true,&choice);

      if (status  != MENU_OK)
        break;

        ott->rs485_port = choice;
        save_config_sensor();


      break;
    case OTT_SMP3_CFG_ID:
      status = input_decimal_prompt("ID",&dec, 0, 247);
      if(status !=MENU_OK)
        break;
        ott->modbus_id = dec;
        save_config_sensor();

      break;
    default:
      break;
  }
  
  return status;
}

int32_t rain_present_config_set(eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t dec = 0;
  rain_present_config_t *rain_present;

  rain_present = get_sensor_config(type,sensor->model);;
  if (rain_present == NULL)
  {
    ERROR_PRINTF("강우 감지 NULL");
    return 0;
  }

  switch (menu_index)
  {
    case RAIN_PRESENT_DELAY:
      status = input_decimal_prompt("지연시간(s)", &dec, 1, 300);
      if (status != MENU_OK)
        break;
      rain_present->off_delay_sec = dec;
      save_config_sensor();

      break;

  }

  return status;
}

#define GENERAL_FREQ_CHANNEL 0
#define GENERAL_FREQ_SCALE_FACTOR 1
int32_t general_freq_config_set(eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  float factor = 0;
  frequency_config_t *freq;
  int dec;

  freq = get_sensor_config(type,sensor->model);;
  if (freq == NULL)
  {
    ERROR_PRINTF("GENERAL FREQ NULL");
    return 0;
  }

  switch (menu_index)
  {
    case GENERAL_FREQ_CHANNEL:
      status = input_decimal_prompt("채널", &dec, 0, 1);
      if (status != MENU_OK)
        break;
      freq->channel = dec;
      save_config_sensor();
      break;
    case GENERAL_FREQ_SCALE_FACTOR:
      status = input_float_adv("변환식 보정계수", 0, 0, &factor,"%6.3f");
      if (status != MENU_OK)
        break;
      freq->scale_factor = factor;
      save_config_sensor();
      break;
  }

  return status;
}

int32_t hjsnow_config_set( eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  const char *portList[10];
  uint16_t portCnt;
  int32_t status;
  int32_t choice;
  snow_hj_config_t *hjsnow;


  hjsnow = get_sensor_config(type,sensor->model);;
  if (hjsnow == NULL)
  {
    ERROR_PRINTF("화진 적설설 설정값 NULL");
    return 0;
  }
  switch (menu_index)
  {
    case HJSNOW_CFG_MENU_PHY:
      status = select_index_from_table(physical_list, NULL, _countof(physical_list), true,&choice);
      if (status != MENU_OK)
      break;
 
        hjsnow->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
        save_config_sensor();

      break;
    case HJSNOW_CFG_MENU_PORT:
      if (hjsnow->physical_layer == ePHYSICAL_RS232)
      {
        portCnt = rs232_get_portList(portList, _countof(portList));
        status = select_index_from_table( portList, NULL, portCnt, true,&choice);
        if (status !=MENU_OK)
        break;
   
         hjsnow->rs232_port = choice;
         save_config_sensor();
        }
      else
      {
        portCnt = drv_rs485_get_portList(portList, _countof(portList));
        choice = hjsnow->rs485_port;
        status = select_index_from_table( portList, NULL, portCnt, true,&choice);
        if (status != MENU_OK)
        break;
    
          hjsnow->rs485_port = choice;
          save_config_sensor();
 
      }

      break;
    case HJSNOW_CFG_MENU:
    hj_snow_menu();
    break;
    
    default:
      break;
  }
  
  return status;
  
}



int32_t general_adc_config_set( eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  
  int32_t dec;
  adc_config_t *adc;

  int status=0;
  int choice;

  adc = get_sensor_config(type,sensor->model);
  switch (menu_index)
  {
    case ADC_SET_CH_MODE:  // 1.채널 모드
      status = select_index_from_table(adcChModeList, NULL, _countof(adcChModeList), true,&choice);
      if (status !=MENU_OK)
        break;


        adc->mode =choice;
        save_config_sensor();

      break;
    case ADC_SET_CHANNLEL:  // channel;
      if(adc->mode ==0)
      {
      status = input_decimal_prompt("싱글 채널",&dec, 0, 17);
      if(status !=MENU_OK)
      break;
             adc->single_channel = dec;
      }
      else
      {
      status = input_decimal_prompt("차동 채널",&dec, 0, 8);
      if(status !=MENU_OK)
       adc->diff_channel = dec;
      }

        save_config_sensor();

      break;
    case ADC_SET_HIGHSCALE:  // hish cale;
      status = input_decimal_prompt("High Value",&dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->high_scale = dec;
        save_config_sensor();

      break;
    case ADC_SET_LOWSCALE:  // low cale;
      status = input_decimal_prompt("Low Value", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->low_scale = dec;
        save_config_sensor();

      break;
    case ADC_SET_SCALE:  // ale;
      status = input_decimal_prompt("Scale", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->scale = dec;
        save_config_sensor();

      break;

    case ADC_SET_OUTMAXVOLT:
      status = input_decimal_prompt("Max Votage(mv)", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->out_max_mv = dec;
        save_config_sensor();

      break;

    case ADC_SET_OUTMINVOLT:
      status = input_decimal_prompt("Min Voltage(mv)", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->out_min_mv = dec;
        save_config_sensor();

      break;

  }

  return status;
}

int32_t sjgp215_config_set(eSENSOR_TYPE_t type,sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t choice;

  barometer_jinsung_sjgp215_config_t *sjgp215;
  const char *portList[10];
  uint16_t portListCnt;

  sjgp215 = get_sensor_config(type,sensor->model);;
  if (sjgp215 == NULL)
  {
    ERROR_PRINTF("진성 일사 설정값 NULL");
    return 0;
  }

  switch (menu_index)
  {
  case OTT_SMP3_CFG_PORT:
    portListCnt = rs232_get_portList(portList, _countof(portList));
    choice  =   sjgp215->rs232_port;
    status = select_index_from_table(portList, NULL, portListCnt, true, &choice);

    if (status != MENU_OK)
      break;

    sjgp215->rs232_port = choice;
    save_config_sensor();

    break;

  default:
    break;
  }

  return status;
}
/*
센서 모델과 모델 설정 함수 연결
센서가 추가되거나 센서고유의 설정값을 변경하려면 처리 함수를 작성해야한다.
*/
const sensor_config_entry_t g_config_sensor_table[] = {
    {.sensor_type = S_T_ADC, .config_set = general_adc_config_set},
    {.sensor_type = S_T_FREQ, .config_set = general_freq_config_set},
    {.sensor_type = S_T_WIND_DIRECTION_HJ_485, .config_set = hjwinddir_config_set},
    {.sensor_type = S_T_WIND_SPEED_HJ_485, .config_set = hjwind_config_set},
    {.sensor_type = S_T_SNOW_HJ, .config_set = hjsnow_config_set},
    {.sensor_type = S_T_TEMPERATURE_HJ, .config_set = hjtemp_config_set},
    {.sensor_type = S_T_HUMINITY_HJ, .config_set = hjhumi_config_set},
    {.sensor_type = S_T_SOLAR_RADIATION_OTT_SMP3, .config_set = ott_smp3_config_set},
    {.sensor_type = S_T_RAIN_PRESENT_DI, .config_set = rain_present_config_set},
    {.sensor_type = S_T_BARO_JINSUNG_SJGP215, .config_set = sjgp215_config_set}};

int32_t sensor_set( eSENSOR_TYPE_t type,sensor_t *p_sensor, uint8_t choice)
{
  int32_t status=MENU_OK;

  for (int i = 0; i < _countof(g_config_sensor_table); i++)
  {  // 센서마다 고유의 처리 함수를 사용한다.
    if (g_config_sensor_table[i].sensor_type == p_sensor->model)
    {
      status = g_config_sensor_table[i].config_set(type,p_sensor, choice - 1);
      break;
    }
  }

  return status;
}

/*
센서 설정
*/
int32_t menu_sensor( eSENSOR_TYPE_t list)
{
  int32_t status = 0;
  int32_t choice  = 0;

  // 선택된 센서의 설정 정보를 가져온다.
  sensor_t *sensor = &get_config_app()->sensor[(int)list];
  do
  {
    /*
    센서의 현재 정보를 출력하고, 수정을 원하는 항목의 번호를 입력받는다.
    0.type       :화진 RS485 9600
    1.port        :EX1 RS485 A
    이런 화면이 나타남남
    */
    status = select_menu_index(list,sensor,&choice);
    if (status != MENU_OK)
        break; 
    
    switch (choice)
    {
      case 0:  // 센서가 사용하고자하는 센서 타입을 설정한다.
               // 센서마다 지원가능한 목록을 넘겨지고 출력하여 선택하도록 한다.
        status =sensor_model_set( (eSENSOR_TYPE_t)choice,sensor, sensor_table[list].list, sensor_table[list].cnt);
        break;
      default:  // 센서 타입이 아닌 센서 고유 속성들은 이 함수 에서 처리한다.
        // 현재의 센서 정보와 사용자가 수정하고자한 항목 번호를 넘긴다.
        status = sensor_set(list,sensor, choice);  // 센서별 설정값 변경
        break;
    }

    if(status == MENU_ABORT)
      break;
  } while (1);

  return status;
}

/*
 0.기온          :미사용                                  ,  32.전천복사      :미사용
 1.풍향          :미사용                                  ,  33.반사복사      :미사용
 2.풍속          :미사용                                  ,  34.직달          :미사용
...계속

여기서 수정하고 싶은 센서번호를 입력
센서번호는 정해진 순서대로 입력되어야함
 */
#define LABEL_W 14
#define S_LABEL_W 20
int32_t print_menu_sensor(void)
{
  char opt[25];
  char label[50];
  char sensor_label[50];
  int32_t cnt = 0;
  int i = 0;

  debug_printf("\r\n");


  cnt = _countof(sensor_name_list) / 2;

  for (i = 0; i < cnt; i++)
  {
    make_utf8_string(label, sizeof(label), LABEL_W,sensor_name_list[i]);
    make_utf8_string(sensor_label, sizeof(sensor_label), S_LABEL_W,(ITEM_LIST(get_config_app()->sensor[i].model, g_sensor_model_table)));
    make_option((eSENSOR_TYPE_t)i,&get_config_app()->sensor[i], opt, sizeof(opt));
    debug_printf("%2d.%-14s:%-20s %-22s, ", i, label, sensor_label, opt);

    make_utf8_string(label, sizeof(label), LABEL_W,sensor_name_list[i + cnt]);
    make_utf8_string(sensor_label, sizeof(sensor_label), S_LABEL_W,(ITEM_LIST(get_config_app()->sensor[i + cnt].model, g_sensor_model_table)));
    make_option((eSENSOR_TYPE_t)i,&get_config_app()->sensor[i + cnt], opt, sizeof(opt));
    debug_printf("%2d.%-14s:%-20s %-22s\r\n", i + cnt, label, sensor_label, opt);
  }


  cnt = _countof(sensor_name_list);
  return cnt;
}

int32_t aws_menu_sensor(void)
{
  int32_t status=0;
  int32_t choice;

  do
  {
    // 모든 센서의 출력, 기본정보 출력
    status = select_index_from_table( NULL, print_menu_sensor, 0, false,&choice);
    if (status != MENU_OK)
          break;
    status = menu_sensor((eSENSOR_TYPE_t)(choice));
    if(status ==MENU_ABORT)
    break;
  }while(1);

  debug_printf("장비리셋 후 설정값이 적용됩니다.\r\n");
  return status;
}