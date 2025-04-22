
#include "app_console.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "app_adc.h"
#include "app_bsp.h"
#include "app_charger.h"
#include "app_dataLogging.h"
#include "app_di.h"
#include "app_flash.h"
#include "app_logging.h"
#include "app_rs232.h"
#include "app_rs485.h"
#include "app_rtc.h"
#include "app_sensor.h"
#include "app_version.h"
#include "aws_data.h"
#include "boot_version.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "config_adc.h"
#include "config_nvm.h"
#include "dev_io.h"
#include "driver_485.h"
#include "mcu_debug.h"
#include "system_err.h"
#include "task_logging.h"
#include "terminal.h"
#include "usDelay.h"
#include "utile.h"
#include "utile_time.h"
#include "vt100_command.h"
#include "ymodem.h"
#include "task_direct.h"
#include "task_tcpServer.h"
#include "task_measure.h"
#include "console_cali.h"


#define EXIT_PROGRAM -3
#define EXIT_BACK -1

#define ITEM_LIST(cnt, list) cnt >= _countof(list) ? g_unknown : (char *)list[cnt]

// »ç¿ë°¡´ÉÇÑ ½Ì±Û Ã¤³Î ¼³Á¤Á¤
const bool single_en[32] = {1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0,
                            1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0};

/// @brief
typedef int32_t (*menu_func)(p_shell_context_t);

typedef struct menuFunc_s
{
  const char *title;
  menu_func func;
} menuFunc_t;

typedef enum val_e
{
  eUINT8,
  eUINT16,
  eUINT32,
  eFLOAT
} eVAL_TYPE_t;

typedef struct select_menu_s
{
  p_shell_context_t ctx;
  const char **list;
  int32_t (*func)(p_shell_context_t ctx);
  uint8_t cnt;
  bool show;
  const menuFunc_t *menuFunc;
} select_menu_t;

const char *protocolList[] = {"kma ver 1", "kma ver 2"};
const char *cdmaModellList[] = {"TX700", "NTLE9607"};
const char *panelList[] = {"model a", "model b"};
const char *doorStatusList[] = {"´ÝÈû", "¿­¸²"};
const char *linkStatusList[] = {"-", "up", "down"};
const char *g_chgList[] = {"smart charger", "aws charger"};
const char *g_unknown = "unknown";
const char *generalStatusList[] = {"Á¤»ó", "ºñÁ¤»ó"};
const char *unusedList[] = {"¹Ì»ç¿ë"};
const char *adcChModeList[] = {"single", "diff"};
const char *rs232ParityList[] = {"none", "even", "odd"};
const char *enableList[] = {"¹Ì»ç¿ë", "»ç¿ë"};
const char *ethModeList[] = {"Å¬¶óÀÌ¾ðÆ®", "¼­¹ö"};

int32_t print_common_cfg(p_shell_context_t ctx, sensor_t *sensor, uint8_t c);

void make_comList(char *out, uint16_t outsize)
{
  int32_t len = 0;
  if (config.eth_use)
  {
    len = snprintf(&out[len], outsize - len, "[ETH]");
  }
  if (config.cdma_use)
  {
    len += snprintf(&out[len], outsize - len, "[CDMA]");
  }
  if (config.direct_use)
  {
    len += snprintf(&out[len], outsize - len, "[DIRECT]");
  }

  if (len == 0)
  {
    snprintf(&out[len], outsize - len, "¹Ì»ç¿ë");
  }
}

void update_val(void *val, void *target, eVAL_TYPE_t type)
{
  switch (type)
  {
    case eUINT8:
      *(uint8_t *)target = *(uint8_t *)val;
      break;
    case eUINT16:
      *(uint16_t *)target = *(uint16_t *)val;
      break;
    case eUINT32:
      *(uint32_t *)target = *(uint32_t *)val;
      break;
    case eFLOAT:
      *(float *)target = *(float *)val;
      break;
    default:
      break;
  }
}

void print_items(char *title, char *items[], uint8_t itmeCnt)
{
  debug_printf("\r\n");
  debug_printf("(0lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk(B\r\n");
  debug_printf("(0x(B %s(0x(B\r\n", title);
  debug_printf("(0tqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqu(B\r\n");
  for (int i = 0; i < itmeCnt; i++)
  {
    debug_printf("(0x(B %s(0x(B\r\n", items[i]);
  }
  debug_printf("(0mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj(B\r\n");
}

int32_t select_indexFromList(p_shell_context_t ctx, const char *list[],
                             int32_t (*func)(p_shell_context_t), uint16_t listCnt, bool number)
{
  int cnt;
  int index = 0;
  int funcCnt = 0;
  int indexMax;
  do
  {
    if (list)
    {
      // ¸ñ·ÏÀ» Ãâ·ÂÇÑ´Ù.
      for (int i = 0; i < listCnt; i++)
      {
        if (number == true)
        {
          ctx->printf("%d.%s\r\n", i, list[i]);
        }
        else
        {
          ctx->printf("%s\r\n", list[i]);
        }
      }
      indexMax = listCnt;
    }
    if (func)
    {
      funcCnt = func(ctx);
      indexMax = funcCnt;
    }

    if (func == NULL && list == NULL)
    {
      indexMax = listCnt;
    }

    vt100_printfColor(GREEN, "¹øÈ£¸¦ ¼±ÅÃÇØ ÁÖ¼¼¿ä:");
    // »ç¿ëÀÚ·Î ºÎÅÍ ¸Þ´º¸¦ ¼±ÅÃ ¹Þ´Â´Ù.
    cnt = console_scanf("%d", &index);  //-1 ctrl+c, 0 enter esc
    ctx->printf("\r\n");
    if (cnt == 1)
    {
      if (index < indexMax)
        break;
    }
    else if (cnt == EXIT_BACK)
    {
      return EXIT_BACK;
    }
    else if (cnt == EXIT_PROGRAM)
    {
      return EXIT_PROGRAM;
    }
    // ctx->printf("Invalid input. Please select a menu number again.\r\n");
    vt100_printfColor(RED, "À¯È¿ÇÑ ¹øÈ£°¡ ¾Æ´Õ´Ï´Ù\r\n");
  } while (1);

  return (index + 1);
}

int32_t select_indexMenu(p_shell_context_t ctx, sensor_t *sensor)
{
  int cnt;
  int index = 0;
  int funcCnt = 0;
  int indexMax;

  do
  {
    funcCnt = print_common_cfg(ctx, sensor, 0);
    indexMax = funcCnt;

    vt100_printfColor(GREEN, "¹øÈ£¸¦ ¼±ÅÃÇØ ÁÖ¼¼¿ä:");
    // »ç¿ëÀÚ·Î ºÎÅÍ ¸Þ´º¸¦ ¼±ÅÃ ¹Þ´Â´Ù.
    cnt = console_scanf("%d", &index);  //-1 ctrl+c, 0 enter esc
    ctx->printf("\r\n");
    if (cnt == 1)
    {
      if (index < indexMax)
        break;
    }
    else if (cnt == EXIT_BACK)
    {
      return EXIT_BACK;
    }
    else if (cnt == EXIT_PROGRAM)
    {
      return EXIT_PROGRAM;
    }
    vt100_printfColor(RED, "À¯È¿ÇÑ ¹øÈ£°¡ ¾Æ´Õ´Ï´Ù\r\n");
  } while (1);

  return (index + 1);
}

bool wait_break(uint32_t timeoutms)
{
  int32_t ch;
  osDelay(timeoutms);
  ch = DbgConsole_GetcharNonBlocking();
  if (ch == -1)
  {
    return true;
  }

  return false;
}

#define DISP_WIDTH 26

extern uint32_t g_debug_elased_time;
int32_t print_systemInfo(uint16_t row, uint16_t column)
{
  uint8_t line = row + 3;
  char buff[30];

  snprintf(buff, sizeof(buff), "%04d-%02d-%02d %02d:%02d:%02d\r\n", Date_Time.Year, Date_Time.Month,
           Date_Time.Day, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);

  vt100_print_frame(row, column, "½Ã½ºÅÛ", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "%s\r\n", buff);
  vt100_print_bar(line++, column, -DISP_WIDTH, "¹® »óÅÂ   :%s\r\n",
                  ITEM_LIST(IS_DOOR_OPENED(), doorStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "ÀúÀå ±â´É :%s\r\n",
                  ITEM_LIST(IS_DATA_ERR(), generalStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "Àåºñ Àü¿ø :%5.2f V\r\n", read_battery());
  vt100_print_bar(line++, column, -DISP_WIDTH, "Àåºñ ¿Âµµ :%5.2f C\r\n", read_temperature());

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return 4 + 2;
}

int32_t print_chargerInfo(uint16_t row, uint16_t column)
{
  char buff[10];

  uint8_t line = row + 3;
  uint8_t err;

  read_chargerStatus(buff, sizeof(buff));
  vt100_print_frame(row, column, "ÃæÀü±â", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "»óÅÂ           :%s\r\n", buff);

  if (is_chargerValid())
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "  ÃæÀü Àü¾Ð(V) :%.2f\r\n",
                    read_solarVoltage1(&err));
    vt100_print_bar(line++, column, -DISP_WIDTH, "  ÃæÀü Àü·ù(A) :%.2f\r\n",
                    read_solarCurrrent1(&err));
    vt100_print_bar(line++, column, -DISP_WIDTH, "¹èÅÍ¸® Àü¾Ð(V) :%.2f\r\n",
                    read_batteryVoltage1(&err));
    vt100_print_bar(line++, column, -DISP_WIDTH, " ºÎÇÏ1 Àü·ù(A) :%.2f\r\n",
                    read_loadCurrent1(&err));
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "  ÃæÀü Àü¾Ð(V) :--\r\n", 0);
    vt100_print_bar(line++, column, -DISP_WIDTH, "  ÃæÀü Àü·ù(A) :--\r\n", 0);
    vt100_print_bar(line++, column, -DISP_WIDTH, "¹èÅÍ¸® Àü¾Ð(V) :--\r\n", 0);
    vt100_print_bar(line++, column, -DISP_WIDTH, " ºÎÇÏ1 Àü·ù(A) :--\r\n", 0);
  }
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return 4 + 2;
}
int32_t print_ethInfo(uint16_t row, uint16_t column)
{
  char buff[30];

  uint8_t line = row + 3;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "ÀÌ´õ³Ý", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "¸µÅ©  :%s\r\n",
                  ITEM_LIST(get_direct_system()->link_status + 1, linkStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "¼Û½Å  :%d\r\n", get_tcp_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "¼ö½Å  :%d\r\n", get_tcp_system()->rx_cnt);

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

int32_t print_cdmaInfo(uint16_t row, uint16_t column)
{
  char buff[30];
  char num[20];
  uint8_t line = row + 3;
  int8_t rssi;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "CDMA", '+', '|', '-', DISP_WIDTH, WHITE);
  if (System.cdma_link_status == -1)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "¸µÅ©    :-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "¸µÅ©    :%s\r\n",
                    ITEM_LIST(System.cdma_link_status + 1, linkStatusList));
  }
  if (System.cdma_num[0] != '0')
  {
    num[0] = '-';
    num[1] = 0;
  }
  else
  {
    snprintf(num, sizeof(num), "%s", System.cdma_num);
  }
  vt100_print_bar(line++, column, -DISP_WIDTH, "ÀüÈ­¹øÈ£:%s\r\n", num);
  if (System.cdma_rssi == -1)
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼ö½Å°¨µµ:-\r\n");
  }
  else
  {
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼ö½Å°¨µµ:%d\r\n", System.cdma_rssi);
  }

  vt100_print_bar(line++, column, -DISP_WIDTH, "¼Û½Å    :%d\r\n", get_tcp_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "¼ö½Å    :%d\r\n", get_tcp_system()->rx_cnt);
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

int32_t print_directInfo(uint16_t row, uint16_t column)
{
  char buff[30];
  char num[20];
  uint8_t line = row + 3;
  int8_t rssi;

  make_comList(buff, sizeof(buff));
  vt100_print_frame(row, column, "DIRECT", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++, column, -DISP_WIDTH, "¸µÅ©    :%s\r\n",
                  ITEM_LIST(get_direct_system()->link_status + 1, linkStatusList));
  vt100_print_bar(line++, column, -DISP_WIDTH, "¼Û½Å    :%d\r\n", get_direct_system()->tx_cnt);
  vt100_print_bar(line++, column, -DISP_WIDTH, "¼ö½Å    :%d\r\n", get_direct_system()->rx_cnt);
  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);

  return line - (row);
}

int32_t print_awsRealLefinfo(uint16_t row, uint16_t column, uint8_t mode, void *arg)
{
  char buff[50];
  uint8_t line = row + 3;
  sensor_data_t *pdata;
  const char *aswTitleList[] = {"RAW", "Æò±Õ","1ºÐ", "10ºÐ","ÇÑ½Ã°£"};
  kma_data_t *pkma=NULL;


  switch (mode)
  {
  case 0: //raw
  pkma = &g_kma_raw;
  break;
  case 1://avg
    pkma = &g_kma_inst;
    break;
  case 2://1min
    pkma = &g_kma_1min;
    break;
  case 3://10min
    pkma = &g_kma_10min;
    break;
  case 4:  // 10min
    pkma = &g_kma_hour;
    break;
  }

    snprintf(buff, sizeof(buff), "AWS %s %.2fms", aswTitleList[mode],
              (float)g_debug_elased_time / 1000.0f);



#define KMA_TO_TEMPERATURE(x) \
  ((float)((x - 1000) / 10.0f))  //[AWS = (°üÃø°ª+100)/10, °üÃø°ª = (x-1000)/10]
#define KMA_TO_GENERAL(x) ((float)(x / 10.0f))

#define KMA_TO_1000(x) ((float)((x - 1000) / 10.0f))

  sensor_t *p_sensor = get_config_app()->sensor;

#if 1
  vt100_print_frame(row, column, buff, '+', '|', '-', DISP_WIDTH, WHITE);
  if (p_sensor[A1_TEMPERATURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "±â¿Â          :%5.1f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->temperature));
  if (p_sensor[A2_WIND_DIRECTION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Ç³Çâ          :%5.1f µµ\r\n",
                    KMA_TO_GENERAL(pkma->wind_direction_avg));
  if (p_sensor[A3_WIND_SPEED].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Ç³¼Ó          :%5.1f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_avg));
  if (p_sensor[A4_INSTANT_WIND_DIRECTION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼ø°£ Ç³Çâ     :%5.1f µµ\r\n",
                    KMA_TO_GENERAL(pkma->wind_direction_instant));
  if (p_sensor[A5_INSTANT_WIND_SPEED].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼ø°£ Ç³¼Ó     :%5.1f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_instant));
  if (p_sensor[A6_RAINFALL_DOT5_1MM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "°­¼ö·®        :%5.1f mm\r\n",
                    (float)(pkma->precipitation / 10.0f));
  if (p_sensor[A7_PRESSURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "±â¾Ð          :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->pressure));
  if (p_sensor[A8_RAIN_PRESENT].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "°­¼öÀ¯¹«      :%5s\r\n",
                    pkma->precipitation_presence == 10 ? "À¯" : "¹«");
  if (p_sensor[A9_SNOW_DEPTH].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Àû¼³          :%5d mm\r\n",
                    (int)(pkma->snowfall / 10.0f));
  if (p_sensor[A10_RELATIVE_HUMIDITY].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "»ó´ë½Àµµ      :%5.1f %%\r\n",
                    KMA_TO_GENERAL(pkma->relative_humidity));
  if (p_sensor[A11_RAINFALL_DOT1MM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "°­¼ö·®        :%5d mm\r\n",
                    pkma->precipitation_fine);
  if (p_sensor[B1_SOLAR_RADIATION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÀÏ»ç          :%5.2f MJ/m2\r\n",
                    pkma->solar_radiation / 100.f);  // Ç¥Çö¹üÀ§	¡æ	0	¢¦
                                                        // 32767 [´©Àû	°ª(MJ/m2)	¡¿	100]
  if (p_sensor[B2_SUNSHINE_DURATION].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÀÏÁ¶          :%5d s\r\n",
                    (int)pkma->sunshine_duration);  // Ç¥Çö¹üÀ§	¡æ	0	¢¦
                                                       // 65535	[´©Àû½Ã°£(ÃÊ	´ÜÀ§)
  if (p_sensor[B3_GROUND_TEMPERATURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Áö¸é¿Âµµ      :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->surface_temperature));  // Ç¥Çö¹üÀ§	¡æ	500
                                                                        // ¢¦	2000	[(°üÃø°ª
                                                                        // £«	100)	¡¿	10
  if (p_sensor[B4_SURFACE_TEMPERATURE].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÃÊ»ó¿Âµµ      :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->grass_temperature));
  if (p_sensor[B5_SOIL_TEMPERATURE_5CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 5cm  :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_5cm));
  if (p_sensor[B6_SOIL_TEMPERATURE_10CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 10cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_10cm));
  if (p_sensor[B7_SOIL_TEMPERATURE_20CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 20cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_20cm));
  if (p_sensor[B8_SOIL_TEMPERATURE_30CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 30cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_30cm));
  if (p_sensor[B9_SOIL_TEMPERATURE_50CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 50cm :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_50cm));
  if (p_sensor[B10_SOIL_TEMPERATURE_100CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 1m   :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_1m));
  if (p_sensor[B11_SOIL_TEMPERATURE_150CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 1.5m :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_1_5m));
  if (p_sensor[B12_SOIL_TEMPERATURE_300CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 3m   :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_3m));
  if (p_sensor[B13_SOIL_TEMPERATURE_500CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÁöÁß¿Âµµ 5cm  :%5.2f C\r\n",
                    KMA_TO_TEMPERATURE(pkma->soil_temperature_5m));
  if (p_sensor[C1_CLOUD_BASE1].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "1Ãþ ¿î°í      :%dm\r\n",
                    (pkma->cloud_height_1st));  // Ç¥Çö¹üÀ§	¡æ	1
                                                   // ¢¦ 8000	[°üÃø°ª(m)]
  if (p_sensor[C2_CLOUD_BASE2].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "2Ãþ ¿î°í      :%dm\r\n",
                    (pkma->cloud_height_2nd));  // Ç¥Çö¹üÀ§	¡æ	1
                                                   // ¢¦ 8000	[°üÃø°ª(m)]
  if (p_sensor[C3_CLOUD_BASE3].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "3Ãþ ¿î°í      :%dm\r\n",
                    (pkma->cloud_height_3rd));  // Ç¥Çö¹üÀ§	¡æ	1
                                                   // ¢¦ 8000	[°üÃø°ª(m)]
  if (p_sensor[27].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¿î·®          :%5d\r\n",
                    (pkma->cloud_amount));  // Ç¥Çö¹üÀ§	¡æ	0
                                               // ¢¦	10	(°üÃø°ª)
  if (p_sensor[28].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "½ÃÁ¤          :%5d m\r\n",
                    (pkma->visibility));  // Ç¥Çö¹üÀ§	¡æ	1
                                             // ¢¦	50000	[°üÃø°ª(m)]
  if (p_sensor[29].type)
    vt100_print_bar(
        line++, column, -DISP_WIDTH, "PM10          :%5.1f ¥ìg/§©\r\n",
        KMA_TO_GENERAL(pkma->pm10_concentration));  // Ç¥Çö¹üÀ§	¡æ	1	¢¦
                                                       // 3599	[°üÃø°ª(¥ìg/§©)	¡¿	10]
  if (p_sensor[30].type)
    vt100_print_bar(
        line++, column, -DISP_WIDTH, "PM2.5         :%5.1f ¥ìg/§©\r\n",
        KMA_TO_GENERAL(pkma->pm25_concentration));  // Ç¥Çö¹üÀ§	¡æ	1	¢¦
                                                       // 3599	[°üÃø°ª(¥ìg/§©)	¡¿	10]
  if (p_sensor[31].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼øº¹»ç        :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->net_radiation));  // Ç¥Çö¹üÀ§	¡æ	0
                                                           // ¢¦	32767	{[°üÃø°ª(W/m2)
                                                           // +	1000]	¡¿	10}
  if (p_sensor[32].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÀüÃµº¹»ç      :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->total_radiation));  // Ç¥Çö¹üÀ§	¡æ	0
                                                             // ¢¦	32767	{[°üÃø°ª(W/m2)
                                                             // +	1000]	¡¿	10}
  if (p_sensor[33].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¹Ý»çº¹»ç      :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->reflected_radiation));  // Ç¥Çö¹üÀ§	¡æ	0
                                                                 // ¢¦	32767	{[°üÃø°ª(W/m2)
                                                                 // +	1000]	¡¿	10}
  if (p_sensor[34].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Á÷´ÞÀÏ»ç      :%5f W/m2\r\n",
                    KMA_TO_1000(pkma->direct_radiation));  // Ç¥Çö¹üÀ§	¡æ	0
                                                              // ¢¦	32767	{[°üÃø°ª(W/m2)
                                                              // +	1000]	¡¿	10}
  if (p_sensor[35].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "ÇöÀçÀÏ±â      :%5f\r\n",
                    pkma->current_weather);  // Ç¥Çö¹üÀ§	¡æ	0
                                                // ¢¦	99	(°üÃø°ª)
  if (p_sensor[36].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Åä¾ç¼öºÐ10cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_10cm));  // Ç¥Çö¹üÀ§	¡æ	0	¢¦
                                                                   // 1000	(°üÃø°ª	¡¿	10)
  if (p_sensor[37].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Åä¾ç¼öºÐ20cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_20cm));  // Ç¥Çö¹üÀ§	¡æ	0	¢¦
                                                                   // 1000	(°üÃø°ª	¡¿	10)
  if (p_sensor[38].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Åä¾ç¼öºÐ30cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_30cm));  // Ç¥Çö¹üÀ§	¡æ	0	¢¦
                                                                   // 1000	(°üÃø°ª	¡¿	10)
  if (p_sensor[39].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Åä¾ç¼öºÐ50cm  :%5.1f\r\n",
                    KMA_TO_GENERAL(pkma->soil_moisture_50cm));  // Ç¥Çö¹üÀ§	¡æ	0	¢¦
                                                                   // 1000	(°üÃø°ª	¡¿	10)
  if (p_sensor[40].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Á¶µµ·®        :%5.2f klux\r\n",
                    (pkma->illuminance));  // Ç¥Çö¹üÀ§	¡æ	0	¢¦	32767
                                              // (°üÃø°ª(klux)	¡¿	100)
  if (p_sensor[N6_WIND_VELOCITY_150CM].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Ç³¼Ó1.5m      :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_1_5m));  // Ç¥Çö¹üÀ§	¡æ	1	¢¦
                                                                // 1000	(°üÃø°ª	¡¿	10)
  if (p_sensor[42].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Ç³¼Ó4m        :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->wind_speed_4m));  // Ç¥Çö¹üÀ§	¡æ	1	¢¦
                                                              // 1000	(°üÃø°ª	¡¿	10)
  if (p_sensor[43].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼ø°£Ç³¼Ó1.5m  :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->instant_wind_speed_1_5m));  // Ç¥Çö¹üÀ§	¡æ
                                                                        // 1 ¢¦	1000
                                                                        // (°üÃø°ª	¡¿ 10)
  if (p_sensor[44].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "¼ø°£Ç³¼Ó4m    :%5.2f m/s\r\n",
                    KMA_TO_GENERAL(pkma->instant_wind_speed_4m));  // Ç¥Çö¹üÀ§	¡æ
                                                                      // 1	¢¦ 1000	(°üÃø°ª
                                                                      // ¡¿	10)
  if (p_sensor[45].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "±â¿Â0.5m      :%5.2f C\r\n",
                    (pkma->temperature_0_5m));  // Ç¥Çö¹üÀ§	¡æ	500	¢¦	1500
                                                   // [(°üÃø°ª	£«	100)	¡¿	10
  if (p_sensor[46].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "±â¿Â4m        :%5.2f C\r\n",
                    (pkma->temperature_4m));  // Ç¥Çö¹üÀ§	¡æ	500	¢¦	1500
                                                 // [(°üÃø°ª	£«	100)	¡¿	10
  if (p_sensor[47].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "½Àµµ0.5m      :%5.2f %%\r\n",
                    (pkma->humidity_0_5m));  // Ç¥Çö¹üÀ§	¡æ	500	¢¦	1500
                                                // [(°üÃø°ª	£«	100)	¡¿	10
  if (p_sensor[48].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "½Àµµ4m        :%5.2f %%\r\n",
                    (pkma->humidity_4m));  // Ç¥Çö¹üÀ§	¡æ	500	¢¦	1500
                                              // [(°üÃø°ª	£«	100)	¡¿	10
  if (p_sensor[49].type)
    vt100_print_bar(line++, column, -DISP_WIDTH, "Å¸ÄÚ4m        :%5d\r\n",
                    (pkma->tacometer));  // Ç¥Çö¹üÀ§	¡æ	0	¢¦	8000	(°üÃø°ª)

  vt100_print_line(line++, column, '+', '-', DISP_WIDTH);
#endif
  return 5 + 2;
}

#define KEY_UP 0x41
#define KEY_DOWN 0x42
#define KEY_LEFT 0x44
#define KEY_RIGHT 0x43
#define KEY_ENTER 0x0D

#define AWS_MODE_MAX 3


char recv_key(void)
{
  char key;
  char ch=0;

  while(1)
  {
    if(debug_recv(&ch, 1, 1000))
    {
      if(ch==0x1B || ch==0x5B)
      {
        continue;
      }
      break;
    }
    break;
  }
return ch;

}

int32_t menu_display(p_shell_context_t ctx)
{
  char key;
  int32_t line = 0;
  uint8_t awsMode = 0;

  debug_printf(VT100_CLEAR_SCREEN);
  debug_printf(VT100_CURSOR_OFF);

  do
  {
    debug_printf(VT100_CURSOR_HOME);
    debug_printf("\r\n");
    print_systemInfo(1, 0);

    line = 0;
    if (get_config_app()->cdma_use)
    {
      line = print_cdmaInfo(1, 30);
    }
    if (get_config_app()->direct_use)
    {
      line += print_directInfo(1 + line, 30);
    }
    if (get_config_app()->eth_use)
    {
      line += print_ethInfo(1 + line, 30);
    }

    print_chargerInfo(20, 0);
    print_awsRealLefinfo(1, 60, awsMode, NULL);

    key = recv_key();

    if (key == KEY_RIGHT)
    {
      if (awsMode < AWS_MODE_MAX)
      {
        awsMode++;
      }
    }
    else if (key == KEY_LEFT)
    {
      if (awsMode > 0)
      {
        awsMode--;
      }
    }

    if (key == ASCII_CODE_CTRL_Q)
    {
      break;
    }
  } while (1);
  vt100_print(50, 0, "\r\n");
  debug_printf(VT100_CURSOR_ON);
  return 0;
}

int32_t input_date(p_shell_context_t ctx, DATE_TIME_BUF *nt)
{
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
  int cnt;

  ctx->printf("format:YYYY-MM-DD hh:mm:ss,2020-01-01 00:11:22\r\n");

  cnt = console_scanf("%04d-%02d-%02d %02d:%02d:%02d", &year, &month, &day, &hour, &min, &sec);

  if (cnt == 6)
  {
    nt->Year = year;
    nt->Month = month;
    nt->Day = day;
    nt->Hour = hour;
    nt->Min = min;
    nt->Sec = sec;

    return 6;
  }

  return cnt;
}

int input_decimal(p_shell_context_t ctx, int32_t start, int32_t stop, int32_t *dec)
{
  int32_t cnt;

  ctx->printf("¹üÀ§:%d~%d\r\n", start, stop);
  vt100_printfColor(GREEN, "°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%d", dec);
  if (cnt == 1)
  {
    if (*dec >= start && *dec <= stop)
    {
      return 1;
    }
    else
    {
      vt100_printfColor(RED, "ÀÔ·Â°ªÀÇ ¹üÀ§¸¦ È®ÀÎÇØ ÁÖ¼¼¿ä\r\n");
      return 0;
    }
  }

  return cnt;
}

int32_t input_use(p_shell_context_t ctx, bool *en)
{
  int32_t cnt;
  int32_t dec;
  ctx->printf("0:¹Ì»ç¿ë\r\n");
  ctx->printf("1:»ç¿ë\r\n");
  vt100_printfColor(GREEN, "¹øÈ£¸¦ ¼±ÅÃÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%d", &dec);
  if (cnt == 1)
  {
    if (dec >= 0 && dec <= 1)
    {
      *en = (bool)dec;
      return 1;
    }
    else
    {
      ctx->printf("ÀÔ·Â ¹üÀ§¸¦ È®ÀÎÇØÁÖ¼¼¿ä\r\n");
      return 0;
    }
  }

  return cnt;
}

int input_digit(p_shell_context_t ctx, int32_t start, int32_t stop, void *target, eVAL_TYPE_t type)
{
  int32_t cnt;
  int8_t i8val;
  int16_t i16val;
  int32_t i32val;

  ctx->printf("\r\n¹üÀ§:%d~%d\r\n", start, stop);
  vt100_printfColor(GREEN, "°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%d", &i32val);
  if (cnt == 1)
  {
    if (i32val < start || i32val > stop)
    {
      vt100_printfColor(RED, "ÀÔ·Â°ªÀ» ¹üÀ§¸¦ È®ÀÎÇØ ÁÖ¼¼¿ä\r\n");
      return 0;
    }
    switch (type)
    {
      case eUINT8:
        i8val = i32val;
        update_val(&i8val, target, type);
        break;
      case eUINT16:
        i16val = i32val;
        update_val(&i16val, target, type);
        break;
      case eUINT32:
        update_val(&i32val, target, type);
        break;
    }

    return 1;
  }

  return cnt;
}

int32_t print_menu_system(p_shell_context_t ctx)
{
  char buff[50];
  int cnt = 0;

  make_timeToStr(&Date_Time, buff, sizeof(buff));
  ctx->printf("%2d.time        :%s\r\n", cnt++, buff);
  ctx->printf("%2d.id          :%d\r\n", cnt++, get_config_app()->id);
  ctx->printf("%2d.password    :%d\r\n", cnt++, get_config_app()->password);
  ctx->printf("%2d:charger type:%s\r\n", cnt++, ITEM_LIST(get_config_app()->charger_model, g_chgList));

  return cnt;
}

int32_t menu_system(p_shell_context_t ctx)
{
  int cnt;
  int32_t dec;
  DATE_TIME_BUF nt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_system, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    switch (cnt)
    {
      case 0:
        cnt = input_date(ctx, &nt);
        if (cnt > 0)
        {
          rtc_set(&nt);
          rtc_update();
        }
        break;
      case 1:  // id
        cnt = input_decimal(ctx, 0, 9999, &dec);
        if (cnt)
        {
          config.id = dec;
          WRITE_CFG(id);
        }
        break;
      case 2:  // password
        cnt = input_decimal(ctx, 0, 9999, &dec);
        if (cnt)
        {
          config.password = dec;
          WRITE_CFG(password);
        }
        break;
      case 3:  // charger type
        cnt = select_indexFromList(ctx, g_chgList, NULL, sizeof(g_chgList) / sizeof(g_chgList[0]),
                                   true);
        if (cnt > 0)
        {
          cnt--;
          config.charger_model = cnt;
          WRITE_CFG(charger_model);
        }
        break;
    }
  } while (1);
}

void make_option(sensor_t *sensor, char *out, uint16_t outSize)
{
  void *cfg;
  const char *list[10];

  out[0] = 0;

  cfg = get_sensor_config(sensor);

  if (cfg == NULL)
  {
    snprintf(out, outSize, "%s","NULL");
    return;
  }

  switch (sensor->type)
  {
    case S_T_SNOW_HJ_232:
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
    case S_T_SNOW_HJ_485:

    {
      rs485_config_t *rs485_cfg = (rs485_config_t *)cfg;
      rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[rs485_cfg->port]);
    }
    break;
    case S_T_ADC:
    {
      adc_config_t *adc_cfg = (adc_config_t *)cfg;
      snprintf(out, outSize, "[%s.%d]", adcChModeList[adc_cfg->mode], adc_cfg->channel);
    }
    break;
    case S_T_HUMI_HJ_485:
    case S_T_TEMPERATURE_HJ_485:
    {
      hjtemp_config_t *hjtemp = (hjtemp_config_t *)cfg;
      rs485_get_portList(list, sizeof(list));
      snprintf(out, outSize, "[%s]", list[hjtemp->rs485_port]);
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
    default:
      out[0] = 0;
      break;
  }
}

/*
 0.±â¿Â          :¹Ì»ç¿ë                                  ,  32.ÀüÃµº¹»ç      :¹Ì»ç¿ë
 1.Ç³Çâ          :¹Ì»ç¿ë                                  ,  33.¹Ý»çº¹»ç      :¹Ì»ç¿ë
 2.Ç³¼Ó          :¹Ì»ç¿ë                                  ,  34.Á÷´Þ          :¹Ì»ç¿ë
...°è¼Ó

¿©±â¼­ ¼öÁ¤ÇÏ°í ½ÍÀº ¼¾¼­¹øÈ£¸¦ ÀÔ·Â
¼¾¼­¹øÈ£´Â Á¤ÇØÁø ¼ø¼­´ë·Î ÀÔ·ÂµÇ¾î¾ßÇÔ

 */
int32_t print_menu_sensor(p_shell_context_t ctx)
{
  char opt[20];
  int32_t cnt = 0;
  int i = 0;

  ctx->printf("\r\n");

#if 1
  cnt = _countof(sensor_name_list) / 2;

  for (i = 0; i < cnt; i++)
  {
    make_option(&get_config_app()->sensor[i], opt, sizeof(opt));
    ctx->printf("%2d.%-14s:%-24s %-15s,  ", i, sensor_name_list[i],
                ITEM_LIST(get_config_app()->sensor[i].type, g_sensor_model_list), opt);
    make_option(&get_config_app()->sensor[i + cnt], opt, sizeof(opt));
    ctx->printf("%2d.%-14s:%-24s %-15s\r\n", i + cnt, sensor_name_list[i + cnt],
                ITEM_LIST(get_config_app()->sensor[i + cnt].type, g_sensor_model_list), opt);
  }

#endif
  cnt = _countof(sensor_name_list);
  return cnt;
}

uint8_t print_rs232_cfg(p_shell_context_t ctx, rs232_config_t *rs232_config, uint8_t cnt)
{
  const char *portNameList[10];

  rs232_get_portList(portNameList, _countof(portNameList));
  ctx->printf("%2d.port       :%s\r\n", cnt++, portNameList[rs232_config->port]);
  ctx->printf("%2d.baud       :%d\r\n", cnt++, rs232_config->baud);
  ctx->printf("%2d:paraity    :%s\r\n", cnt++, ITEM_LIST(rs232_config->parityIdx, rs232ParityList));
  return cnt;
}

uint8_t print_rs485_cfg(p_shell_context_t ctx, rs485_config_t *rs485_config, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));

  ctx->printf("%2d.port       :%s\r\n", cnt++, portNameList[rs485_config->port]);
  ctx->printf("%2d.baud       :%d\r\n", cnt++, rs485_config->baud);
  ctx->printf("%2d:paraity    :%s\r\n", cnt++, ITEM_LIST(rs485_config->parityIdx, rs232ParityList));
  return cnt;
}

#define HJWIND_CFG_FULL 0
#define HJWIND_CFG_OFF 1
#define HJWIND_CFG_PORT 2
uint8_t print_hjwind_cfg(p_shell_context_t ctx, hjwindspeed_config_t *hjwindCfg, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));
  ctx->printf("%2d.fullset     :%d\r\n", cnt++, hjwindCfg->full);
  ctx->printf("%2d.offset      :%d\r\n", cnt++, hjwindCfg->offset);
  ctx->printf("%2d.port        :%s\r\n", cnt++, portNameList[hjwindCfg->rs485_port]);

  return cnt;
}

#define HJWIND_DIR_CFG_PORT 0
uint8_t print_hjwindDir_cfg(p_shell_context_t ctx, hjwindspeed_config_t *hjwindCfg, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));

  ctx->printf("%2d.port       :%s\r\n", cnt++, portNameList[hjwindCfg->rs485_port]);

  return cnt;
}

// È­Áø ¿Âµµ
#define HJTEMP_CFG_PORT 0
uint8_t print_hjtemp_cfg(p_shell_context_t ctx, hjtemp_config_t *hjtempCfg, uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList, _countof(portNameList));
  // 0.type       :
  ctx->printf("%2d.port        :%s\r\n", cnt++, portNameList[hjtempCfg->rs485_port]);  // °íÁ¤
  return cnt;
}

uint8_t print_hjsnow_cfg(p_shell_context_t ctx, hjsnow_config_t *hjsnow, uint8_t cnt)
{
  const char *portNameList[10];

  rs232_get_portList(portNameList, _countof(portNameList));
  ctx->printf("%2d.port        :%s\r\n", cnt++, portNameList[hjsnow->port]);  // °íÁ¤
  return cnt;
}

uint8_t print_adc_cfg(p_shell_context_t ctx, adc_config_t *adc_config, uint8_t cnt)
{
  ctx->printf("%2d.adc mode   :%s\r\n", cnt++, ITEM_LIST(adc_config->mode, adcChModeList));
  ctx->printf("%2d.channel    :%d\r\n", cnt++, adc_config->channel);
  ctx->printf("%2d.high scale :%d\r\n", cnt++, adc_config->highScale);
  ctx->printf("%2d.low scale  :%d\r\n", cnt++, adc_config->lowScale);
  ctx->printf("%2d.scale      :%d\r\n", cnt++, adc_config->scale);
  ctx->printf("%2d.outMaxVolt :%d\r\n", cnt++, adc_config->outMaxV);
  ctx->printf("%2d.outMinVolt :%d\r\n", cnt++, adc_config->outMinV);
  return cnt;
}

int32_t select_item(p_shell_context_t ctx, const char *list[], int listCnt, void *target,
                    eVAL_TYPE_t type)
{
  int cnt;

  cnt = select_indexFromList(ctx, list, NULL, listCnt, true);
  if (cnt > 0)
  {
    cnt--;
    update_val(&cnt, target, type);
    return 0;
  }

  return cnt;
}

/**
 * @brief index·Î ÀúÀåµÈ ¼¾¼­ ¸ñ·ÏÀ» ¹®ÀÚ¿­ ¸ñ·ÏÀ¸·Î °¡Á®¿À±â
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

#define ADC_SET_CH_MODE 0
#define ADC_SET_CHANNLEL 1
#define ADC_SET_HIGHSCALE 2
#define ADC_SET_LOWSCALE 3
#define ADC_SET_SCALE 4
#define ADC_SET_OUTMAXVOLT 5
#define ADC_SET_OUTMINVOLT 6

void adc_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  adc_config_t *adc;

  adc = get_sensor_config(sensor);
  switch (cnt)
  {
    case ADC_SET_CH_MODE:  // 1.Ã¤³Î ¸ðµå
      cnt = select_indexFromList(ctx, adcChModeList, NULL, _countof(adcChModeList), true);
      if (cnt > 0)
      {
        adc->mode = (cnt - 1);
        save_config_sensor();
      }
      break;
    case ADC_SET_CHANNLEL:  // channel;
      cnt = input_decimal(ctx, 0, 17, &dec);
      if (cnt)
      {
        adc->channel = dec;
        save_config_sensor();
      }
      break;
    case ADC_SET_HIGHSCALE:  // hish cale;
      cnt = input_decimal(ctx, -100000, 100000, &dec);
      if (cnt)
      {
        adc->highScale = dec;
        save_config_sensor();
      }
      break;
    case ADC_SET_LOWSCALE:  // low cale;
      cnt = input_decimal(ctx, -100000, 100000, &dec);
      if (cnt)
      {
        adc->lowScale = dec;
        save_config_sensor();
      }
      break;
    case ADC_SET_SCALE:  // ale;
      cnt = input_decimal(ctx, -100000, 100000, &dec);
      if (cnt)
      {
        adc->scale = dec;
        save_config_sensor();
      }
      break;

    case ADC_SET_OUTMAXVOLT:
      cnt = input_decimal(ctx, -100000, 100000, &dec);
      if (cnt)
      {
        adc->outMaxV = dec;
        save_config_sensor();
      }
      break;

    case ADC_SET_OUTMINVOLT:
      cnt = input_decimal(ctx, -100000, 100000, &dec);
      if (cnt)
      {
        adc->outMinV = dec;
        save_config_sensor();
      }
      break;
  }
}

// 232¼³Á¤

#define RS232_SET_PORT 0
#define RS232_SET_BAUD 1
#define RS232_SET_PARITY 2

void rs232_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  rs232_config_t *rs232;
  const char *portList[10];
  rs232 = get_sensor_config(sensor);
  if (rs232 == 0)
  {
    sensor_add(sensor);
    rs232 = get_sensor_config(sensor);
    debug_printf("rs232 err\r\n");
  }
  switch (cnt)
  {
    case RS232_SET_PORT:

      cnt = rs232_get_portList(portList, _countof(portList));

      cnt = select_indexFromList(ctx, portList, NULL, cnt, true);
      if (cnt)
      {
        if (rs232_is_opened(
                (eRS232_PORT_t)rs232->port))  // ¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
        {
          ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
          ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
          rs232_close((eRS232_PORT_t)rs232->port);
        }

        rs232->port = cnt - 1;
        if (rs232_is_opened((eRS232_PORT_t)rs232->port))  // ¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
        {
          ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n", cnt - 1);
        }
        else
        {
          uart_config_t uart_config = {.dataLen = UART_DATA_LEN_8, .stop_bit = 0};
          uart_config.baud = rs232->baud;
          uart_config.parityIdx = rs232->parityIdx;

          rs232_open((eRS232_PORT_t)rs232->port, &uart_config);
          ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n", portList[rs232->port]);
        }

        save_config_sensor();
      }
      break;

    case RS232_SET_BAUD:
      cnt = input_decimal(ctx, 9600, 115200, &dec);
      if (cnt)
      {
        rs232->baud = dec;
        save_config_sensor();
      }

      break;
    case RS232_SET_PARITY:
      cnt = select_indexFromList(ctx, rs232ParityList, NULL, _countof(rs232ParityList), true);
      {
        rs232->parityIdx = cnt - 1;
        save_config_sensor();
      }
      break;
    default:
      break;
  }
}

#define RS485_SET_PORT 0
#define RS485_SET_BAUD 1
#define RS485_SET_PARITY 2

void rs485_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  rs485_config_t *rs485;
  const char *portList[10];

  rs485 = get_sensor_config(sensor);
  if (rs485 == NULL)
  {
    return;
  }
  switch (cnt)
  {
    case RS485_SET_PORT:

      cnt = rs485_get_portList(portList, _countof(portList));

      cnt = select_indexFromList(ctx, portList, NULL, cnt, true);
      if (cnt)
      {
        if (rs485_is_opened(
                (eRS485_PORT_t)rs485->port))  // ¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
        {
          ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
          ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
          rs485_close((eRS485_PORT_t)rs485->port);
        }

        rs485->port = cnt - 1;
        if (rs485_is_opened((eRS485_PORT_t)rs485->port))  // ¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
        {
          ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n", cnt - 1);
        }
        else
        {
          uart_config_t uart_config;
          uart_config.baud = rs485->baud;
          uart_config.parityIdx = rs485->parityIdx;
          uart_config.stop_bit = 0;
          rs485_open((eRS485_PORT_t)rs485->port, &uart_config);

          ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n", portList[rs485->port]);
        }

        save_config_sensor();
      }
      break;

    case RS485_SET_BAUD:
      cnt = input_decimal(ctx, 9600, 115200, &dec);
      if (cnt)
      {
        rs485->baud = dec;
        save_config_sensor();
      }

      break;
    case RS485_SET_PARITY:
      cnt = select_indexFromList(ctx, rs232ParityList, NULL, _countof(rs232ParityList), true);
      {
        rs485->parityIdx = cnt - 1;
        save_config_sensor();
      }
      break;
    default:
      break;
  }
}

void hjwind_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  hjwindspeed_config_t *hjwind;
  const char *portList[10];

  hjwind = get_sensor_config(sensor);
  if (hjwind == NULL)
  {
    return;
  }
  switch (cnt)
  {
    case HJWIND_CFG_FULL:
      cnt = input_decimal(ctx, 0, 999999, &dec);
      if (cnt)
      {
        hjwind->full = dec;
        save_config_sensor();
      }
      break;
    case HJWIND_CFG_OFF:
      cnt = input_decimal(ctx, 0, 999999, &dec);
      if (cnt)
      {
        hjwind->offset = dec;
        save_config_sensor();
      }
      break;
    case HJWIND_CFG_PORT:
      cnt = rs485_get_portList(portList, _countof(portList));

      cnt = select_indexFromList(ctx, portList, NULL, cnt, true);
      if (cnt)
      {
        if (rs485_is_opened(
                (eRS485_PORT_t)hjwind->rs485_port))  // ¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
        {
          ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
          ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
          rs485_close((eRS485_PORT_t)hjwind->rs485_port);
        }

        hjwind->rs485_port = cnt - 1;
        if (rs485_is_opened(
                (eRS485_PORT_t)hjwind->rs485_port))  // ¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
        {
          ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n", cnt - 1);
        }
        else
        {
          uart_config_t uart_config;
          uart_config.baud = 19200;
          uart_config.parityIdx = 0;
          uart_config.stop_bit = 0;
          rs485_open((eRS485_PORT_t)hjwind->rs485_port, &uart_config);

          ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n", portList[hjwind->rs485_port]);
        }

        save_config_sensor();
      }
      break;
    default:
      break;
  }
}

void hjwinddir_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  hjwindspeed_config_t *hjwind;
  const char *portList[10];

  hjwind = get_sensor_config(sensor);
  if (hjwind == NULL)
  {
    return;
  }
  switch (cnt)
  {
    case HJWIND_DIR_CFG_PORT:
      cnt = rs485_get_portList(portList, _countof(portList));

      cnt = select_indexFromList(ctx, portList, NULL, cnt, true);
      if (cnt)
      {
        if (rs485_is_opened(
                (eRS485_PORT_t)hjwind->rs485_port))  // ¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
        {
          ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
          ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
          rs485_close((eRS485_PORT_t)hjwind->rs485_port);
        }

        hjwind->rs485_port = cnt - 1;
        if (rs485_is_opened(
                (eRS485_PORT_t)hjwind->rs485_port))  // ¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
        {
          ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n", cnt - 1);
        }
        else
        {
          uart_config_t uart_config;
          uart_config.baud = 19200;
          uart_config.parityIdx = 0;
          uart_config.stop_bit = 0;
          rs485_open((eRS485_PORT_t)hjwind->rs485_port, &uart_config);

          ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n", portList[hjwind->rs485_port]);
        }

        save_config_sensor();
      }
      break;
    default:
      break;
  }
}
void hjtemp_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  hjtemp_config_t *hjtemp;
  const char *portList[10];

  hjtemp = get_sensor_config(sensor);
  if (hjtemp == NULL)
  {
    return;
  }
  switch (cnt)
  {
    case HJTEMP_CFG_PORT:
      cnt = rs485_get_portList(portList, _countof(portList));

      cnt = select_indexFromList(ctx, portList, NULL, cnt, true);
      if (cnt)
      {
        if (rs485_is_opened(
                (eRS485_PORT_t)hjtemp->rs485_port))  // ¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
        {
          ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
          ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
          rs485_close((eRS485_PORT_t)hjtemp->rs485_port);
        }

        hjtemp->rs485_port = cnt - 1;
        if (rs485_is_opened(
                (eRS485_PORT_t)hjtemp->rs485_port))  // ¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
        {
          ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n", cnt - 1);
        }
        else
        {
          uart_config_t uart_config;
          uart_config.baud = 9600;
          uart_config.parityIdx = 0;
          uart_config.stop_bit = 0;
          rs485_open((eRS485_PORT_t)hjtemp->rs485_port, &uart_config);

          ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n", portList[hjtemp->rs485_port]);
        }

        save_config_sensor();
      }
      break;
    default:
      break;
  }
}
void rain_reed_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt) {}

void rain_hall_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt) {}

void rain_reed_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt);
void rain_hall_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt);

typedef struct
{
  uint8_t sensorType;
  void (*config_set)(p_shell_context_t, sensor_t *, uint8_t);
} config_sen_func_t;

/*
¼¾¼­ ¸ðµ¨°ú ¸ðµ¨ ¼³Á¤ ÇÔ¼ö ¿¬°á

¼¾¼­°¡ Ãß°¡µÇ°Å³ª ¼¾¼­°íÀ¯ÀÇ ¼³Á¤°ªÀ» º¯°æÇÏ·Á¸é Ã³¸® ÇÔ¼ö¸¦ ÀÛ¼ºÇØ¾ßÇÑ´Ù.
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
    {.sensorType = S_T_WIND_SPEED_MAX_VAL, .config_set = 0},
    {.sensorType = S_T_WIND_DIRECTION_MAX_VAL, .config_set = 0},
    {.sensorType = S_T_SNOW_HJ_485, .config_set = rs485_config_set},
    {.sensorType = S_T_SNOW_HJ_232, .config_set = rs232_config_set},
    {.sensorType = S_T_GENERAL_485, .config_set = rs485_config_set},
    {.sensorType = S_T_TEMPERATURE_HJ_485, .config_set = hjtemp_config_set},
    {.sensorType = S_T_HUMI_HJ_485, .config_set = hjtemp_config_set}};

/*
 ¼¾¼­ °³º°
 */
int32_t print_common_cfg(p_shell_context_t ctx, sensor_t *sensor, uint8_t c)
{
  int32_t cnt = 0;

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);

  switch (sensor->type)
  {
    case S_T_ADC:  // ADC
      cnt = print_adc_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_GENERAL_232:
      cnt = print_rs232_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_GENERAL_485:
      cnt = print_rs485_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_SNOW_HJ_232:
    case S_T_SNOW_HJ_485:
      cnt = print_hjsnow_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_WIND_DIRECTION_HJ_485:
      cnt = print_hjwindDir_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_WIND_SPEED_HJ_485:
      cnt = print_hjwind_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_TEMPERATURE_HJ_485:
      cnt = print_hjtemp_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_HUMI_HJ_485:
      cnt = print_hjtemp_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
  }
  return cnt;
}
int32_t print_menu_sensor_temp(p_shell_context_t ctx)
{
  int cnt = 0;
  sensor_t *sensor;
  sensor = &get_config_app()->sensor[A1_TEMPERATURE];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);

  if (sensor->type != S_T_UNSUED)
  {
    cnt = print_common_cfg(ctx, sensor, cnt);
  }

  return cnt;
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

// ¿Âµµ ¼³Á¤
int32_t menu_sensor_temp(p_shell_context_t ctx)
{
  const char *itemList[10];

  int32_t cnt;
  uint8_t itemListCnt;

  sensor_t *sensor = &get_config_app()->sensor[A1_TEMPERATURE];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_temp, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      itemListCnt = gen_sensorItemList(itemList, temperatureList, _countof(temperatureList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)temperatureList[cnt - 1];
        set_type(sensor);
      }
    }

    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 2);
          break;
        }
      }
    }
  } while (1);
}

// Ç³Çâ ¼³Á¤Á¤
int32_t print_menu_sensor_windDirection(p_shell_context_t ctx)
{
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A2_WIND_DIRECTION];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}
int32_t menu_sensor_windDirection(p_shell_context_t ctx)
{
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A2_WIND_DIRECTION];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_windDirection, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      // ¹æÇâ ¼¾¼­ ¸ñ·Ï °¡Á®¿È
      itemListCnt = gen_sensorItemList(itemList, windDirectionList, _countof(windDirectionList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)windDirectionList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

// Ç³¼Ó ¼³Á¤Á¤
int32_t print_menu_sensor_windSpeed(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A3_WIND_SPEED];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}
int32_t menu_sensor_windSpeed(p_shell_context_t ctx)
{
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A3_WIND_SPEED];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_windSpeed, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      // ¹æÇâ ¼¾¼­ ¸ñ·Ï °¡Á®¿È
      itemListCnt = gen_sensorItemList(itemList, windDirectionList, _countof(windDirectionList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)windDirectionList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

int32_t print_menu_sensor_windDirectionInstanct(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A4_INSTANT_WIND_DIRECTION];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);

  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}
int32_t menu_sensor_windDirectionInstant(p_shell_context_t ctx)
{
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A4_INSTANT_WIND_DIRECTION];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_windDirectionInstanct, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      itemListCnt = gen_sensorItemList(itemList, windDirectionInstantList,
                                       _countof(windDirectionInstantList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)windDirectionInstantList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

int32_t print_menu_sensor_windSpeedInstanct(p_shell_context_t ctx)
{
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A5_INSTANT_WIND_SPEED];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);

  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}
int32_t menu_sensor_windSpeedInstant(p_shell_context_t ctx)
{
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A5_INSTANT_WIND_SPEED];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_windSpeedInstanct, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      itemListCnt = gen_sensorItemList(itemList, windDirectionInstantList,
                                       _countof(windDirectionInstantList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)windDirectionInstantList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

// Àû¼³ ¼³Á¤
int32_t print_menu_sensor_snow(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A9_SNOW_DEPTH];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}
int32_t menu_sensor_snow(p_shell_context_t ctx)
{
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A9_SNOW_DEPTH];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_snow, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      itemListCnt = gen_sensorItemList(itemList, snowList, _countof(snowList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)snowList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

int32_t print_menu_sensor_rain(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A6_RAINFALL_DOT5_1MM];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}

int32_t menu_sensor_rain(p_shell_context_t ctx)
{
  // int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A6_RAINFALL_DOT5_1MM];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_rain, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      // °­¿ì ¼¾¼­ÀÇ ¹®ÀÚ¿­ ¸ñ·ÏÀ» °¡Á®¿Â´Ù.
      itemListCnt = gen_sensorItemList(itemList, rainList, _countof(rainList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)rainList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

// ±â¾Ð ¼³Á¤
int32_t print_menu_sensor_pressure(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A7_PRESSURE];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}

int32_t menu_sensor_pressure(p_shell_context_t ctx)
{
  // int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A7_PRESSURE];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_pressure, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      // °­¿ì ¼¾¼­ÀÇ ¹®ÀÚ¿­ ¸ñ·ÏÀ» °¡Á®¿Â´Ù.
      itemListCnt = gen_sensorItemList(itemList, pressureList, _countof(pressureList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)pressureList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

// ½Àµµ ¼³Á¤
int32_t print_menu_sensor_humi(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A10_RELATIVE_HUMIDITY];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}

int32_t menu_sensor_humi(p_shell_context_t ctx)
{
  // int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A10_RELATIVE_HUMIDITY];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_humi, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      itemListCnt = gen_sensorItemList(itemList, humiList, _countof(humiList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)humiList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

// °­¼öÀ¯¹« ¼³Á¤
int32_t print_menu_sensor_rainPresent(p_shell_context_t ctx)
{
  // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A8_RAIN_PRESENT];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  switch (sensor->type)
  {
    case S_T_RAIN_PRESENT_DI:
      // cnt = print_adc_cfg( ctx,get_sensor_config(sensor),cnt);
      break;
  }
  return cnt;
}

int32_t menu_sensor_rainPresent(p_shell_context_t ctx)
{
  // int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &get_config_app()->sensor[A8_RAIN_PRESENT];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_rainPresent, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    if (cnt == 0)
    {
      itemListCnt = gen_sensorItemList(itemList, rainPresentList, _countof(rainPresentList));
      cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
      if (cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)rainPresentList[cnt - 1];
        set_type(sensor);
      }
    }
    else
    {
      for (int i = 0; i < _countof(sen_func); i++)
      {
        if (sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx, sensor, cnt - 1);
          break;
        }
      }
    }
  } while (1);
}

void sensor_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  for (int i = 0; i < _countof(sen_func); i++)
  {  // ¼¾¼­¸¶´Ù °íÀ¯ÀÇ Ã³¸® ÇÔ¼ö¸¦ »ç¿ëÇÑ´Ù.
    if (sen_func[i].sensorType == sensor->type)
    {
      // °íÀ¯ Ã³¸® ÇÔ¼ö´Â 0¹øºÎÅÍ Ã³¸®ÇÏµµ·Ï µÇ¾îÀÖ¾î¼­ -1ÇØÁØ´Ù.
      //
      sen_func[i].config_set(ctx, sensor, cnt - 1);
      break;
    }
  }
}

/**
 * @brief ¼¾¼­ ¸ðµ¨ º¯°æ
 */
void sensor_type_set(p_shell_context_t ctx, sensor_t *sensor, const uint8_t *list, uint8_t listCnt)
{
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  itemListCnt = gen_sensorItemList(itemList, list, listCnt);
  cnt = select_indexFromList(ctx, itemList, NULL, itemListCnt, true);
  if (cnt > 0)
  {
    sensor->type = (eSENSOR_MODEL_t)list[cnt - 1];
    set_type(sensor);
  }
}

// ±âº» ¼³Á¤
int32_t print_menu_sensor_default(p_shell_context_t ctx)
{
  int cnt = 0;
  sensor_t *sensor;

  sensor = &get_config_app()->sensor[A11_RAINFALL_DOT1MM];

  ctx->printf("%2d.type       :%s\r\n", cnt++, g_sensor_model_list[sensor->type]);
  cnt = print_common_cfg(ctx, sensor, cnt);
  return cnt;
}

int32_t menu_sensor_default(p_shell_context_t ctx)
{
  int32_t cnt;
  sensor_t *sensor = &get_config_app()->sensor[A11_RAINFALL_DOT1MM];

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_humi, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:  // Å¸ÀÔ
        sensor_type_set(ctx, sensor, humiList, _countof(humiList));
        break;
      default:
        sensor_set(ctx, sensor, cnt - 1);
        break;
    }
  } while (1);
}

/*
¼¾¼­ ¼³Á¤


*/
int32_t menu_sensor_default_2(p_shell_context_t ctx, eSENSOR_LIST_t list)
{
  int32_t cnt = 0;

  // ¼±ÅÃµÈ ¼¾¼­ÀÇ ¼³Á¤ Á¤º¸¸¦ °¡Á®¿Â´Ù.
  sensor_t *sensor = &get_config_app()->sensor[(int)list];
  do
  {
    /*
    ¼¾¼­ÀÇ ÇöÀç Á¤º¸¸¦ Ãâ·ÂÇÏ°í, ¼öÁ¤À» ¿øÇÏ´Â Ç×¸ñÀÇ ¹øÈ£¸¦ ÀÔ·Â¹Þ´Â´Ù.
    0.type       :È­Áø RS485 9600
    1.port        :EX1 RS485 A
    ÀÌ·± È­¸éÀÌ ³ªÅ¸³²³²
    */
    cnt = select_indexMenu(ctx, sensor);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:  // ¼¾¼­°¡ »ç¿ëÇÏ°íÀÚÇÏ´Â ¼¾¼­ Å¸ÀÔÀ» ¼³Á¤ÇÑ´Ù.
               // ¼¾¼­¸¶´Ù Áö¿ø°¡´ÉÇÑ ¸ñ·ÏÀ» ³Ñ°ÜÁö°í Ãâ·ÂÇÏ¿© ¼±ÅÃÇÏµµ·Ï ÇÑ´Ù.
        sensor_type_set(ctx, sensor, supported_sensors[list].list, supported_sensors[list].cnt);
        break;
      default:  // ¼¾¼­ Å¸ÀÔÀÌ ¾Æ´Ñ ¼¾¼­ °íÀ¯ ¼Ó¼ºµéÀº ÀÌ ÇÔ¼ö ¿¡¼­ Ã³¸®ÇÑ´Ù.
        // ÇöÀçÀÇ ¼¾¼­ Á¤º¸¿Í »ç¿ëÀÚ°¡ ¼öÁ¤ÇÏ°íÀÚÇÑ Ç×¸ñ ¹øÈ£¸¦ ³Ñ±ä´Ù.
        sensor_set(ctx, sensor, cnt);  // ¼¾¼­º° ¼³Á¤°ª º¯°æ
        break;
    }
  } while (1);
}

/**
 * @brief ¼¾¼­ ¸Þ´º
 * @retval ¼¾¼­ ¸ñ·ÏÀ» Ãâ·ÂÇÏ°í ¼¾¼­¸¦ ¼±ÅÃÇÑ´Ù.
 */
int32_t menu_sensor(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }

    cnt = menu_sensor_default_2(ctx, (eSENSOR_LIST_t)(cnt - 1));
  } while (cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t print_net_use(p_shell_context_t ctx)
{
  int32_t cnt = 3;

  ctx->printf(" 0.ÀÌ´õ³Ý  :%s\r\n", ITEM_LIST((int)get_config_app()->eth_use, enableList));
  ctx->printf(" 1.CDMA    :%s\r\n", ITEM_LIST((int)get_config_app()->cdma_use, enableList));
  ctx->printf(" 2.Á÷Á¢Åë½Å:%s\r\n", ITEM_LIST((int)get_config_app()->direct_use, enableList));

  return cnt;
}

int32_t menu_net_use(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_use, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
        if (input_use(ctx, &get_config_app()->eth_use))
        {
          WRITE_CFG(eth_use);
        }
        break;
      case 1:
        if (input_use(ctx, &get_config_app()->cdma_use))
        {
          if (get_config_app()->cdma_use)
          {
            config.direct_use = 0;
            WRITE_CFG(direct_use);
          }

          WRITE_CFG(cdma_use);
        }
        break;
      case 2:
        if (input_use(ctx, &get_config_app()->direct_use))
        {
          if (get_config_app()->direct_use)
          {
            config.cdma_use = 0;
            WRITE_CFG(cdma_use);
          }
          WRITE_CFG(direct_use);
        }
        break;
    }
  } while (1);
}

int32_t print_net_eth_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  ctx->printf("%2d.¹æ½Ä         :%s \r\n", cnt++, ITEM_LIST(get_config_app()->eth_mode, ethModeList));
  ctx->printf("%2d.¿ø°Ý ¼­¹ö Á¤º¸\r\n", cnt++);
  ctx->printf("%2d.±âº» ±¸¼º\r\n", cnt++);

  return cnt;
}

int32_t print_net_eth_remote_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  uint8_t *ip = get_config_app()->eth_server_ip;

  ctx->printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  ctx->printf("%2d.port    :%d\r\n", cnt++, get_config_app()->eth_server_port);
  ctx->printf("%2d.protocol:%s\r\n", cnt++, ITEM_LIST(get_config_app()->eth_protocol, protocolList));

  return cnt;
}

int32_t menu_net_eth_remote_set(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t a, b, c, d;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_eth_remote_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:
        ctx->printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_server_ip[0] = a;
          config.eth_server_ip[1] = b;
          config.eth_server_ip[2] = c;
          config.eth_server_ip[3] = d;
          WRITE_CFG(eth_server_ip);
        }
        break;
      case 1:
        if (input_decimal(ctx, 0, 60000, &dec))
        {
          config.eth_server_port = dec;
          WRITE_CFG(eth_server_port);
        }
        break;
      case 2:  // ÇÁ·ÎÅäÄÝ
        cnt = select_indexFromList(ctx, protocolList, NULL, _countof(protocolList), true);
        if (cnt > 0)
        {
          cnt--;
          config.eth_protocol = cnt;
          WRITE_CFG(eth_protocol);
        }
        break;
    }
  } while (1);
}

int32_t print_net_eth_default_set(p_shell_context_t ctx)
{
  int32_t cnt = 3;
  uint8_t *ip = config.eth_ip;
  uint8_t *gw = config.eth_gateway;
  uint8_t *subnet = config.eth_subnet;

  ctx->printf(" 0.ip      :%d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
  ctx->printf(" 1.subnet  :%d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);
  ctx->printf(" 2.gateway :%d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);

  return cnt;
}

int32_t menu_net_eth_default_set(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t a, b, c, d;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_eth_default_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:  // ip
        ctx->printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_ip[0] = a;
          config.eth_ip[1] = b;
          config.eth_ip[2] = c;
          config.eth_ip[3] = d;
          WRITE_CFG(eth_ip);
        }
        break;
      case 1:  // subnet
        ctx->printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_subnet[0] = a;
          config.eth_subnet[1] = b;
          config.eth_subnet[2] = c;
          config.eth_subnet[3] = d;
          WRITE_CFG(eth_subnet);
        }
        break;
      case 2:  // gateway
        ctx->printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.eth_gateway[0] = a;
          config.eth_gateway[1] = b;
          config.eth_gateway[2] = c;
          config.eth_gateway[3] = d;
          WRITE_CFG(eth_gateway);
        }
        break;
    }
  } while (1);
}

int32_t menu_net_eth_mode_set(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t a, b, c, d;

  cnt = select_indexFromList(ctx, ethModeList, NULL, _countof(ethModeList), true);
  if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
  {
    return cnt;
  }

  cnt--;
  switch (cnt)
  {
    case 0:
      config.eth_mode = cnt;
      WRITE_CFG(eth_mode);
      break;
  }
  return cnt;
}

int32_t menu_net_eth_set(p_shell_context_t ctx)
{
  int32_t cnt;

  const menu_func menu[] = {menu_net_eth_mode_set, menu_net_eth_remote_set,
                            menu_net_eth_default_set};
  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_eth_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
        cnt = menu_net_eth_mode_set(ctx);
        break;
      default:
        cnt = menu[cnt](ctx);
        if (cnt == EXIT_PROGRAM)
        {
          return cnt;
        }
        break;
    }

  } while (1);
}

int32_t print_net_cdma_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  uint8_t *ip = config.cdma_server_ip;
  int32_t port = config.cdma_port;

  ctx->printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  ctx->printf("%2d.port    :%d\r\n", cnt++, port);
  ctx->printf("%2d.protocol:%s\r\n", cnt++, ITEM_LIST(config.cdma_protocol, protocolList));
  ctx->printf("%2d.model   :%s\r\n", cnt++, ITEM_LIST(config.cdma_model, cdmaModellList));

  return cnt;
}

int32_t menu_net_cdma_set(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t a, b, c, d;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_cdma_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:
        ctx->printf("xxx.xxx.xxx.xxx:");
        if (console_scanf("%d.%d.%d.%d", &a, &b, &c, &d) == 4)
        {
          config.cdma_server_ip[0] = a;
          config.cdma_server_ip[1] = b;
          config.cdma_server_ip[2] = c;
          config.cdma_server_ip[3] = d;
          WRITE_CFG(cdma_server_ip);
        }
        break;
      case 1:
        if (input_decimal(ctx, 0, 60000, &dec))
        {
          config.cdma_port = dec;
          WRITE_CFG(cdma_port);
        }
        break;
      case 2:  // ÇÁ·ÎÅäÄÝ
        cnt = select_indexFromList(ctx, protocolList, NULL, _countof(protocolList), true);
        if (cnt > 0)
        {
          cnt--;
          config.cdma_protocol = cnt;
          WRITE_CFG(cdma_protocol);
        }
        break;
      case 3:  // ¸ðµ¨
        cnt = select_indexFromList(ctx, cdmaModellList, NULL, _countof(cdmaModellList), true);
        if (cnt > 0)
        {
          cnt--;
          config.cdma_model = cnt;
          WRITE_CFG(cdma_model);
        }
        break;
    }
  } while (1);
}

int32_t print_net_direct_set(p_shell_context_t ctx)
{
  int32_t cnt = 2;

  ctx->printf("%2d.baud     :%d\r\n", cnt++, config.direct_baud);
  ctx->printf("%2d.protocol :%s\r\n", cnt++, ITEM_LIST(config.direct_protocol, protocolList));

  return cnt;
}

int32_t menu_net_direct_set(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_direct_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK)
    {
      return cnt;
    }

    cnt--;
    switch (cnt)
    {
      case 0:  // baud
        if (input_decimal(ctx, 0, 115200, &dec))
        {
          config.direct_baud = dec;
          WRITE_CFG(direct_baud);
        }
        break;
      case 2:  // ÇÁ·ÎÅäÄÝ
        cnt = select_indexFromList(ctx, protocolList, NULL, _countof(protocolList), true);
        if (cnt > 0)
        {
          cnt--;
          config.direct_protocol = cnt;
          WRITE_CFG(direct_protocol);
        }
        break;
    }
  } while (1);
}

int32_t print_net_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("%2d.ÀÌ´õ³Ý\r\n", cnt++);
  ctx->printf("%2d.CDMA\r\n", cnt++);
  ctx->printf("%2d.Á÷Á¢ Åë½Å\r\n", cnt++);

  return cnt;
}

int32_t menu_net_set(p_shell_context_t ctx)
{
  int32_t cnt;
  const menu_func menu[] = {menu_net_eth_set, menu_net_cdma_set, menu_net_direct_set};
  do
  {
    cnt = select_indexFromList(ctx, NULL, print_net_set, 0, false);
    if (cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
  } while (cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t print_menu_vhf(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("%2d.±×·ì         :%d\r\n", cnt++, config.vhf_group);
  ctx->printf("%2d.VHF ID       :%d\r\n", cnt++, config.vhf_id);
  ctx->printf("%2d.Áß°è ID      :%d\r\n", cnt++, config.vhf_repeater_id);
  ctx->printf("%2d.ÅëÁ¦ ID      :%d\r\n", cnt++, config.vhf_host_id);
  ctx->printf("%2d.PTT ½Ã°£(ms) :%d\r\n", cnt++, config.vhf_ptt_delay);
  ctx->printf("%2d.VHF °¡»ó ¼³Á¤\r\n", cnt++);
  ctx->printf("%2d.VHF ·çÇÁ Å×½ºÆ®\r\n", cnt++);
  ctx->printf("%2d.VHF Åæ Å×½ºÆ®\r\n", cnt++);

  return cnt;
}

int32_t menu_net_vhf_vir_set(p_shell_context_t ctx) { return 0; }
int32_t menu_net_vhf_loop_test(p_shell_context_t ctx) { return 0; }

int32_t menu_net_vhf_tone_test(p_shell_context_t ctx) { return 0; }

int32_t menu_net_vhf(p_shell_context_t ctx)
{
  int32_t cnt;
  //  int32_t ret;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_vhf, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:  // ±×·ì
        if (input_decimal(ctx, 0, 255, &dec))
        {
          config.vhf_group = dec;
          WRITE_CFG(vhf_group);
        }
        break;
      case 1:  // id
        if (input_decimal(ctx, 0, 255, &dec))
        {
          config.vhf_id = dec;
          WRITE_CFG(vhf_id);
        }
        break;
      case 2:  // Áß°è
        if (input_decimal(ctx, 0, 255, &dec))
        {
          config.vhf_repeater_id = dec;
          WRITE_CFG(vhf_repeater_id);
        }
        break;
      case 3:  // È£½ºÆ®
        if (input_decimal(ctx, 0, 255, &dec))
        {
          config.vhf_host_id = dec;
          WRITE_CFG(vhf_host_id);
        }
        break;
      case 4:  // ptt
        if (input_decimal(ctx, 0, 255, &dec))
        {
          config.vhf_ptt_delay = dec;
          WRITE_CFG(vhf_ptt_delay);
        }
        break;
      case 5:  //
        menu_net_vhf_vir_set(ctx);
        break;
      default:
        break;
    }

  } while (cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t print_menu_net(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  char buff[50] = {0};

  make_comList(buff, sizeof(buff));

  ctx->printf("%2d.Åë½Å ¹æ½Ä:%s\r\n", cnt++, buff);
  ctx->printf("%2d.Åë½Å ¼³Á¤\r\n", cnt++);
  ctx->printf("%2d.VHF\r\n", cnt++);

  return cnt;
}

int32_t menu_network(p_shell_context_t ctx)
{
  int32_t cnt;

  const menu_func menu[] = {menu_net_use, menu_net_set, menu_net_vhf};
  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_net, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
  } while (cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t print_menu_data(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("%2d.µ¥ÀÌÅÍ È®ÀÎ\r\n", cnt++);
  ctx->printf("%2d.µ¥ÀÌÅÍ ÆíÁý\r\n", cnt++);
  ctx->printf("%2d.µ¥ÀÌÅÍ ÃÊ±âÈ­\r\n", cnt++);

  return cnt;
}

int32_t menu_data_view(p_shell_context_t ctx)
{
  const uint8_t kLoggingIntervalMin = 1;
  DATE_TIME_BUF ut;
  int32_t year;
  int32_t month;
  int32_t day;
  int32_t hour;
  int32_t min;
  int32_t sec;
  int32_t readCnt;
  int32_t cnt;
  uint32_t timeTick;
  uint32_t timeTickEnd;

  kma_data_t kma_data;
  do
  {
    ctx->printf("yyyy-mm-dd hh:mm:ss,cnt >>");

    cnt = console_scanf("%04d-%02d-%02d %02d:%02d:%02d,%d", &year, &month, &day, &hour, &min, &sec,
                        &readCnt);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    ut.Year = year;
    ut.Month = month;
    ut.Day = day;
    ut.Hour = hour;
    ut.Min = min;
    ut.Sec = sec;

    timeTick = time_cvt_timestamp(&ut);
    timeTickEnd = timeTick + kLoggingIntervalMin * 60 * readCnt;
    for (uint32_t tick = timeTick; tick <= timeTickEnd;)
    {
      read_data(&ut, &kma_data, sizeof(kma_data), 0, 1);

      ctx->printf("%04d-%02d-%02d %02d:%02d:%02d\r\n", ut.Year, ut.Month, ut.Day, ut.Hour, ut.Min,
                  ut.Sec);

      ctx->printf("±â¿Â            : %-6d\r\n", kma_data.temperature);
      ctx->printf("Ç³Çâ            : %-6d\r\n", kma_data.wind_direction_avg);
      ctx->printf("Ç³¼Ó            : %-6d\r\n", kma_data.wind_speed_avg);
      ctx->printf("Ç³Çâ(¼ø°£)      : %-6d\r\n", kma_data.wind_direction_instant);
      ctx->printf("Ç³¼Ó(¼ø°£)      : %-6d\r\n", kma_data.wind_speed_instant);
      ctx->printf("°­¼ö·®          : %-6d\r\n", kma_data.precipitation);
      ctx->printf("±â¾Ð            : %-6d\r\n", kma_data.pressure);
      ctx->printf("°­¼ö À¯¹«       : %-6d\r\n", kma_data.precipitation_presence);
      ctx->printf("Àû¼³            : %-6d\r\n", kma_data.snowfall);
      ctx->printf("½Àµµ            : %-6d\r\n", kma_data.relative_humidity);
      ctx->printf("°­¼ö·®(0.1mm)   : %-6d\r\n", kma_data.precipitation_fine);
      ctx->printf("ÀÏ»ç            : %-6d\r\n", kma_data.solar_radiation);
      ctx->printf("ÀÏÁ¶            : %-6d\r\n", kma_data.sunshine_duration);
      ctx->printf("Áö¸é¿Âµµ        : %-6d\r\n", kma_data.surface_temperature);
      ctx->printf("ÃÊ»ó¿Âµµ        : %-6d\r\n", kma_data.grass_temperature);
      ctx->printf("ÁöÁß¿Âµµ 5cm    : %-6d\r\n", kma_data.soil_temperature_5cm);
      ctx->printf("ÁöÁß¿Âµµ 10cm   : %-6d\r\n", kma_data.soil_temperature_10cm);
      ctx->printf("ÁöÁß¿Âµµ 20cm   : %-6d\r\n", kma_data.soil_temperature_20cm);
      ctx->printf("ÁöÁß¿Âµµ 30cm   : %-6d\r\n", kma_data.soil_temperature_30cm);
      ctx->printf("ÁöÁß¿Âµµ 50cm   : %-6d\r\n", kma_data.soil_temperature_50cm);
      ctx->printf("ÁöÁß¿Âµµ   1m   : %-6d\r\n", kma_data.soil_temperature_1m);
      ctx->printf("ÁöÁß¿Âµµ 1_5m   : %-6d\r\n", kma_data.soil_temperature_1_5m);
      ctx->printf("ÁöÁß¿Âµµ   3m   : %-6d\r\n", kma_data.soil_temperature_3m);
      ctx->printf("ÁöÁß¿Âµµ   5m   : %-6d\r\n", kma_data.soil_temperature_5m);
      ctx->printf("¿î°í(1Ãþ)       : %-6d\r\n", kma_data.cloud_height_1st);
      ctx->printf("¿î°í(2Ãþ)       : %-6d\r\n", kma_data.cloud_height_2nd);
      ctx->printf("¿î°í(3Ãþ)       : %-6d\r\n", kma_data.cloud_height_3rd);
      ctx->printf("¿î·®·®          : %-6d\r\n", kma_data.cloud_amount);
      ctx->printf("½ÃÁ¤Á¤          : %-6d\r\n", kma_data.visibility);
      ctx->printf("PM1.0           : %-6d\r\n", kma_data.pm10_concentration);
      ctx->printf("PM2.5           : %-6d\r\n", kma_data.pm25_concentration);
      ctx->printf("¼øº¹»ç          : %-6d\r\n", kma_data.net_radiation);
      ctx->printf("ÀüÃµº¹»ç        : %-6d\r\n", kma_data.total_radiation);
      ctx->printf("¹Ý»çº¹»ç»ç      : %-6d\r\n", kma_data.reflected_radiation);
      ctx->printf("Á÷´Þº¹»ç»ç      : %-6d\r\n", kma_data.direct_radiation);
      ctx->printf("ÇöÀç ÀÏ±â±â     : %-6d\r\n", kma_data.current_weather);
      ctx->printf("Åä¾ç¼öºÐ(10cm)  : %-6d\r\n", kma_data.soil_moisture_10cm);
      ctx->printf("Åä¾ç¼öºÐ(20cm)  : %-6d\r\n", kma_data.soil_moisture_20cm);
      ctx->printf("Åä¾ç¼öºÐ(30cm)  : %-6d\r\n", kma_data.soil_moisture_30cm);
      ctx->printf("Åä¾ç¼öºÐ(50cm)  : %-6d\r\n", kma_data.soil_moisture_50cm);
      ctx->printf("Á¶µµ·®·®        : %-6d\r\n", kma_data.illuminance);
      ctx->printf("Ç³¼Ó(1.5m)      : %-6d\r\n", kma_data.wind_speed_1_5m);
      ctx->printf("Ç³¼Ó(4.0m)      : %-6d\r\n", kma_data.wind_speed_4m);
      ctx->printf("¼ø°£ Ç³¼Ó(1.5m) : %-6d\r\n", kma_data.instant_wind_speed_1_5m);
      ctx->printf("¼ø°£ Ç³¼Ó(4.0m) : %-6d\r\n", kma_data.instant_wind_speed_4m);
      ctx->printf("±â¿Â(0.5m)      : %-6d\r\n", kma_data.temperature_0_5m);
      ctx->printf("±â¿Â(4.0m)      : %-6d\r\n", kma_data.temperature_4m);
      ctx->printf("½Àµµ(0.5m)      : %-6d\r\n", kma_data.humidity_0_5m);
      ctx->printf("½Àµµ(4.0m)      : %-6d\r\n", kma_data.humidity_4m);
      ctx->printf("Å¸ÄÚ¹ÌÅÍ        : %-6d\r\n", kma_data.tacometer);

      tick += (60 * kLoggingIntervalMin);
      time_cvt_secTotime(tick, &ut);
    }

    osDelay(100);
  } while (1);
}

int32_t menu_data_edit(p_shell_context_t ctx) {}

int32_t menu_data_reset(p_shell_context_t ctx) {}

menu_func g_dataMenu[] = {menu_data_view, menu_data_edit, menu_data_reset};

int32_t menu_data(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_data, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = g_dataMenu[cnt](ctx);
    if (cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);
}

int32_t print_menu_panel(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("%2d.ÆÐ³Î Á¾·ù:%s\r\n", cnt++, ITEM_LIST(config.panel_model, panelList));

  return cnt;
}
int32_t menu_display_panel(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_panel, 0, false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
        cnt = select_indexFromList(ctx, panelList, NULL, _countof(panelList), false);

        if (cnt > 0)
        {
          cnt--;
          config.panel_model = cnt;
          WRITE_CFG(panel_model);
        }
        break;
    }

  } while (1);
}

int32_t menu_manage_version(p_shell_context_t ctx)
{
  char buff[30];

  uint8_t a, b, c, d;
  DATE_TIME_BUF ct;

  get_appVer(&a, &b, &c, &d);
  get_appBuild(&ct);

  ctx->printf("App:%d.%d.%d.%d\r\n", a, b, c, d);
  make_timeToStr(&ct, buff, sizeof(buff));
  ctx->printf("App build:%s\r\n", buff);

  get_bootVer(&a, &b, &c, &d);
  get_bootBuild(&ct);

  ctx->printf("Boot:%d.%d.%d.%d\r\n", a, b, c, d);
  make_timeToStr(&ct, buff, sizeof(buff));
  ctx->printf("Boot build:%s\r\n", buff);
  return 0;
}
int32_t download_file(int32_t (*save_file)(char *path, uint32_t offset, uint8_t *data,
                                           uint32_t dataLen),
                      char *path, uint32_t offset, uint32_t *len, uint32_t limit);

int32_t save_file(char *path, uint32_t offset, uint8_t *data, uint32_t dataLen)
{
  flash_write(offset, data, dataLen);

  return 0;
}

int32_t menu_manage_update(p_shell_context_t ctx)
{
  char buff[20];
  uint32_t len;

  ctx->printf("10ÃÊµÚ¿¡ ÆÄÀÏÀ» Àü¼ÛÇØÁÖ¼¼¿ä\r\n");
  osDelay(10000);

  if (download_file(save_file, buff, 0, &len, 512 * 1024) == 0)
  {
    ctx->printf("ÆÄÀÏ Å©±â:%d\r\n", len);
  }
  else
  {
    ctx->printf("ÆÄÀÏ ¼ö½Å ¿À·ù\r\n");
  }

  return 0;
}
int32_t menu_manage_device_reset(p_shell_context_t ctx)
{
  reset_system(0, "console reset");
  return 0;
}

void config_hj_reset(void)
{
  config_t hj_config;
  adc_config_t *adc_config;
  hjtemp_config_t *hjtemp_cfg;
  hjwindspeed_config_t *hjwind_cfg;
  hjwindDirection_config_t *hjwindDir_cfg;
  rs485_config_t *rs485_cfg;
  hjsnow_config_t *hjsnow_cfg;
  uint8_t single_channel = 0;

  memset(&hj_config, 0, sizeof(hj_config));

  memset(&g_config_sensor, 0, sizeof(g_config_sensor));
  save_config_sensor();

  // ¿Âµµ ¼¾¼­[È­Áø ¿Âµµ 9600]
  hj_config.sensor[A1_TEMPERATURE].type = S_T_TEMPERATURE_HJ_485;
  sensor_add(&hj_config.sensor[A1_TEMPERATURE]);
  hjwind_cfg = get_sensor_config(&hj_config.sensor[A1_TEMPERATURE]);
  hjtemp_cfg->rs485_port = RS485_A;

  // ½Àµµ ¼¾¼­[È­Áø ½Àµµ 9600]
  hj_config.sensor[A10_RELATIVE_HUMIDITY].type = S_T_HUMI_HJ_485;
  sensor_add(&hj_config.sensor[A10_RELATIVE_HUMIDITY]);
  hjwind_cfg = get_sensor_config(&hj_config.sensor[A10_RELATIVE_HUMIDITY]);
  hjtemp_cfg->rs485_port = RS485_A;

  // Ç³Çâ[È­Áø RS485 Ç³Çâ 19200]
  hj_config.sensor[A2_WIND_DIRECTION].type = S_T_WIND_DIRECTION_HJ_485;
  sensor_add(&hj_config.sensor[A2_WIND_DIRECTION]);
  hjwindDir_cfg = get_sensor_config(&hj_config.sensor[A2_WIND_DIRECTION]);
  hjwindDir_cfg->rs485_port = RS485_B;

  // Ç³¼Ó[È­Áø RS485 Ç³¼Ó 19200]
  hj_config.sensor[A3_WIND_SPEED].type = S_T_WIND_SPEED_HJ_485;
  sensor_add(&hj_config.sensor[A3_WIND_SPEED]);
  hjwind_cfg = get_sensor_config(&hj_config.sensor[A3_WIND_SPEED]);
  hjwind_cfg->rs485_port = RS485_B;
  hjwind_cfg->full = 3200;
  hjwind_cfg->offset = 0;

  // °­¿ì°¨Áö[È­Áø Á¢Á¡]
  hj_config.sensor[A8_RAIN_PRESENT].type = S_T_RAIN_PRESENT_DI;

  // °­¼ö·®[¸®µåÇü]
  hj_config.sensor[A6_RAINFALL_DOT5_1MM].type = S_T_RAIN_REED_1MM;

  // ¼ø°£ Ç³Çâ
  hj_config.sensor[A4_INSTANT_WIND_DIRECTION].type = S_T_WIND_DIRECTION_MAX_VAL;

  // ¼ø°£ Ç³¼Ó
  hj_config.sensor[A5_INSTANT_WIND_SPEED].type = S_T_WIND_SPEED_MAX_VAL;

  // Àû¼³[È­Áø RS485 19200]
  hj_config.sensor[A9_SNOW_DEPTH].type = S_T_SNOW_HJ_232;
  sensor_add(&hj_config.sensor[A9_SNOW_DEPTH]);
  hjsnow_cfg = get_sensor_config(&hj_config.sensor[A9_SNOW_DEPTH]);
  hjsnow_cfg->port = eRS232_1;

  // ±â¾Ð[RM YOUNG]
  hj_config.sensor[A7_PRESSURE].type = S_T_ADC;
  sensor_add(&hj_config.sensor[A7_PRESSURE]);
  adc_config = get_sensor_config(&hj_config.sensor[A7_PRESSURE]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 200000;
  adc_config->lowScale = 0;
  adc_config->scale = 100;

  save_config_sensor();

  config = hj_config;
  save_config_app();
  save_config_sensor();
}

// ÃÊ±âÈ­
int32_t menu_manage_config_reset(p_shell_context_t ctx)
{
  int32_t cnt;
  const char *config_menu[] = {"0.AWS È­Áø ±âº» ¼³Á¤", "1.°øÀå ÃÊ±âÈ­ "};

  cnt = select_indexFromList(ctx, config_menu, NULL, _countof(config_menu), false);

  if (cnt > 0)
  {
    cnt--;
    switch (cnt)
    {
      case 0:
        config_hj_reset();

        break;
      case 1:
        memset(config.sensor, 0, sizeof(config.sensor));

        WRITE_CFG(sensor);
        memset(&g_config_sensor, 0, sizeof(g_config_sensor));
        save_config_sensor();
        break;
    }
  }

  return 0;
}

int32_t menu_manage_print_config_all(p_shell_context_t ctx)
{
  ctx->printf("ID               :%d\r\n", config.id);
  ctx->printf("ºñ¹Ð¹øÈ£         :%d\r\n", config.password);
  ctx->printf("ÃæÀü±â Á¾·ù      :%s\r\n", ITEM_LIST(config.charger_model, g_chgList));
  ctx->printf("·Î±× Ä«¿îÆ®      :%d\r\n", g_config_nvm.logCnt);

  ctx->printf("ÀÌ´õ³Ý ¼­ºê³Ý    :%d.%d.%d.%d\r\n", config.eth_subnet[0], config.eth_subnet[1],
              config.eth_subnet[2], config.eth_subnet[3]);
  ctx->printf("ÀÌ´õ³Ý °ÔÀÌÆ®¿þÀÌ:%d.%d.%d.%d\r\n", config.eth_gateway[0], config.eth_gateway[1],
              config.eth_gateway[2], config.eth_gateway[3]);
  ctx->printf("ÀÌ´õ³Ý IP        :%d.%d.%d.%d\r\n", config.eth_ip[0], config.eth_ip[1],
              config.eth_ip[2], config.eth_ip[3]);
  ctx->printf("ÀÌ´õ³Ý ¿ø°Ý ¼­¹ö :%d.%d.%d.%d\r\n", config.eth_server_ip[0], config.eth_server_ip[1],
              config.eth_server_ip[2], config.eth_server_ip[3]);
  ;

  ctx->printf("ÀÌ´õ³Ó Æ÷Æ®      :%d\r\n", config.eth_server_port);
  ctx->printf("ÀÌ´õ³Ý ÇÁ·ÎÅäÄÝ  :%s\r\n", ITEM_LIST(config.eth_protocol, protocolList));
  ctx->printf("CDMA ¿ø°Ý ¼­¹ö   :%d.%d.%d.%d\r\n", config.cdma_server_ip[0],
              config.cdma_server_ip[1], config.cdma_server_ip[2], config.cdma_server_ip[3]);
  ctx->printf("CDMA Æ÷Æ®        :%d\r\n", config.cdma_port);
  ctx->printf("CDMA ÇÁ·ÎÅäÄÝ    :%s\r\n", ITEM_LIST(config.cdma_protocol, protocolList));
  ctx->printf("CDMA Á¾·ù        :%s\r\n", ITEM_LIST(config.cdma_model, cdmaModellList));
  ctx->printf("ÀÌ´õ³Ý »ç¿ë      :%s\r\n", ITEM_LIST((int32_t)config.eth_use, enableList));
  ctx->printf("CDMA »ç¿ë        :%s\r\n", ITEM_LIST((int32_t)config.cdma_use, enableList));
  ctx->printf("Á÷Á¢Åë½Å         :%s\r\n", ITEM_LIST((int32_t)config.direct_use, enableList));
  ctx->printf("Á÷Á¢Åë½Å ÇÁ·ÎÅäÄÝ:%s\r\n", ITEM_LIST(config.direct_protocol, protocolList));
  ctx->printf("Á÷Á¢Åë½Å ¼Óµµ    :%d\r\n", config.direct_baud);
  ctx->printf("ÆÐ³Î Á¾·ù        :%s\r\n", ITEM_LIST(config.panel_model, panelList));
  ctx->printf("VHF ID           :%d\r\n", config.vhf_id);
  ctx->printf("VHF ±×·ì         :%d\r\n", config.vhf_group);
  ctx->printf("VHF HOST         :%d\r\n", config.vhf_host_id);
  ctx->printf("VHF Áß°è         :%d\r\n", config.vhf_repeater_id);
  ctx->printf("VHF PTT Áö¿¬     :%d\r\n", config.vhf_ptt_delay);

  return 0;
}
menu_func g_manageMenu[] = {
    [0] = menu_manage_version, menu_manage_device_reset, menu_manage_config_reset};

int32_t print_menu_manage(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");
  ctx->printf(" 0.¹öÀü\r\n");
  ctx->printf(" 1.Àåºñ ¸®¼Â\r\n");
  ctx->printf(" 2.¼³Á¤ °ª\r\n");
  cnt = 5;
  return cnt;
}

int32_t menu_manage(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_manage, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = g_manageMenu[cnt](ctx);
    if (cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);

  // return 0;//
}

int32_t print_menu_cali_adc(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");

  ctx->printf(" 0.offset  :%d\r\n", g_config_adc.single[0].offset);
  ctx->printf(" 1.fullset :%d\r\n", g_config_adc.single[0].fullset);

  cnt = 2;
  return cnt;
}

int32_t print_menu_cali_single(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");
  for (int i = 0; i < 18; i++)
  {
    ctx->printf("%2d.single channel %d\r\n", i, i);
  }
  cnt = 18;
  return cnt;
}

int32_t inpu_adc_cali(p_shell_context_t ctx, int adcMode, int channel, int32_t cfg_adc,
                      int32_t cfg_ref, int32_t *adc_data, int32_t *ref_vol)
{
  uint8_t err = 0;
  int32_t ch;
  int32_t adc;
  int32_t voltage = 0;

  ctx->printf("ADC %s,ch:%d\r\n", adcChModeList[adcMode], channel);
  ctx->printf("config adc:%d, ref:%d\r\n", cfg_adc, cfg_ref);
  do
  {
    adc = 0;
    if (adcMode == 0)  // single
    {
      adc = adc_read_single_avg(channel, &err, 10);
    }
    else
    {
      adc = adc_read_diff_avg(channel, &err, 10);
    }
    if (err)
    {
      ctx->printf("adc error:%d\r", err);
    }
    else
    {
      ctx->printf("current adc:%7d\r", adc);
    }

    osDelay(1000);
    ch = DbgConsole_GetcharNonBlocking();
    if (ch != -1)
    {
      break;
    }

  } while (1);

  ctx->printf("\r\n");
  vt100_printfColor(GREEN, "ADC °ªÀ» ¼öµ¿À¸·Î ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  if (input_digit(ctx, -8388607, 8388607, &adc, eUINT32) != 1)
  {
    return 1;
  }

  ctx->printf("\r\n");
  vt100_printfColor(GREEN, "ÀÔ·ÂµÈ Àü¾Ð°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä(mV):");
  if (input_digit(ctx, 0, 5000, &voltage, eUINT32) != 1)
  {
    return 1;
  }

  *adc_data = adc;
  *ref_vol = voltage;

  return 0;
}

int32_t menu_cali_single(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t channel;
  int32_t index;
  int32_t adc;
  int32_t voltage;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_cali_single, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    channel = cnt;

    while (1)
    {
      ctx->printf(" 0.offset  :%d\r\n", g_config_adc.single[channel].offset);
      ctx->printf(" 1.fullset :%d\r\n", g_config_adc.single[channel].fullset);
      ctx->printf("Please enter a number:");

      cnt = console_scanf("%d", &index);

      if (cnt == EXIT_BACK)
      {
        break;
      }
      if (cnt == EXIT_PROGRAM)
      {
        return cnt;
      }

      switch (index)
      {
        case 0:  // offset
          if (inpu_adc_cali(ctx, 0, channel, g_config_adc.single[channel].offset,
                            g_config_adc.single[channel].offset_input, &adc, &voltage) == 0)
          {
            ctx->printf("offset:%d, voltage:%d\r\n", adc, voltage);
            g_config_adc.single[channel].offset = adc;
            g_config_adc.single[channel].offset_input = voltage;
            WRITE_ADC(single[channel].offset);
            WRITE_ADC(single[channel].offset_input);
          }
          break;

        case 1:  // fullset
          if (inpu_adc_cali(ctx, 0, channel, g_config_adc.single[channel].fullset,
                            g_config_adc.single[channel].fullset_input, &adc, &voltage) == 0)
          {
            ctx->printf("offset:%d, voltage:%d\r\n", adc, voltage);
            g_config_adc.single[channel].fullset = adc;
            g_config_adc.single[channel].fullset_input = voltage;
            WRITE_ADC(single[channel].fullset);
            WRITE_ADC(single[channel].fullset_input);
          }
          break;
      }
    }
  } while (1);

  // return 0;//
}

int32_t print_menu_cali_diff(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");
  for (int i = 0; i < 8; i++)
  {
    ctx->printf("%2d.diff channel %d\r\n", i, i);
  }
  cnt = 18;
  return cnt;
}

int32_t menu_cali_diff(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t channel;
  int32_t index;
  int32_t adc;
  int32_t voltage;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_cali_diff, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    channel = cnt;

    ctx->printf(" 0.offset  :%d\r\n", g_config_adc.diff[channel].offset);
    ctx->printf(" 1.fullset :%d\r\n", g_config_adc.diff[channel].fullset);

    ctx->printf("num:");

    cnt = console_scanf("%d", &index);

    if (cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }

    switch (index)
    {
      case 0:  // offset
        if (inpu_adc_cali(ctx, 0, channel, g_config_adc.diff[channel].offset,
                          g_config_adc.diff[channel].offset_input, &adc, &voltage) == 0)
        {
          ctx->printf("offset:%d, voltage:%d\r\n", adc, voltage);
          g_config_adc.diff[channel].offset = adc;
          g_config_adc.diff[channel].offset_input = voltage;
          WRITE_ADC(diff[channel].offset);
          WRITE_ADC(diff[channel].offset_input);
        }
        break;
      case 1:  // fullset
        if (inpu_adc_cali(ctx, 0, channel, g_config_adc.diff[channel].fullset,
                          g_config_adc.diff[channel].fullset_input, &adc, &voltage) == 0)
        {
          ctx->printf("offset:%d, voltage:%d\r\n", adc, voltage);
          g_config_adc.diff[channel].fullset = adc;
          g_config_adc.diff[channel].fullset_input = voltage;
          WRITE_ADC(diff[channel].fullset);
          WRITE_ADC(diff[channel].fullset_input);
        }
        break;
    }

    if (cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);

  // return 0;//
}

int32_t menu_cali_config_all(p_shell_context_t ctx)
{
  float voltage;
  int32_t adc;
  uint8_t err = 0;
  int32_t off, full, off_in, full_in;

  ctx->printf(VT100_CLEAR_SCREEN);
  ctx->printf(VT100_CURSOR_OFF);

  do
  {
    ctx->printf(VT100_CURSOR_HOME);

    ctx->printf("%-10s %-2s %-7s %-10s %-10s %-10s %-12s %-10s\r\n", "Mode", "Ch", "offset",
                "fullset", "o_in(mv)", "f_in(mv)", "adc_avg(10)", "voltage(v)");

    for (int i = 0; i < 18; i++)
    {
      adc = adc_read_single_raw(i, &err);
      off = g_config_adc.single[i].offset;
      full = g_config_adc.single[i].fullset;
      off_in = g_config_adc.single[i].offset_input;
      full_in = g_config_adc.single[i].fullset_input;
      voltage = cvt_adcToVol(adc, off, full, off_in, full_in);
      if (err)
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12s %-10s \r\n", "Single", i + 1, off,
                    full, off_in, full_in, "error", " ");
        ;
      }
      else
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12d %-8.6f \r\n", "Single", i + 1, off,
                    full, off_in, full_in, adc, voltage / 1000.0);
        ;
      }
    }

    for (int i = 0; i < 8; i++)
    {
      adc = adc_read_diff_raw(i, &err);
      off = g_config_adc.diff[i].offset;
      full = g_config_adc.diff[i].fullset;
      off_in = g_config_adc.diff[i].offset_input;
      full_in = g_config_adc.diff[i].fullset_input;
      voltage = cvt_adcToVol(adc, off, full, off_in, full_in);
      if (err)
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12s %-10s \r\n", "Diff", i + 1, off, full,
                    off_in, full_in, "error", " ");
        ;
      }
      else
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12d %-8.6f \r\n", "Diff", i + 1, off, full,
                    off_in, full_in, adc, voltage / 1000.0);
        ;
      }
    }
  } while (wait_break(1000));

  return 0;
}

bool check_password(void)
{
  int32_t password;

  debug_printf("Please enter the password:\r\n");

  if (console_scanf("%d", &password) == 1)
  {
    if (password == 7777)
    {
      return true;
    }
  }

  debug_printf("The password does not match\r\n");
  return false;
}

int32_t menu_cali_config_factory(p_shell_context_t ctx)
{
  if (check_password() != true)
  {
    return 0;
  }

  for (int i = 0; i < 18; i++)
  {
    g_config_adc.single[i].offset = -2559;
    g_config_adc.single[i].offset_input = 0;
    g_config_adc.single[i].fullset = 8384783;
    g_config_adc.single[i].fullset_input = 5000;
  }

  for (int i = 0; i < 8; i++)
  {
    g_config_adc.diff[i].offset = 63325;
    g_config_adc.diff[i].offset_input = 0;
    g_config_adc.diff[i].fullset = 8319265;
    g_config_adc.diff[i].fullset_input = 5000;
  }

  save_config_adc();

  debug_printf("+config facory:ok\r\n");
  return 0;
}

#include "driver_uart.h"
driver_t *g_osc_port;

int32_t g_adc;
/**
 * @brief 1ÃÊ¸¶´Ù ADC °ªÀ» Ãâ·Â
 */
int32_t menu_cali_print_adc(p_shell_context_t ctx)
{
  char buff[30];

  int32_t channel;
  int32_t cnt;
  int32_t adc;
  uint8_t err;
  float voltage;
  eADC_CH_TYPE_t adcMode;
  int32_t start, stop;
  uint32_t start_time;
  uint32_t elased_time;
  uint32_t sample_cnt = 0;
  float avg = 0;

  uart_config_t uart_config = {.dataLen = UART_DATA_LEN_8, .stop_bit = 0};
  uart_config.baud = 115200;
  uart_config.parityIdx = 0;
  g_osc_port = driver_uart_open(UART_0_D_SUB_0, &uart_config);

  ctx->printf("Ã¤³Î ¸ðµå¸¦ ¼±ÅÃÇØÁÖ¼¼¿ä\r\n");

  cnt = select_indexFromList(ctx, adcChModeList, NULL, _countof(adcChModeList), true);

  if (cnt <= 0)
  {
    return 0;
  }

  cnt--;
  adcMode = (eADC_CH_TYPE_t)cnt;

  // ctx->printf("Ã¤³Î ¹øÈ£¸¦ ÀÔ·ÂÇØÁÖ¼¼¿ä\r\n");
  vt100_printfColor(GREEN, "Ã¤³Î ¹øÈ£¸¦ ÀÔ·ÂÇØÁÖ¼¼¿ä:");

  if (adcMode == eSINGLE_ADC)
  {
    start = 1;
    stop = 18;
  }
  else
  {
    start = 1;
    stop = 8;
  }

  if (input_decimal(ctx, start, stop, &channel) == 1)
  {
    channel--;  // 0±âÁØÀ¸·Î
    do
    {
      start_time = mcu_get_clk();
      if (adcMode == eSINGLE_ADC)  // single
      {
        adc = adc_read_single_raw(channel, &err);
      }
      else  // diff
      {
        adc = adc_read_diff_raw(channel, &err);
      }

      g_adc = adc;
      elased_time = cal_elapsed_us(start_time);
      voltage = adc_chToVoltage(adcMode, channel, adc);
      make_timeToStr(&Date_Time, buff, sizeof(buff));
      ctx->printf("%s MODE:%s CH:%d ADC:%8d %8.6f %.3fms\r\n", buff,
                  adcMode == eSINGLE_ADC ? "s" : "d", channel + 1, adc, voltage,
                  elased_time / 1000.0f);
      // ctx->printf("{\"data1\":%d}\r\n",adc);
      // ctx->printf("%d,\r\n",adc);
      snprintf(buff, sizeof(buff), "%d,\r\n", adc);
      driver_uart_send(g_osc_port, buff, strlen(buff));
    } while ( wait_break(200));
  }
  return 0;
}

int32_t menu_cali_print_adc_all(p_shell_context_t ctx)
{
  char buff[30];
  uint8_t err;
  int32_t adc;
  float voltage;

  do
  {
    make_timeToStr(&Date_Time, buff, sizeof(buff));
    ctx->printf("%s,", buff);

    for (int i = 0; i < 18; i++)
    {
      adc = adc_read_single_avg(i, &err, 5);

      if (err)
      {
        voltage = NAN;
      }
      else
      {
        voltage = adc_chToVoltage(0, i, adc);
      }

      ctx->printf("%2d:%7d,%8.6f ", i + 1, adc, voltage);
    }
    ctx->printf("\r\n");

  } while (wait_break(1000));

  return 0;
}

menu_func g_calibraionMenu[] = {[0] = menu_cali_single,  menu_cali_diff,
                                menu_cali_config_all,    menu_cali_print_adc,
                                menu_cali_print_adc_all, menu_cali_config_factory};

int32_t print_menu_calibration(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");
  ctx->printf(" 0.single\r\n");
  ctx->printf(" 1.differential\r\n");
  ctx->printf(" 2.config all\r\n");
  ctx->printf(" 3.print adc\r\n");
  ctx->printf(" 4.print adc single all\r\n");
  ctx->printf(" 5.config factory reset\r\n");
  cnt = 6;
  return cnt;
}

int32_t menu_calibration(p_shell_context_t ctx)
{
  run_calibration_menu();
  

  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_calibration, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = g_calibraionMenu[cnt](ctx);
    if (cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);

  // return 0;//
}

int32_t menu_developer_interrupt(p_shell_context_t ctx)
{
  PrintAllInterrupts();
  return 0;
}

void print_flash(uint32_t start, uint32_t size, uint32_t width)
{
  uint8_t buff[512];
  uint32_t quot;
  uint32_t rem;
  uint32_t i;

  quot = size / 512;
  rem = size % 512;

  for (i = 0; i < quot; i++)
  {
    flash_read(start + i * 512, buff, 512, 512);
    LOG_MEM(buff, sizeof(buff), start + i * 512, width);
  }

  if (rem)
  {
    flash_read(start + i * 512, buff, 512, rem);
    LOG_MEM(buff, rem, start + i * 512, width);
  }
}

int32_t menu_developer_memory(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t inCnt;
  int32_t start, size, len;

  const char *memList[] = {"flash", "fram"};

  cnt = select_indexFromList(ctx, memList, NULL, _countof(memList), true);

  if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
  {
    return cnt;
  }

  ctx->printf("start,size,len>>");

  inCnt = console_scanf("%d,%d,%d", &start, &size, &len);
  if (inCnt == EXIT_BACK || inCnt == EXIT_PROGRAM && cnt <= 0)
  {
    return inCnt;
  }

  cnt--;
  switch (cnt)
  {
    case 0:  // flash;
      print_flash(start, size, len);
      break;
    case 1:  // fram
      break;
  }
  return 0;
}

int32_t print_developer_sensor(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  //  int32_t i=0;

  ctx->printf("\r\n");
  for (int i = 0; i < _countof(sensor_name_list); i++)
  {
    //  ctx->printf("%2d.%-15s:%s,%d\r\n",i,sensor_name_list[i],
    //  ITEM_LIST(g_sensor_emul[i].use,enableList),g_sensor_emul[i].data);
    cnt++;
  }

  return cnt;
}

int32_t menu_developer_sensor(p_shell_context_t ctx)
{
  int32_t cnt;
  // int32_t inCnt;
  // int32_t start,size,len;
  //  float fVal;
  int32_t dec;
  int32_t use;
  while (1)
  {
    cnt = select_indexFromList(ctx, NULL, print_developer_sensor, 0, true);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }

    cnt--;

    ctx->printf("use,data:");
    if (console_scanf("%d,%d", &use, &dec) == 2)
    {
     // g_sensor_emul[cnt].enable = use;
     // g_sensor_emul[cnt].data.i = dec;
    }
  }
}

int32_t print_menu_developer(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");
  ctx->printf("%2d.interrupt\r\n", cnt++);
  ctx->printf("%2d.memory\r\n", cnt++);
  ctx->printf("%2d.sensor emul\r\n", cnt++);
  ctx->printf("%2d.print sensor config\r\n", cnt++);
  ctx->printf("%2d.view system logging\r\n", cnt++);
  return cnt;
}

int32_t menu_developer_sensor_config(p_shell_context_t ctx)
{
  // char opt[20];
  int32_t cnt = 0;
  int i = 0;

  ctx->printf("\r\n");

#if 1
  cnt = _countof(sensor_name_list);

  for (i = 0; i < cnt; i++)
  {
    ctx->printf("%2d:%d", i, config.sensor[i].configCnt);
    for (int j = 0; j < 4; j++)
    {
      ctx->printf("[%-15s.%d]", ITEM_LIST(config.sensor[i].config[j][0], g_sensor_model_list),
                  config.sensor[i].config[j][1]);
    }

    ctx->printf("\r\n");
  }

#endif
  cnt = _countof(sensor_name_list);
  return cnt;
}

int32_t menu_developer_test_rs232(p_shell_context_t ctx) { return 0; }

int32_t print_menu_developer_test(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("%2d:RS232", cnt++);

  return cnt;
}

int32_t menu_developer_test(p_shell_context_t ctx)
{
  int32_t cnt;

  const menu_func menu[] = {[0] = menu_developer_test_rs232};

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_developer_test, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if (cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);
}

int32_t menu_developer_logging(p_shell_context_t ctx)
{
  int32_t startCnt, endCnt;
  loggingMsg_t log;
  int32_t cnt;
  int32_t year, month, day, hour, min, sec;

  do
  {
    debug_printf("·Î±× ½ÃÀÛ Ä«¿îÆ®:%d\r\n", logging_get_logCnt());
    debug_printf("start,end>>");

    cnt = console_scanf("%d,%d,%d", &startCnt, &endCnt);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }

    if (cnt == 2)
    {
      for (int32_t i = startCnt; i <= endCnt; i++)
      {
        logging_read_log(i, &log);
        sscanf(log.msg, "%02d%02d%02d%02d%02d%02d", &year, &month, &day, &hour, &min, &sec);
        debug_printf("%4d,%04d-%02d-%02d %02d:%02d:%02d,%s\r\n", i, year + 2000, month, day, hour,
                     min, sec, &log.msg[13]);
      }
    }
  } while (1);
}
int32_t menu_developer(p_shell_context_t ctx)
{
  int32_t cnt;
  const menu_func menu[] = {[0] = menu_developer_interrupt,
                            menu_developer_memory,
                            menu_developer_sensor,
                            menu_developer_sensor_config,
                            menu_developer_logging};
  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_developer, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if (cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);
}

const menuFunc_t menuFunc[] = {{.title = "0.diplay", .func = menu_display},
                               {.title = "1.system", .func = menu_system},
                               {.title = "2.sensor", .func = menu_sensor},
                               {.title = "3.network", .func = menu_network},
                               {.title = "4.data", .func = menu_data},
                               {.title = "5.display panel", .func = menu_display_panel},
                               {.title = "6.manage", .func = menu_manage},
                               {.title = "7.calibraion", .func = menu_calibration},
                               {.title = "8.developer", .func = menu_developer}};

int32_t print_menu_root(p_shell_context_t ctx)
{
  int i;
  for (i = 0; i < _countof(menuFunc); i++)
  {
    ctx->printf("%s\r\n", menuFunc[i].title);
  }
  return i;
}

int32_t menu_root_(p_shell_context_t ctx, int32_t argc, char **argv)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_root, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt = menuFunc[cnt - 1].func(ctx);
  } while (cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t select_menu(select_menu_t *select_menu)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(select_menu->ctx, select_menu->list, select_menu->func,
                               select_menu->cnt, select_menu->show);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt = select_menu->menuFunc[cnt - 1].func(select_menu->ctx);
  } while (cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t menu_root(p_shell_context_t ctx, int32_t argc, char **argv)
{
  select_menu_t menu;

  menu.ctx = ctx;
  menu.list = NULL;
  menu.func = print_menu_root;
  menu.cnt = 0;
  menu.show = false;
  menu.menuFunc = menuFunc;

  return select_menu(&menu);
}