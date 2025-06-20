

#include "cli_input.h"
#include "config_app.h"
#include "config_sensor.h"
#include "console_scanf.h"
#include "console_utile.h"
#include "dev_io.h"
#include "driver_interface.h"
#include "hj_snow_menu.h"
#include "system_err.h"
#include "temperature\hj_temperature.h"
#include "temperature\temperature_define.h"
#include "terminal.h"
#include "util_memory.h"
#include "vt100_command.h"
#include "hj_temp_menu\hjtemp_menu.h"
const char *adcChModeList[] = {"single", "diff"};
const char *unusedList[] = {"미사용"};
const char *rs232ParityList[] = {"none", "even", "odd"};
const char *physical_list[] = {"RS232", "RS485"};

typedef struct
{
  uint8_t sensorType;
  int32_t (*config_set)(sensor_t *, uint8_t);
} config_sen_func_t;

void make_option(sensor_t *sensor, char *out, uint16_t outSize)
{
  void *cfg;
  const char *list[10] = {" "};

  out[0] = 0;

  cfg = get_sensor_config(sensor);

  if (cfg == NULL)
  {
    snprintf(out, outSize, "%s", " ");
    return;
  }

  switch (sensor->type)
  {
    case S_T_TEMP_232:
    case S_T_GENERAL_232:
    {
      rs232_config_t *rs232_cfg = (rs232_config_t *)cfg;
      rs232_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[rs232_cfg->port]);
    }
    break;
    case S_T_TEMP_485:
    case S_T_GENERAL_485:
    {
      rs485_config_t *rs485_cfg = (rs485_config_t *)cfg;
      rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[rs485_cfg->port]);
    }
    break;
    case S_T_SNOW_HJ:
    {
      hjsnow_config_t *hjsnow = (hjsnow_config_t *)cfg;
      if (hjsnow->physical_layer == ePHYSICAL_RS232)
      {
        rs232_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s]", list[hjsnow->port]);
      }
      else
      {
        rs485_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s]", list[hjsnow->port]);
      }
    }
    break;
    case S_T_ADC:
    {
      adc_config_t *adc_cfg = (adc_config_t *)cfg;
      snprintf(out, outSize, "[%s.%d]", adcChModeList[adc_cfg->mode], adc_cfg->channel);
    }
    break;
    case S_T_HUMINITY_HJ:
    case S_T_TEMPERATURE_HJ:
    {
      hjtemp_config_t *hjtemp = (hjtemp_config_t *)cfg;
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        rs232_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s][A.%d]", list[hjtemp->port], hjtemp->modbus_id);
      }
      else
      {
        rs485_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s][A.%d]", list[hjtemp->port], hjtemp->modbus_id);
      }
    }
    break;
    case S_T_WIND_DIRECTION_HJ_485:
    {
      hjwindDirection_config_t *hjwindDir = (hjwindDirection_config_t *)cfg;
      rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[hjwindDir->rs485_port]);
    }
    break;
    case S_T_WIND_SPEED_HJ_485:
    {
      hjwindspeed_config_t *hjwind = (hjwindspeed_config_t *)cfg;
      rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[hjwind->rs485_port]);
    }
    break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
    {
      ott_smp3_config_t *ott = (ott_smp3_config_t *)cfg;
      rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s][A.%d]", list[ott->port], ott->modbus_id);
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

uint8_t print_adc_cfg( adc_config_t *adc_config, uint8_t cnt)
{
  io_printf("%2d.adc mode   :%s\r\n", cnt++, ITEM_LIST(adc_config->mode, adcChModeList));
  io_printf("%2d.channel    :%d\r\n", cnt++, adc_config->channel);
  io_printf("%2d.high scale :%d\r\n", cnt++, adc_config->highScale);
  io_printf("%2d.low scale  :%d\r\n", cnt++, adc_config->lowScale);
  io_printf("%2d.scale      :%d\r\n", cnt++, adc_config->scale);
  io_printf("%2d.outMaxVolt :%d\r\n", cnt++, adc_config->outMaxV);
  io_printf("%2d.outMinVolt :%d\r\n", cnt++, adc_config->outMinV);

  return cnt;
}

uint8_t print_rs232_cfg( rs232_config_t *rs232_config, uint8_t cnt)
{
  const char *portNameList[10];

  rs232_get_portList(portNameList, _countof(portNameList));
  io_printf("%2d.port       :%s\r\n", cnt++, portNameList[rs232_config->port]);
  io_printf("%2d.baud       :%d\r\n", cnt++, rs232_config->baud);
  io_printf("%2d:paraity    :%s\r\n", cnt++, ITEM_LIST(rs232_config->parityIdx, rs232ParityList));
  return cnt;
}

uint8_t print_rs485_cfg( rs485_config_t *rs485_config, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));

  io_printf("%2d.port       :%s\r\n", cnt++, portNameList[rs485_config->port]);
  io_printf("%2d.baud       :%d\r\n", cnt++, rs485_config->baud);
  io_printf("%2d:paraity    :%s\r\n", cnt++, ITEM_LIST(rs485_config->parityIdx, rs232ParityList));
  return cnt;
}

#define HJWIND_CFG_FULL 0
#define HJWIND_CFG_OFF 1
#define HJWIND_CFG_PORT 2
uint8_t print_hjwind_cfg( hjwindspeed_config_t *hjwindCfg, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));
  io_printf("%2d.fullset     :%d\r\n", cnt++, hjwindCfg->full);
  io_printf("%2d.offset      :%d\r\n", cnt++, hjwindCfg->offset);
  io_printf("%2d.port        :%s\r\n", cnt++, portNameList[hjwindCfg->rs485_port]);

  return cnt;
}

#define HJWIND_DIR_CFG_PORT 0
uint8_t print_hjwindDir_cfg( hjwindspeed_config_t *hjwindCfg, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));

  io_printf("%2d.port       :%s\r\n", cnt++, portNameList[hjwindCfg->rs485_port]);

  return cnt;
}

// 화진 온도

/*nn.물리장치  :RS232|RS485
  nn.포트      :n
*/
#define HJTEMP_CFG_PHYSICAL_LAYER 0
#define HJTEMP_CFG_PORT 1
#define HJTEMP_CFG_MODBUS_ID 2
#define HJTEMP_CTRL_OFFSET 3
uint8_t print_hjtemp_cfg( hjtemp_config_t *hjtempCfg, uint8_t cnt)
{
  const char *portNameList[10];

  io_printf("%2d.물리장치   :%s\r\n", cnt++, physical_list[hjtempCfg->physical_layer]);  // 고정

  if (hjtempCfg->physical_layer == ePHYSICAL_RS232)
  {
    rs232_get_portList(portNameList, _countof(portNameList));
  }
  else
  {
    rs485_get_portList(portNameList, _countof(portNameList));
  }
  io_printf("%2d.포트       :%s\r\n", cnt++, portNameList[hjtempCfg->port]);  // 고정
  io_printf("%2d.모드버스 ID:%d\r\n", cnt++, hjtempCfg->modbus_id);           // 고정
  io_printf("%2d.온습도 메뉴[제어]\r\n", cnt++);                                   // 고정
  return cnt;
}

#define OTT_SMP3_CFG_PORT 0
#define OTT_SMP3_CFG_ID 1
uint8_t print_ott_smp3_cfg( ott_smp3_config_t *ott, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));
  io_printf("%2d.포트       :%s\r\n", cnt++, portNameList[ott->port]);  // 고정
  io_printf("%2d.MODBUS ID  :%d\r\n", cnt++, ott->modbus_id);           // 고정
  return cnt;
}

#define RAIN_PRESENT_DELAY 0
uint8_t print_rain_present_cfg( rain_present_config_t *rain_present, uint8_t cnt)
{


  io_printf("%2d.지연시간       :%d\r\n", cnt++, rain_present->delay);  // 고정

  return cnt;
}


uint8_t print_freq_cfg(frequency_config_t *freq, uint8_t cnt)
{
  io_printf("%2d.채널       :%d\r\n", cnt++, freq->channel);
  io_printf("%2d.factor     :%f\r\n", cnt++, freq->scale_factor);

  return cnt;
}

#define HJSNOW_CFG_MENU_PHY  0
#define HJSNOW_CFG_MENU_PORT 1
#define HJSNOW_CFG_MENU      2
uint8_t print_hjsnow_cfg( hjsnow_config_t *hjsnow, uint8_t cnt)
{
  const char *portNameList[10];

  io_printf("%2d.물리장치   :%s\r\n", cnt++,
              physical_list[hjsnow->physical_layer]);  // 고정

  if (hjsnow->physical_layer == ePHYSICAL_RS232)
  {
    rs232_get_portList(portNameList, _countof(portNameList));
  }
  else
  {
    rs485_get_portList(portNameList, _countof(portNameList));
  }

  io_printf("%2d.port        :%s\r\n", cnt++, portNameList[hjsnow->port]);  // 고정
  io_printf("%2d.화진 적설 메뉴(제어)\r\n", cnt++);
  
  return cnt;
}

/*
 센서 개별
 */
int32_t print_common_cfg( sensor_t *sensor, uint8_t c)
{
  int32_t cnt = 0;

  io_printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);

  switch (sensor->type)
  {
    case S_T_ADC:  // ADC
      cnt = print_adc_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_GENERAL_232:
      cnt = print_rs232_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_GENERAL_485:
      cnt = print_rs485_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_SNOW_HJ:
      cnt = print_hjsnow_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_WIND_DIRECTION_HJ_485:
      cnt = print_hjwindDir_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_WIND_SPEED_HJ_485:
      cnt = print_hjwind_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_TEMPERATURE_HJ:
      cnt = print_hjtemp_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_HUMINITY_HJ:
      cnt = print_hjtemp_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
      cnt = print_ott_smp3_cfg( get_sensor_config(sensor), cnt);
      break;
    case S_T_RAIN_PRESENT_DI:
      cnt = print_rain_present_cfg(get_sensor_config(sensor), cnt);
      break;
    case S_T_FREQ:
      cnt = print_freq_cfg(get_sensor_config(sensor), cnt);
      break;
  }
  return cnt;
}

int32_t select_indexMenu( sensor_t *sensor,int *choice)
{
  int index = 0;
  int funcCnt = 0;
  int indexMax;
  int status;

  do
  {
    funcCnt = print_common_cfg( sensor, 0);
    indexMax = funcCnt;


    status = input_decimal_prompt("번호를 선택해 주세요",&index,0,indexMax-1);


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
uint16_t gen_sensorItemList(const char **itemListOut, const uint8_t *idxList, uint8_t listCnt)
{
  int i;

  for (i = 0; i < listCnt; i++)
  {
    itemListOut[i] = g_sensor_model_list[idxList[i]];
  }

  return i;
}

void set_type(sensor_t *sensor)
{
  WRITE_CFG_MEM(&sensor->type, sizeof(sensor->type));
  if (sensor->type != S_T_UNSUED)
  {
    void *cfg = get_sensor_config(sensor);
    if (cfg == NULL)
    {
      sensor_add(sensor);
    }
  }
}
/**
 * @brief 센서 모델 변경
 */
int32_t sensor_type_set( sensor_t *sensor, const uint8_t *list, uint8_t listCnt)
{

  uint8_t itemListCnt;
  int32_t status=0;
  int32_t choice;
  const char *itemList[10];

  itemListCnt = gen_sensorItemList(itemList, list, listCnt);
  status = select_indexFromList(itemList, NULL, itemListCnt, true,&choice);
  if (status !=MENU_OK)
  {
    return status;
  }
  sensor->type = (eSENSOR_MODEL_t)list[choice];
  set_type(sensor);
  

  return MENU_OK;
}

// 232설정

#define RS232_SET_PORT 0
#define RS232_SET_BAUD 1
#define RS232_SET_PARITY 2

int32_t rs232_config_set( sensor_t *sensor, uint8_t cnt)
{
  int32_t choice;
  int32_t status =0;
  int32_t dec;
  rs232_config_t *rs232;
  const char *portList[10];
  rs232 = get_sensor_config(sensor);
  if (rs232 == 0)
  {
    sensor_add(sensor);
    rs232 = get_sensor_config(sensor);
    io_printf("rs232 err\r\n");
  }
  switch (cnt)
  {
    case RS232_SET_PORT:

      cnt = rs232_get_portList(portList, _countof(portList));

      status = select_indexFromList( portList, NULL, cnt, true,&choice);

      if(status != MENU_OK)
      {
        break;
      }
        rs232->port = choice;
        save_config_sensor();

      break;

    case RS232_SET_BAUD:
      status = input_decimal_prompt("통신속도",&dec,9600, 115200);
      if(status != MENU_OK)
        break;
        rs232->baud = dec;
        save_config_sensor();
      break;
    case RS232_SET_PARITY:
      status = select_indexFromList(rs232ParityList, NULL, _countof(rs232ParityList), true,&choice);
      if(status != MENU_OK)
        break;

        rs232->parityIdx = choice;
        save_config_sensor();
      break;
    default:
      break;
  }
  return status;
}

#define RS485_SET_PORT 0
#define RS485_SET_BAUD 1
#define RS485_SET_PARITY 2

int32_t rs485_config_set( sensor_t *sensor, uint8_t cnt)
{
  int32_t status=0;
  int32_t dec;
  int32_t choice;
  rs485_config_t *rs485;
  const char *portList[10];

  rs485 = get_sensor_config(sensor);
  if (rs485 == NULL)
  {
    return 0;
  }
  switch (cnt)
  {
    case RS485_SET_PORT:

      cnt = rs485_get_portList(portList, _countof(portList));
      status = select_indexFromList( portList, NULL, cnt, true,&choice);
      if(status != MENU_OK)
        break;

        rs485->port = choice;
        save_config_sensor();

      break;

    case RS485_SET_BAUD:
      status = input_decimal_prompt("통신속도",&dec,9600, 115200);
      if(status != MENU_OK)
        break;

        rs485->baud = dec;
        save_config_sensor();


      break;
    case RS485_SET_PARITY:
      status = select_indexFromList(rs232ParityList, NULL, _countof(rs232ParityList), true,&choice);
      if(status != MENU_OK)
        break;

        rs485->parityIdx = choice;
        save_config_sensor();

      break;
    default:
      break;
  }

  return status;
}

int32_t hjwind_config_set(  sensor_t *sensor, uint8_t munu_index)
{
  int32_t status=0;
  int32_t choice;
  const char *portList[10];
  int32_t dec;
  uint16_t port_cnt;

  hjwindspeed_config_t *hjwind;

  hjwind = get_sensor_config(sensor);
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
      port_cnt = rs485_get_portList(portList, _countof(portList));
      status = select_indexFromList( portList, NULL, port_cnt, true,&choice);
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

int32_t hjwinddir_config_set( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status = 0;
  int32_t choice;
  hjwindspeed_config_t *hjwind;
  const char *portList[10];

  uint16_t port_cnt;

  hjwind = get_sensor_config(sensor);
  if (hjwind == NULL)
  {
    return 0;
  }
  switch (menu_index)
  {
    case HJWIND_DIR_CFG_PORT:
      port_cnt = rs485_get_portList(portList, _countof(portList));

      status = select_indexFromList( portList, NULL, port_cnt, true,&choice);
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

int32_t hjtemp_config_set( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  hjtemp_config_t *hjtemp;
  const char *portList[10];
  uint16_t portListCnt;

  hjtemp = get_sensor_config(sensor);
  if (hjtemp == NULL)
  {
    return 0;
  }

  switch (menu_index)
  {
    case HJTEMP_CFG_PHYSICAL_LAYER:
      status = select_indexFromList( physical_list, NULL, _countof(physical_list), true,&choice);
      if (status!= MENU_OK)
        break;
        hjtemp->physical_layer = (ePHYSOCAL_LAYER_t)(choice );
        save_config_sensor();
      break;
    case HJTEMP_CFG_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        status = select_indexFromList( portList, NULL, portListCnt, true,&choice);
  
        if(status != MENU_OK)
          break;
          hjtemp->port = choice;
          save_config_sensor();
      }
      else
      {
        portListCnt = rs485_get_portList(portList, _countof(portList));
        status = select_indexFromList( portList, NULL, portListCnt, true,&choice);
      if (status!= MENU_OK)
        break;

          hjtemp->port = choice;
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

int32_t hjhumi_config_set( sensor_t *sensor, uint8_t menu_index)
{
  const char *portList[10];
  uint16_t portListCnt;
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  hjtemp_config_t *hjtemp;
  float f_offset;

  hjtemp = get_sensor_config(sensor);
  if (hjtemp == NULL)
  {
    ERROR_PRINTF("화진 온습도 설정값 NULL");    
    return MENU_OK;
  }

  switch (menu_index)
  {
    case HJTEMP_CFG_PHYSICAL_LAYER:
      status = select_indexFromList(physical_list, NULL, _countof(physical_list), true,&choice);
      if(status !=MENU_OK)
        break;
        hjtemp->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
        save_config_sensor();
        break;
    case HJTEMP_CFG_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        status = select_indexFromList( portList, NULL, portListCnt, true,&choice);
        
        if(status !=MENU_OK)
        break;

          hjtemp->port = choice;
          save_config_sensor();
      }
      else
      {
        portListCnt = rs485_get_portList(portList, _countof(portList));
        status = select_indexFromList( portList, NULL, portListCnt, true,&choice);

        if(status !=MENU_OK)
        break;
        hjtemp->port = choice;
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

int32_t ott_smp3_config_set( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t choice;
  int32_t dec = 0;
  ott_smp3_config_t *ott;
  const char *portList[10];
  uint16_t portListCnt;

  ott = get_sensor_config(sensor);
  if (ott == NULL)
  {
    ERROR_PRINTF("OTT 일사 설정값 NULL");
    return 0;
  }

  switch (menu_index)
  {
    case OTT_SMP3_CFG_PORT:
      portListCnt = rs485_get_portList(portList, _countof(portList));
      status = select_indexFromList( portList, NULL, portListCnt, true,&choice);

      if (status  != MENU_OK)
        break;

        ott->port = choice;
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

int32_t rain_present_config_set(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t dec = 0;
  rain_present_config_t *rain_present;

  rain_present = get_sensor_config(sensor);
  if (rain_present == NULL)
  {
    ERROR_PRINTF("강우 감지 NULL");
    return 0;
  }

  switch (menu_index)
  {
    case RAIN_PRESENT_DELAY:
      status = input_decimal_prompt("지연시간(s)", &dec, 1, 10);
      if (status != MENU_OK)
        break;
      rain_present->delay = dec;
      save_config_sensor();

      break;

  }

  return status;
}

#define GENERAL_FREQ_CHANNEL 0
#define GENERAL_FREQ_SCALE_FACTOR 1
int32_t general_freq_config_set(sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  float factor = 0;
  frequency_config_t *freq;
  int dec;

  freq = get_sensor_config(sensor);
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
      status = input_float_prompt("변환식 factor", -100000, 100000, &factor);
      if (status != MENU_OK)
        break;
      freq->scale_factor = factor;
      save_config_sensor();
      break;
  }

  return status;
}

int32_t hjsnow_config_set( sensor_t *sensor, uint8_t menu_index)
{
  int32_t status;
  int32_t choice;
  hjsnow_config_t *hjsnow;
  const char *portList[10];
  uint16_t portCnt;

  hjsnow = get_sensor_config(sensor);
  if (hjsnow == NULL)
  {
    ERROR_PRINTF("화진 적설설 설정값 NULL");
    return 0;
  }
  switch (menu_index)
  {
    case HJSNOW_CFG_MENU_PHY:
      status = select_indexFromList(physical_list, NULL, _countof(physical_list), true,&choice);
      if (status != MENU_OK)
      break;
 
        hjsnow->physical_layer = (ePHYSOCAL_LAYER_t)(choice);
        save_config_sensor();

      break;
    case HJSNOW_CFG_MENU_PORT:
      if (hjsnow->physical_layer == ePHYSICAL_RS232)
      {
        portCnt = rs232_get_portList(portList, _countof(portList));
        status = select_indexFromList( portList, NULL, portCnt, true,&choice);
        if (status !=MENU_OK)
        break;
   
         hjsnow->port = choice;
         save_config_sensor();
        }
      else
      {
        portCnt = rs485_get_portList(portList, _countof(portList));
        status = select_indexFromList( portList, NULL, portCnt, true,&choice);
        if (status != MENU_OK)
        break;
    
          hjsnow->port = choice;
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



int32_t adc_config_set( sensor_t *sensor, uint8_t menu_index)
{
  
  int32_t dec;
  adc_config_t *adc;

  int status=0;
  int choice;

  adc = get_sensor_config(sensor);
  switch (menu_index)
  {
    case ADC_SET_CH_MODE:  // 1.채널 모드
      status = select_indexFromList(adcChModeList, NULL, _countof(adcChModeList), true,&choice);
      if (status !=MENU_OK)
        break;


        adc->mode =choice;
        save_config_sensor();

      break;
    case ADC_SET_CHANNLEL:  // channel;
      status = input_decimal_prompt("채널",&dec, 0, 17);
      if(status !=MENU_OK)
      break;

        adc->channel = dec;
        save_config_sensor();

      break;
    case ADC_SET_HIGHSCALE:  // hish cale;
      status = input_decimal_prompt("HIGH SCALE",&dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->highScale = dec;
        save_config_sensor();

      break;
    case ADC_SET_LOWSCALE:  // low cale;
      status = input_decimal_prompt("LOW SCALE", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->lowScale = dec;
        save_config_sensor();

      break;
    case ADC_SET_SCALE:  // ale;
      status = input_decimal_prompt("SCALE", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->scale = dec;
        save_config_sensor();

      break;

    case ADC_SET_OUTMAXVOLT:
      status = input_decimal_prompt("OUT VOLTAGE MAX(mv)", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->outMaxV = dec;
        save_config_sensor();

      break;

    case ADC_SET_OUTMINVOLT:
      status = input_decimal_prompt("OUT VOLTAGE MIN(mv)", &dec, -1000000, 1000000);
      if (status != MENU_OK)
        break;

        adc->outMinV = dec;
        save_config_sensor();

      break;

  }

  return status;
}

int32_t rain_reed_config_set( sensor_t *sensor, uint8_t cnt)
{
  return 0;
}

int32_t rain_hall_config_set( sensor_t *sensor, uint8_t cnt)
{
  return 0;
}






/*
센서 모델과 모델 설정 함수 연결

센서가 추가되거나 센서고유의 설정값을 변경하려면 처리 함수를 작성해야한다.
*/
const config_sen_func_t sen_func[] = {
    {.sensorType = S_T_ADC, .config_set = adc_config_set},
    {.sensorType = S_T_TEMP_232, .config_set = rs232_config_set},
    {.sensorType = S_T_TEMP_485, .config_set = rs485_config_set},
    {.sensorType = S_T_RAIN_HALL_05MM, .config_set = rain_hall_config_set},
    {.sensorType = S_T_RAIN_HALL_1MM, .config_set = rain_hall_config_set},
    {.sensorType = S_T_RAIN_REED_05MM, .config_set = rain_reed_config_set},
    {.sensorType = S_T_RAIN_REED_1MM, .config_set = rain_reed_config_set},
    {.sensorType = S_T_GENERAL_232, .config_set = rs232_config_set},
    {.sensorType = S_T_WIND_DIRECTION_HJ_485, .config_set = hjwinddir_config_set},
    {.sensorType = S_T_WIND_SPEED_HJ_485, .config_set = hjwind_config_set},
    {.sensorType = S_T_SNOW_HJ, .config_set = hjsnow_config_set},
    {.sensorType = S_T_GENERAL_485, .config_set = rs485_config_set},
    {.sensorType = S_T_TEMPERATURE_HJ, .config_set = hjtemp_config_set},
    {.sensorType = S_T_HUMINITY_HJ, .config_set = hjhumi_config_set},
    {.sensorType = S_T_SOLAR_RADIATION_OTT_SMP3, .config_set = ott_smp3_config_set},
    {.sensorType = S_T_RAIN_PRESENT_DI, .config_set = rain_present_config_set},
    {.sensorType = S_T_FREQ, .config_set = general_freq_config_set}};

int32_t sensor_set( sensor_t *sensor, uint8_t choice)
{
  int32_t status=MENU_OK;

  for (int i = 0; i < _countof(sen_func); i++)
  {  // 센서마다 고유의 처리 함수를 사용한다.
    if (sen_func[i].sensorType == sensor->type)
    {
      status = sen_func[i].config_set( sensor, choice -1);
      break;
    }
  }

  return status;
}

/*
센서 설정


*/
int32_t menu_sensor_default_2( eSENSOR_LIST_t list)
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
    status = select_indexMenu(sensor,&choice);
    if (status != MENU_OK)
        break; 
    
    switch (choice)
    {
      case 0:  // 센서가 사용하고자하는 센서 타입을 설정한다.
               // 센서마다 지원가능한 목록을 넘겨지고 출력하여 선택하도록 한다.
        status =sensor_type_set( sensor, supported_sensors[list].list, supported_sensors[list].cnt);
        break;
      default:  // 센서 타입이 아닌 센서 고유 속성들은 이 함수 에서 처리한다.
        // 현재의 센서 정보와 사용자가 수정하고자한 항목 번호를 넘긴다.
        status = sensor_set(sensor, choice);  // 센서별 설정값 변경
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
int32_t print_menu_sensor(void)
{
  char opt[25];
  int32_t cnt = 0;
  int i = 0;

  io_printf("\r\n");

#if 1
  cnt = _countof(sensor_name_list) / 2;

  for (i = 0; i < cnt; i++)
  {
    make_option(&get_config_app()->sensor[i], opt, sizeof(opt));
    io_printf("%2d.%-14s:%-20s %-22s,  ", i, sensor_name_list[i],
              ITEM_LIST(get_config_app()->sensor[i].type, g_sensor_model_list), opt);
    make_option(&get_config_app()->sensor[i + cnt], opt, sizeof(opt));
    io_printf("%2d.%-14s:%-20s %-22s\r\n", i + cnt, sensor_name_list[i + cnt],
              ITEM_LIST(get_config_app()->sensor[i + cnt].type, g_sensor_model_list), opt);
  }

#endif
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
    status = select_indexFromList( NULL, print_menu_sensor, 0, false,&choice);
    if (status != MENU_OK)
          break;
    status = menu_sensor_default_2((eSENSOR_LIST_t)(choice));
    if(status ==MENU_ABORT)
    break;
  }while(1);

  io_printf("장비리셋 후 설정값이 적용됩니다.\r\n");
  return status;
}