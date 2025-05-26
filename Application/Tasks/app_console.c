
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
#include "app_file.h"
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
#include "cli_key_code.h"
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
#include "config_manager.h"
#include "console_utile.h"
#include "console_aws_display.h"
#include "console_data.h"
#include "cli_input.h"
#include "Sensors\temperature\hj_temperature.h"
#include "Update\update_fw.h"
#include "console_rtos.h"
#include "Protocols\divas\divas_protocol_handler.h"
#define EXIT_PROGRAM -3
#define EXIT_BACK -1



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
  int32_t (*func)(p_shell_context_t ctx);// ¸ñ·Ï Ãâ·Â ÇÔ¼ö
  uint8_t cnt;
  bool show;
  const menuFunc_t *menuFunc;//ÇÔ¼ö Å×ÀÌºí 
} select_menu_t;

const char *protocolList[] = {"KMA2", "KMA3"};
const char *cdmaModellList[] = {"NTLE9607", "TX700"};
const char *panelList[] = {"STD", "MOOJU","HANSUNG"};


const char *g_chgList[] = {"smart charger", "aws charger"};


const char *unusedList[] = {"¹Ì»ç¿ë"};
const char *adcChModeList[] = {"single", "diff"};
const char *rs232ParityList[] = {"none", "even", "odd"};
const char *enableList[] = {"¹Ì»ç¿ë", "»ç¿ë"};
const char *ethModeList[] = {"Å¬¶óÀÌ¾ðÆ®", "¼­¹ö"};

const char *physical_list[]={"RS232","RS485"};

int32_t print_common_cfg(p_shell_context_t ctx, sensor_t *sensor, uint8_t c);



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

/**
 * @retval 0º¸´Ù Å©¸é »ç¿ëÀÚ ÀÔ·ÂÀÌ ÀÖÀ½ 
 */
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

  debug_printf("¹üÀ§:%d~%d\r\n", start, stop);
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


int input_float(p_shell_context_t ctx, float start, float stop, float *target)
{
  int32_t cnt;
  float fVal;

  ctx->printf("\r\n¹üÀ§:%f~%f\r\n", start, stop);
  vt100_printfColor(GREEN, "°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%f", &fVal);
  if (cnt == 1)
  {
    if (fVal < start || fVal > stop)
    {
      vt100_printfColor(RED, "ÀÔ·Â°ªÀ» ¹üÀ§¸¦ È®ÀÎÇØ ÁÖ¼¼¿ä\r\n");
      return 0;
    }

    *target = fVal;
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
  const char *list[10]={" "};

  out[0] = 0;

  cfg = get_sensor_config(sensor);

  if (cfg == NULL)
  {
    snprintf(out, outSize, "%s"," ");
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
        snprintf(out, outSize, "[%s]", list[hjtemp->port]);
      }
      else
      {
        rs485_get_portList(list, sizeof(list));
        snprintf(out, outSize, "[%s]", list[hjtemp->port]);
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
      snprintf(out, outSize, "[%s]", list[ott->port]);
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

int32_t print_menu_sensor_offset(p_shell_context_t ctx)
{
  char opt[20];
  int32_t cnt = 0;
  int i = 0;
  float offset;
  ctx->printf("\r\n");

#if 1
  cnt = _countof(sensor_name_list) / 2;

  for (i = 0; i < cnt; i++)
  {
    offset = get_config_app()->sensor[i].offset;
    ctx->printf("%2d.%-14s:%-24s %.2f,  ", i, sensor_name_list[i],
                ITEM_LIST(get_config_app()->sensor[i].type, g_sensor_model_list), offset);

    ctx->printf("%2d.%-14s:%-24s %.2f\r\n", i + cnt, sensor_name_list[i + cnt],
                ITEM_LIST(get_config_app()->sensor[i + cnt].type, g_sensor_model_list), offset);
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

/*nn.¹°¸®ÀåÄ¡  :RS232|RS485
  nn.Æ÷Æ®      :n
*/
#define HJTEMP_CFG_PHYSICAL_LAYER 0
#define HJTEMP_CFG_PORT           1
#define HJTEMP_CTRL_OFFSET        2
uint8_t print_hjtemp_cfg(p_shell_context_t ctx, hjtemp_config_t *hjtempCfg, uint8_t cnt)
{
  const char *portNameList[10];

  ctx->printf("%2d.¹°¸®ÀåÄ¡   :%s\r\n", cnt++, physical_list[hjtempCfg->physical_layer]);  // °íÁ¤

  if (hjtempCfg->physical_layer == ePHYSICAL_RS232)
  {
    rs232_get_portList(portNameList, _countof(portNameList));
  }
  else
  {
    rs485_get_portList(portNameList, _countof(portNameList));

  }
  ctx->printf("%2d.Æ÷Æ®       :%s\r\n", cnt++, portNameList[hjtempCfg->port]);  // °íÁ¤
  ctx->printf("%2d.¿ÀÇÁ¼Â[Á¦¾î]\r\n", cnt++);  // °íÁ¤
  return cnt;
}

#define OTT_SMP3_CFG_PORT 0
uint8_t print_ott_smp3_cfg(p_shell_context_t ctx, ott_smp3_config_t *ott, uint8_t cnt)
{
  const char *portNameList[10];

   rs485_get_portList(portNameList, _countof(portNameList));
   ctx->printf("%2d.Æ÷Æ®       :%s\r\n", cnt++, portNameList[ott->port]);  // °íÁ¤
   return cnt;
}

#define HJSNOW_CFG_MENU_PHY 0
#define HJSNOW_CFG_MENU_PORT 1
uint8_t print_hjsnow_cfg(p_shell_context_t ctx, hjsnow_config_t *hjsnow, uint8_t cnt)
{
  const char *portNameList[10];

  ctx->printf("%2d.¹°¸®ÀåÄ¡   :%s\r\n", cnt++,
              physical_list[hjsnow->physical_layer]);  // °íÁ¤

  if(hjsnow->physical_layer == ePHYSICAL_RS232)
  {
    rs232_get_portList(portNameList, _countof(portNameList));
  }
  else
  {
    rs485_get_portList(portNameList, _countof(portNameList));
  }

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

void adc_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t menu_index)
{
  int32_t dec;
  adc_config_t *adc;
  int32_t row_index;

  adc = get_sensor_config(sensor);
  switch (menu_index)
  {
    case ADC_SET_CH_MODE:  // 1.Ã¤³Î ¸ðµå
      row_index = select_indexFromList(ctx, adcChModeList, NULL, _countof(adcChModeList), true);

      if (row_index > 0)
      {
        adc->mode = (row_index - 1);
        save_config_sensor();
      }
      break;
    case ADC_SET_CHANNLEL:  // channel;
      if (input_decimal(ctx, 0, 17, &dec))
      {
        adc->channel = dec;
        save_config_sensor();
      }
      break;
    case ADC_SET_HIGHSCALE:  // hish cale;
      if (input_decimal(ctx, -1000000, 1000000, &dec))
      {
        adc->highScale = dec;
        save_config_sensor();
      }
      break;
    case ADC_SET_LOWSCALE:  // low cale;
      if (input_decimal(ctx, -1000000, 1000000, &dec))
      {
        adc->lowScale = dec;
        save_config_sensor();
      }
      break;
    case ADC_SET_SCALE:  // ale;
      if (input_decimal(ctx, -1000000, 1000000, &dec))
      {
        adc->scale = dec;
        save_config_sensor();
      }
      break;

    case ADC_SET_OUTMAXVOLT:
      if (input_decimal(ctx, -1000000, 1000000, &dec))
      {
        adc->outMaxV = dec;
        save_config_sensor();
      }
      break;

    case ADC_SET_OUTMINVOLT:
      if (input_decimal(ctx, -1000000, 1000000, &dec))
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

      if (cnt > 0)
      {
        rs232->port = cnt - 1;
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
      if(cnt>0)
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

      if (cnt>0)
      {


        rs485->port = cnt - 1;

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
      if(cnt>0)
      {
        rs485->parityIdx = cnt - 1;
        save_config_sensor();
      }
      break;
    default:
      break;
  }
}

void hjwind_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t munu_index)
{
  const char *portList[10];
  int32_t dec;
  uint16_t port_cnt;
  int32_t row_index;
  hjwindspeed_config_t *hjwind;

  hjwind = get_sensor_config(sensor);
  if (hjwind == NULL)
  {
    return;
  }
  switch (munu_index)
  {
    case HJWIND_CFG_FULL:
      if (input_decimal(ctx, 0, 999999, &dec))
      {
        hjwind->full = dec;
        save_config_sensor();
      }
      break;
    case HJWIND_CFG_OFF:
      if (input_decimal(ctx, 0, 999999, &dec))
      {
        hjwind->offset = dec;
        save_config_sensor();
      }
      break;
    case HJWIND_CFG_PORT:
      port_cnt = rs485_get_portList(portList, _countof(portList));
      row_index = select_indexFromList(ctx, portList, NULL, port_cnt, true);

      if (row_index > 0)
      {
        hjwind->rs485_port = row_index - 1;
        save_config_sensor();
      }
      break;
    default:
      break;
  }
}

void hjwinddir_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t menu_index)
{
  int32_t dec;
  hjwindspeed_config_t *hjwind;
  const char *portList[10];
  int32_t row_index;
  uint16_t port_cnt;
  
  hjwind = get_sensor_config(sensor);
  if (hjwind == NULL)
  {
    return;
  }
  switch (menu_index)
  {
    case HJWIND_DIR_CFG_PORT:
      port_cnt = rs485_get_portList(portList, _countof(portList));

      row_index = select_indexFromList(ctx, portList, NULL, port_cnt, true);

      if (row_index > 0)
      {
        hjwind->rs485_port = row_index - 1;

        save_config_sensor();
      }
      break;
    default:
      break;
  }
}


/*
0.type:È­Áø RS485 9600
1.port:EX1 RS485 A
*/

void hjtemp_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t menu_index)
{
  int32_t row_idx;
    int32_t dec;
  hjtemp_config_t *hjtemp;
  const char *portList[10];
  uint16_t portListCnt;
  
  hjtemp = get_sensor_config(sensor);
  if (hjtemp == NULL)
  {
    return;
  }

  switch (menu_index)
  {
    case HJTEMP_CFG_PHYSICAL_LAYER:
      row_idx = select_indexFromList(ctx, physical_list, NULL, _countof(physical_list), true);
      
      if(row_idx > 0)
      {
        hjtemp->physical_layer = row_idx - 1;
        save_config_sensor();
      }
      break;
    case HJTEMP_CFG_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        row_idx = select_indexFromList(ctx, portList, NULL,  portListCnt, true);
        if (row_idx > 0)
        {
          hjtemp->port = row_idx - 1;
          save_config_sensor();
        }
      }
      else
      {
        portListCnt = rs485_get_portList(portList, _countof(portList));
        row_idx = select_indexFromList(ctx, portList, NULL, portListCnt, true);

        if (row_idx > 0)
        {
          hjtemp->port = row_idx - 1;
          save_config_sensor();
        }
      }

      break;
      case HJTEMP_CTRL_OFFSET:
      {
        driver_t *hj_temp;
        hjtemp_config_t *hjtemp_config;
        uint16_t offset=0;
        int32_t ret;
        
        hjtemp_config = get_sensor_config(&get_config_app()->sensor[A1_TEMPERATURE]);
  
        hj_temp = hjTemperature_open(HJ_TEMPERATURE,hjtemp_config);
  
        ret = hjTemperature_get(hj_temp,eTEMP_GET_OFFSET,&offset);
        if(ret ==0)
        {
          debug_printf("ÇöÀç ¿Âµµ ¿ÀÇÁ¼Â:%.2f\r\n",((float)offset/100.0f));
          if(get_user_confirm("¿ÀÇÁ¼ÂÀ» º¯°æÇÏ½Ã°Ú½À´Ï±î?")==1)
          {
            float f_offset;
            debug_printf("¿ÀÇÁ¼ÂÀ» ÀÔ·ÂÇØÁÖ¼¼¿ä>>");
            if(input_float(ctx,-5,5,&f_offset))
            {
              offset = (uint16_t)(f_offset*100);
              hjTemperature_set(hj_temp,eTEMP_SET_OFFSET,(void *)offset);
            }
          }
        }
        else
        {
          debug_printf("ÀåÄ¡¿¡ Á¢±ÙÇÒ ¼ö ¾ø½À´Ï´Ù.\r\n");
        }
      }
    default:
      break;
  }
}

void hjhumi_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t menu_index)
{
  int32_t row_idx;
    int32_t dec;
  hjtemp_config_t *hjtemp;
  const char *portList[10];
  uint16_t portListCnt;
  
  hjtemp = get_sensor_config(sensor);
  if (hjtemp == NULL)
  {
    return;
  }

  switch (menu_index)
  {
    case HJTEMP_CFG_PHYSICAL_LAYER:
      row_idx = select_indexFromList(ctx, physical_list, NULL, _countof(physical_list), true);
      
      if(row_idx > 0)
      {
        hjtemp->physical_layer = row_idx - 1;
        save_config_sensor();
      }
      break;
    case HJTEMP_CFG_PORT:
      if (hjtemp->physical_layer == ePHYSICAL_RS232)
      {
        portListCnt = rs232_get_portList(portList, _countof(portList));
        row_idx = select_indexFromList(ctx, portList, NULL,  portListCnt, true);
        if (row_idx > 0)
        {
          hjtemp->port = row_idx - 1;
          save_config_sensor();
        }
      }
      else
      {
        portListCnt = rs485_get_portList(portList, _countof(portList));
        row_idx = select_indexFromList(ctx, portList, NULL, portListCnt, true);

        if (row_idx > 0)
        {
          hjtemp->port = row_idx - 1;
          save_config_sensor();
        }
      }
      break;
    case HJTEMP_CTRL_OFFSET:
    {
      driver_t *hj_temp;
      hjtemp_config_t *hjtemp_config;
      uint16_t offset=0;
      int32_t ret;
      
      hjtemp_config = get_sensor_config(&get_config_app()->sensor[A1_TEMPERATURE]);

      hj_temp = hjTemperature_open(HJ_TEMPERATURE,hjtemp_config);

      ret = hjTemperature_get(hj_temp,eHUMI_GET_OFFSET,&offset);
      if(ret ==0)
      {
        debug_printf("ÇöÀç ½Àµµ ¿ÀÇÁ¼Â:%.2f\r\n",((float)offset/100.0f));
        if(get_user_confirm("¿ÀÇÁ¼ÂÀ» º¯°æÇÏ½Ã°Ú½À´Ï±î?")==1)
        {
          float f_offset;
          debug_printf("¿ÀÇÁ¼ÂÀ» ÀÔ·ÂÇØÁÖ¼¼¿ä>>");
          if(input_float(ctx,-5,5,&f_offset))
          {
            offset = (uint16_t)(f_offset*100);
            hjTemperature_set(hj_temp,eHUMI_SET_OFFSET,(void *)offset);
          }
        }
      }
      else
      {
        debug_printf("ÀåÄ¡¿¡ Á¢±ÙÇÒ ¼ö ¾ø½À´Ï´Ù.\r\n");
      }
    }
    break;
    default:
      break;
  }
}

void ott_smp3_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t menu_index)
{
  int32_t row_idx;
  int32_t dec;
  ott_smp3_config_t *ott;
  const char *portList[10];
  uint16_t portListCnt;

  ott = get_sensor_config(sensor);
  if (ott == NULL)
  {
    return;
  }

  switch (menu_index)
  {
    case OTT_SMP3_CFG_PORT:
      portListCnt = rs485_get_portList(portList, _countof(portList));
      row_idx = select_indexFromList(ctx, portList, NULL, portListCnt, true);

      if (row_idx > 0)
      {
        ott->port = row_idx - 1;
        save_config_sensor();
      }

      break;

    default:
      break;
  }
}
void hjsnow_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t menu_index)
{
  int32_t dec;
  hjsnow_config_t *hjsnow;
  const char *portList[10];
  uint16_t portCnt;
  int32_t row_index;
  hjsnow = get_sensor_config(sensor);
  if (hjsnow == NULL)
  {
    return;
  }
  switch (menu_index)
  {
    case HJSNOW_CFG_MENU_PHY:
      row_index = select_indexFromList(ctx, physical_list, NULL, _countof(physical_list), true);

      if (row_index > 0)
      {
        hjsnow->physical_layer = row_index - 1;
        save_config_sensor();
      }
      break;
    case HJSNOW_CFG_MENU_PORT:
      if (hjsnow->physical_layer == ePHYSICAL_RS232)
      {
        portCnt = rs232_get_portList(portList, _countof(portList));
        row_index = select_indexFromList(ctx, portList, NULL, portCnt, true);
        if (row_index > 0)
        {
          hjsnow->port = row_index - 1;
          save_config_sensor();
        }
      }
      else
      {
        portCnt = rs485_get_portList(portList, _countof(portList));
        row_index = select_indexFromList(ctx, portList, NULL, portCnt, true);
        if (row_index > 0)
        {
          hjsnow->port = row_index - 1;
          save_config_sensor();
        }
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
    {.sensorType = S_T_SNOW_HJ, .config_set = hjsnow_config_set},
    {.sensorType = S_T_GENERAL_485, .config_set = rs485_config_set},
    {.sensorType = S_T_TEMPERATURE_HJ, .config_set = hjtemp_config_set},
    {.sensorType = S_T_HUMINITY_HJ, .config_set = hjhumi_config_set},
    {.sensorType = S_T_SOLAR_RADIATION_OTT_SMP3, .config_set = ott_smp3_config_set}};

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
    case S_T_SNOW_HJ:
      cnt = print_hjsnow_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_WIND_DIRECTION_HJ_485:
      cnt = print_hjwindDir_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_WIND_SPEED_HJ_485:
      cnt = print_hjwind_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_TEMPERATURE_HJ:
      cnt = print_hjtemp_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_HUMINITY_HJ:
      cnt = print_hjtemp_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
      cnt = print_ott_smp3_cfg(ctx, get_sensor_config(sensor), cnt);
      break;
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
    //¸ðµç ¼¾¼­ÀÇ Ãâ·Â, ±âº»Á¤º¸ Ãâ·Â
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
    //¼±ÅÃµÈ ¼¾¼­ ¼³Á¤
    cnt = menu_sensor_default_2(ctx, (eSENSOR_LIST_t)(cnt - 1));
  } while (cnt != EXIT_PROGRAM);

  return cnt;
}


int32_t menu_offset(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_sensor_offset, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }

    debug_printf("%20s offset:%f\r\n",sensor_name_list[cnt-1],
      get_config_app()->sensor[cnt-1].offset);

    if(get_user_confirm("offsetÀ» º¯°æÇÏ½Ã°Ú½À´Ï±î?")==1)
    {
      float offset=0;
      debug_printf("¿ÀÇÁ¼ÂÀ» ÀÔ·ÂÇØÁÖ¼¼¿ä>>");
      cnt = console_scanf("%f", &offset);

      if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
      {
        return cnt;
      }
      if (cnt == 1)
      {
        set_sensor_offset(cnt-1,offset);
      }
      else
      {
        debug_printf("ÀÔ·Â°ª¿¡ ¿À·ù°¡ ÀÖ½À´Ï´Ù.");
      }

    }


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
      break;
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

  return cnt;
}

int32_t print_net_eth_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  ctx->printf("%2d.¹æ½Ä         :%s \r\n", cnt++, ITEM_LIST(get_config_app()->eth_mode, ethModeList));
  ctx->printf("%2d.¼öÁý ¼­¹ö Á¤º¸\r\n", cnt++);
  ctx->printf("%2d.±âº» ±¸¼º\r\n", cnt++);

  return cnt;
}

int32_t print_net_eth_remote_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  uint8_t *ip = get_config_app()->eth_server_ip;

  ctx->printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  ctx->printf("%2d.port    :%d\r\n", cnt++, get_config_app()->eth_server_port);
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
    }
  } while (1);
}

int32_t print_net_eth_default_set(p_shell_context_t ctx)
{
  int32_t cnt = 4;
  uint8_t *ip = config.eth_ip;
  uint8_t *gw = config.eth_gateway;
  uint8_t *subnet = config.eth_subnet;
  uint16_t local_port = config.eth_local_port;

  ctx->printf(" 0.ip      :%d.%d.%d.%d\r\n", ip[0], ip[1], ip[2], ip[3]);
  ctx->printf(" 1.subnet  :%d.%d.%d.%d\r\n", subnet[0], subnet[1], subnet[2], subnet[3]);
  ctx->printf(" 2.gateway :%d.%d.%d.%d\r\n", gw[0], gw[1], gw[2], gw[3]);
  ctx->printf(" 3.port    :%d\r\n", local_port);
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
      case 3:  // port
        ctx->printf("x:");
        if (console_scanf("%d%d", &a) == 1)
        {
          config.eth_local_port = a;

          WRITE_CFG(eth_local_port);
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

      config.eth_mode = cnt;
      WRITE_CFG(eth_mode);

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
  ctx->printf("%2d.model   :%s\r\n", cnt++, ITEM_LIST(config.cdma_model, cdmaModellList));
  return cnt;
}

int32_t print_net_ntle_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  uint8_t *ip = config.cdma_server_ip;
  int32_t port = config.cdma_port;

  ctx->printf("%2d.ip      :%d.%d.%d.%d\r\n", cnt++, ip[0], ip[1], ip[2], ip[3]);
  ctx->printf("%2d.port    :%d\r\n", cnt++, port);
  ctx->printf("%2d.model   :%s\r\n", cnt++, ITEM_LIST(config.cdma_model, cdmaModellList));
  ctx->printf("%2d.VPN     :%s\r\n", cnt++, ITEM_LIST(config.vpn_use, enableList));
  return cnt;
}


int32_t menu_net_cdma_set(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t a, b, c, d;
  int32_t dec;
  int32_t (*menu_set)(p_shell_context_t ctx) = print_net_cdma_set;


    do
    {
      if (get_config_app()->cdma_model == eCDMA_NTLE9607)
      {
        menu_set = print_net_ntle_set;
      }
      else
      {
        menu_set = print_net_cdma_set;
      }
      cnt = select_indexFromList(ctx, NULL, menu_set, 0, false);
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
        case 2:  // ¸ðµ¨
          cnt = select_indexFromList(ctx, cdmaModellList, NULL, _countof(cdmaModellList), true);
          if (cnt > 0)
          {
            cnt--;
            config.cdma_model = cnt;
            WRITE_CFG(cdma_model);
          }
          break;
        case 3:
          if (input_use(ctx, &get_config_app()->vpn_use))
          {
            WRITE_CFG(vpn_use);
          }
          break;
      }
    } while (1);
}

int32_t print_net_direct_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("%2d.baud     :%d\r\n", cnt++, config.direct_baud);

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
      break;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
  } while(1);

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

int32_t menu_net_protocol(p_shell_context_t ctx)
{
  int32_t cnt;

  
    cnt = select_indexFromList(ctx, protocolList, NULL, _countof(protocolList), true);

    if(cnt > 0)
    {
      config.aws_protocol_type = cnt-1;
      WRITE_CFG(aws_protocol_type);
    }
 

  return cnt;
}
int32_t print_menu_net(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  char buff[50] = {0};

  make_comList(buff, sizeof(buff));

  ctx->printf("%2d.Åë½Å ¹æ½Ä:%s\r\n", cnt++, buff);
  ctx->printf("%2d.Åë½Å ¼³Á¤\r\n", cnt++);
  ctx->printf("%2d.Åë½Å ÇÁ·ÎÅäÄÝ:%s\r\n", cnt++, 
                                 ITEM_LIST(get_config_app()->aws_protocol_type, protocolList));
  ctx->printf("%2d.VHF\r\n", cnt++);

  return cnt;
}

int32_t menu_network(p_shell_context_t ctx)
{
  int32_t cnt;

  const menu_func menu[] = {menu_net_use, menu_net_set, menu_net_protocol, menu_net_vhf};
  
  while(1)
  {
    cnt = select_indexFromList(ctx, NULL, print_menu_net, 0, false);
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      break;
    }
  }

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
  console_menu_data();
}

int32_t print_menu_panel(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  bool enalbe;
  ctx->printf("%2d.ÆÐ³Î Á¾·ù:%s\r\n", cnt++, ITEM_LIST(config.panel_model, panelList));
  
  //¹«ÁÖÀÎ °æ¿ì Ãß°¡ ¼³Á¤ Ãâ·Â
  if(get_config_app()->panel_model==ePANEL_MUJU)
  {
  enalbe = get_config_app()->panel_snow_use;
  ctx->printf("%2d.Àû¼³ Ãâ·Â:%s\r\n", cnt++, ITEM_LIST((int32_t)enalbe, enableList));

  enalbe = get_config_app()->panel_barometer_use;
  ctx->printf("%2d.±â¾Ð Ãâ·Â:%s\r\n", cnt++, ITEM_LIST((int32_t)enalbe, enableList));
 }
  return cnt;
}
int32_t aws_menu_display_panel(p_shell_context_t ctx)
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
        cnt = select_indexFromList(ctx, panelList, NULL, _countof(panelList), true);

        if (cnt > 0)
        {
          cnt--;
          config.panel_model = cnt;
          WRITE_CFG(panel_model);
        }
        break;
        case 1:
        if (input_use(ctx, &get_config_app()->panel_snow_use))
        {
          WRITE_CFG(panel_snow_use);
        }
        break;
        case 2:
        if (input_use(ctx, &get_config_app()->panel_barometer_use))
        {
          WRITE_CFG(panel_barometer_use);
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
  adc_config_t *adc_config;
  hjtemp_config_t *hjtemp_cfg;
  hjwindspeed_config_t *hjwind_cfg;
  hjwindDirection_config_t *hjwindDir_cfg;
  rs485_config_t *rs485_cfg;
  hjsnow_config_t *hjsnow_cfg;
  uint8_t single_channel = 0;

  config_app_reset();

  // config Áß ¼¾¼­ ¼³Á¤Á¤º¸¸¸ È­Áø¿¡ ¸Â°Ô ¼³Á¤ÇÑ´Ù.
  config_sensor_reset();
  
  // ¿Âµµ ¼¾¼­[È­Áø ¿Âµµ 9600]
  config.sensor[A1_TEMPERATURE].type = S_T_TEMPERATURE_HJ;
  sensor_add(&config.sensor[A1_TEMPERATURE]);
  hjtemp_cfg = get_sensor_config(&config.sensor[A1_TEMPERATURE]);
  hjtemp_cfg->physical_layer = ePHYSICAL_RS232;
  hjtemp_cfg->port = eRS232_RS485_B;

  // ½Àµµ ¼¾¼­[È­Áø ½Àµµ 9600]
  config.sensor[A10_RELATIVE_HUMIDITY].type = S_T_HUMINITY_HJ;
  sensor_add(&config.sensor[A10_RELATIVE_HUMIDITY]);
  hjtemp_cfg = get_sensor_config(&config.sensor[A10_RELATIVE_HUMIDITY]);
  hjtemp_cfg->physical_layer = ePHYSICAL_RS232;
  hjtemp_cfg->port = eRS232_RS485_B;

  // Ç³Çâ[È­Áø RS485 Ç³Çâ 19200]
  config.sensor[A2_WIND_DIRECTION].type = S_T_WIND_DIRECTION_HJ_485;
  sensor_add(&config.sensor[A2_WIND_DIRECTION]);
  hjwindDir_cfg = get_sensor_config(&config.sensor[A2_WIND_DIRECTION]);
  hjwindDir_cfg->rs485_port = RS485_A;

  // Ç³¼Ó[È­Áø RS485 Ç³¼Ó 19200]
  config.sensor[A3_WIND_SPEED].type = S_T_WIND_SPEED_HJ_485;
  sensor_add(&config.sensor[A3_WIND_SPEED]);
  hjwind_cfg = get_sensor_config(&config.sensor[A3_WIND_SPEED]);
  hjwind_cfg->rs485_port = RS485_A;
  hjwind_cfg->full = 3200;
  hjwind_cfg->offset = 0;

  // °­¿ì°¨Áö[È­Áø Á¢Á¡]
  config.sensor[A8_RAIN_PRESENT].type = S_T_RAIN_PRESENT_DI;

  // °­¼ö·®[¸®µåÇü]
  config.sensor[A6_RAINFALL_DOT5_1MM].type = S_T_RAIN_REED_1MM;

  // Àû¼³[È­Áø RS485 19200]
  config.sensor[A9_SNOW_DEPTH].type = S_T_SNOW_HJ;
  sensor_add(&config.sensor[A9_SNOW_DEPTH]);
  hjsnow_cfg = get_sensor_config(&config.sensor[A9_SNOW_DEPTH]);
  hjsnow_cfg->physical_layer  = ePHYSICAL_RS232;
  hjsnow_cfg->port = eRS232_HART_D;

  // ±â¾Ð[RM YOUNG]
  config.sensor[A7_PRESSURE].type = S_T_ADC;
  sensor_add(&config.sensor[A7_PRESSURE]);
  adc_config = get_sensor_config(&config.sensor[A7_PRESSURE]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 200000;
  adc_config->lowScale = 0;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÀÏ»ç CMP3 0~1.0VDC 
  config.sensor[B1_SOLAR_RADIATION].type = S_T_ADC;
  sensor_add(&config.sensor[B1_SOLAR_RADIATION]);
  adc_config = get_sensor_config(&config.sensor[B1_SOLAR_RADIATION]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 5000;//5v
  adc_config->lowScale = 0;//0v
  adc_config->scale = 1000;
  adc_config->outMaxV = 5000;
  adc_config->outMinV = 0;


  // ÀÏÁ¶ CSD3 ¼¾¼­ Ãâ·Â : 120 w/m2 ÀÌ»óÀÏ ¶§ 1 VDC, ÀÌÇÏÀÏ ¶§ 0 VDC
  // ¼¾¼­°ª ÀÚÃ¼¸¦ Àü¾ÐÀ¸·Î ¹Þ´Â´Ù.
  config.sensor[B2_SUNSHINE_DURATION].type = S_T_ADC;
  sensor_add(&config.sensor[B2_SUNSHINE_DURATION]);
  adc_config = get_sensor_config(&config.sensor[B2_SUNSHINE_DURATION]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 5000;
  adc_config->lowScale = 0;
  adc_config->scale = 1000;
  adc_config->outMaxV = 5000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 5cm
  config.sensor[B5_SOIL_TEMPERATURE_5CM].type = S_T_ADC;
  sensor_add(&config.sensor[B5_SOIL_TEMPERATURE_5CM]);
  adc_config = get_sensor_config(&config.sensor[B5_SOIL_TEMPERATURE_5CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 10cm
  config.sensor[B6_SOIL_TEMPERATURE_10CM].type = S_T_ADC;
  sensor_add(&config.sensor[B6_SOIL_TEMPERATURE_10CM]);
  adc_config = get_sensor_config(&config.sensor[B6_SOIL_TEMPERATURE_10CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 20cm
  config.sensor[B7_SOIL_TEMPERATURE_20CM].type = S_T_ADC;
  sensor_add(&config.sensor[B7_SOIL_TEMPERATURE_20CM]);
  adc_config = get_sensor_config(&config.sensor[B7_SOIL_TEMPERATURE_20CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 30cm
  config.sensor[B8_SOIL_TEMPERATURE_30CM].type = S_T_ADC;
  sensor_add(&config.sensor[B8_SOIL_TEMPERATURE_30CM]);
  adc_config = get_sensor_config(&config.sensor[B8_SOIL_TEMPERATURE_30CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 50cm
  config.sensor[B9_SOIL_TEMPERATURE_50CM].type = S_T_ADC;
  sensor_add(&config.sensor[B9_SOIL_TEMPERATURE_50CM]);
  adc_config = get_sensor_config(&config.sensor[B9_SOIL_TEMPERATURE_50CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 1m
  config.sensor[B10_SOIL_TEMPERATURE_100CM].type = S_T_ADC;
  sensor_add(&config.sensor[B10_SOIL_TEMPERATURE_100CM]);
  adc_config = get_sensor_config(&config.sensor[B10_SOIL_TEMPERATURE_100CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  // ÁöÁß¿Âµµ 1.5m
  config.sensor[B11_SOIL_TEMPERATURE_150CM].type = S_T_ADC;
  sensor_add(&config.sensor[B11_SOIL_TEMPERATURE_150CM]);
  adc_config = get_sensor_config(&config.sensor[B11_SOIL_TEMPERATURE_150CM]);
  adc_config->channel = single_channel++;
  adc_config->mode = eSINGLE_ADC;
  adc_config->highScale = 6000;
  adc_config->lowScale = -4000;
  adc_config->scale = 100;
  adc_config->outMaxV = 1000;
  adc_config->outMinV = 0;

  save_config_app();
  save_config_sensor();
}
int32_t menu_manage_config_sensor(p_shell_context_t ctx)
{
  int32_t cnt;
  const char *config_menu[] = {"0.¿ù°£ ¿ì·®", "1.¿¬°£ ¿ì·®","2.¿ù°£ ÀÏÁ¶","3.¿¬°£ ÀÏÁ¶"};


  while(1)
  {
    cnt = select_indexFromList(ctx, config_menu, NULL, _countof(config_menu), false);
    
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }

  if (cnt > 0)
  {
    cnt--;
    switch (cnt)
    {
      case 0:
      debug_printf("ÇöÀç ¿ù°£ ¿ì·®:%f\r\n",get_config_nvm()->rainfall_monthly);
      if (get_user_confirm("¿ù°£ ¿ì·®À» 0À¸·Î ¼³Á¤ÇÕ´Ï´Ù.")!=1)
      {
        continue;
      }
        
        nvm_set_rainfall_monthly(0.0f);
        break;
      case 1:
      debug_printf("ÇöÀç ¿¬°£ ¿ì·®:%f\r\n",get_config_nvm()->rainfall_yearly);
      if (get_user_confirm("¿¬°£ ¿ì·®À» 0À¸·Î ¼³Á¤ÇÕ´Ï´Ù.")!=1)
      {
        continue;
      }
      nvm_set_rainfall_yearly(0.0f);
        break;
      case 2:
      debug_printf("ÇöÀç ¿ù°£ ÀÏÁ¶:%f\r\n",get_config_nvm()->sunshine_monthly);
      if (get_user_confirm("¿ù°£ ÀÏÁ¶À» 0À¸·Î ¼³Á¤ÇÕ´Ï´Ù.")!=1)
      {
        continue;
      }
      nvm_set_sunshine_monthly(0.0f);
      break;
      case 3:
      debug_printf("ÇöÀç ¿¬°£ ÀÏÁ¶:%f\r\n",get_config_nvm()->sunshine_yearly);
      if (get_user_confirm("¿¬°£ ÀÏÁ¶À» 0À¸·Î ¼³Á¤ÇÕ´Ï´Ù.")!=1)
      {
        continue;
      }
      nvm_set_sunshine_yearly(0.0f);
      break;

    }
  }
  }

  return 0;
}
int32_t menu_manage_config_backup(p_shell_context_t ctx)
{
  int32_t cnt;
  const char *config_menu[] = {"0.¼³Á¤°ª ¹é¾÷", "1.¼³Á¤°ª º¹±¸"};


  while(1)
  {
    cnt = select_indexFromList(ctx, config_menu, NULL, _countof(config_menu), false);
    
    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }

    if (cnt > 0)
    {
      cnt--;
      switch (cnt)
      {
        case 0:
        backup_config();
        break;
        case 1:
        if(get_user_confirm("SDÄ«µå¿¡¼­ ¼³Á¤°ªÀ» ºÒ·¯¿É´Ï´Ù.")==1);
        {
          restore_config();
        }
        break;
      }
    }
  }

  return 0;
}
// ÃÊ±âÈ­
int32_t menu_manage_config_reset(p_shell_context_t ctx)
{
  int32_t cnt;
  const char *config_menu[] = {"0.AWS È­Áø ±âº» ¼³Á¤", "1.°øÀå ÃÊ±âÈ­","2.¼¾¼­","3.¹é¾÷"};

  while(1)
  {
    cnt = select_indexFromList(ctx, config_menu, NULL, _countof(config_menu), false);

    if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }


    if (cnt > 0)
    {
      cnt--;
      switch (cnt)
      {
        case 0:
          if(get_user_confirm("¼¾¼­ ¼³Á¤°ªÀ» È­Áø »çÀü ¼³Á¤°ªÀ¸·Î º¯°æÇÕ´Ï´Ù.")==1)
          {
            config_hj_reset();
            debug_printf("ÃÊ±âÈ­ µÇ¾ú½À´Ï´Ù.\r\n");
          }
          break;
        case 1:
        if(get_user_confirm("¼³Á¤°ªÀ» °øÀåÃÊ±âÈ­ÇÕ´Ï´Ù.")==1)
        {
          config_app_reset();
          save_config_app();
          config_sensor_reset();
          save_config_sensor();
          debug_printf("°øÀå ÃÊ±âÈ­ µÇ¾ú½À´Ï´Ù.\r\n");
        }
          break;
          case 2:
          menu_manage_config_sensor(ctx);
          break;
          case 3:
            menu_manage_config_backup(ctx);
             break;
      }
    }
  }

  return 0;
}

int32_t menu_manage_print_config_all(p_shell_context_t ctx)
{
  ctx->printf("ID               :%d\r\n", config.id);
  ctx->printf("ºñ¹Ð¹øÈ£         :%d\r\n", config.password);
  ctx->printf("ÃæÀü±â Á¾·ù      :%s\r\n", ITEM_LIST(config.charger_model, g_chgList));
  ctx->printf("·Î±× Ä«¿îÆ®      :%d\r\n", get_config_nvm()->logCnt);
  ctx->printf("ÇÁ·ÎÅäÄÝ          :%s\r\n", ITEM_LIST(config.aws_protocol_type, protocolList));
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

  ctx->printf("CDMA ¿ø°Ý ¼­¹ö   :%d.%d.%d.%d\r\n", config.cdma_server_ip[0],
              config.cdma_server_ip[1], config.cdma_server_ip[2], config.cdma_server_ip[3]);
  ctx->printf("CDMA Æ÷Æ®        :%d\r\n", config.cdma_port);

  ctx->printf("CDMA Á¾·ù        :%s\r\n", ITEM_LIST(config.cdma_model, cdmaModellList));
  ctx->printf("ÀÌ´õ³Ý »ç¿ë      :%s\r\n", ITEM_LIST((int32_t)config.eth_use, enableList));
  ctx->printf("CDMA »ç¿ë        :%s\r\n", ITEM_LIST((int32_t)config.cdma_use, enableList));
  ctx->printf("Á÷Á¢Åë½Å         :%s\r\n", ITEM_LIST((int32_t)config.direct_use, enableList));

  ctx->printf("Á÷Á¢Åë½Å ¼Óµµ    :%d\r\n", config.direct_baud);
  ctx->printf("ÆÐ³Î Á¾·ù        :%s\r\n", ITEM_LIST(config.panel_model, panelList));
  ctx->printf("VHF ID           :%d\r\n", config.vhf_id);
  ctx->printf("VHF ±×·ì         :%d\r\n", config.vhf_group);
  ctx->printf("VHF HOST         :%d\r\n", config.vhf_host_id);
  ctx->printf("VHF Áß°è         :%d\r\n", config.vhf_repeater_id);
  ctx->printf("VHF PTT Áö¿¬     :%d\r\n", config.vhf_ptt_delay);

  return 0;
}

int32_t menu_manage_update_fw(p_shell_context_t ctx)
{
  if (get_user_confirm("Æß¿þ¾î ¾÷µ¥ÀÌÆ®¸¦ ÁøÇàÇÒ±î¿ä?") == 1)
  {
    if(check_firmware(UPDATE_LOCAL) ==0)
    {
      debug_printf("Àåºñ°¡ ¸®¼ÂµÇ¸é¼­ ¾÷µ¥ÀÌÆ®°¡ ÁøÇàµË´Ï´Ù.\r\n");
      debug_printf("»óÅÂ LED°¡ Á¡¸êµË´Ï´Ù.\r\n");
      
      set_magic_value(MAGIC_UPDATE_FW_LACAL);
      reset_system(0, "USER update");
    }
  }
}
menu_func g_manageMenu[] = {[0] = menu_manage_version,
                            menu_manage_device_reset,
                            menu_manage_config_reset,
                            menu_manage_update_fw};

int32_t print_menu_manage(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  debug_printf("\r\n");
  debug_printf("%d.¹öÀü\r\n", cnt++);
  debug_printf("%d.Àåºñ ¸®¼Â\r\n", cnt++);
  debug_printf("%d.¼³Á¤ °ª\r\n", cnt++);
  debug_printf("%d.Æß¿þ¾î ¾÷µ¥ÀÌÆ®\r\n", cnt++);
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
      if (cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt <= 0)
      {
        return cnt;
      }
    } while (1);
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
          ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12s %-10s \r\n", "Diff", i + 1, off,
                      full, off_in, full_in, "error", " ");
          ;
        }
        else
        {
          ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12d %-8.6f \r\n", "Diff", i + 1, off,
                      full, off_in, full_in, adc, voltage / 1000.0);
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
  run_calibraion_root();
  
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
    ctx->printf("%13s:%d",sensor_name_list[i], config.sensor[i].configCnt);
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
        debug_printf("%4d,%s\r\n", i,log.msg);
      }
    }
  } while (1);
}


int32_t print_menu_developer(p_shell_context_t ctx)
{
  int32_t cnt = 0;

  ctx->printf("\r\n");
  ctx->printf("%2d.ÀÎÅÍ·´Æ® ¼³Á¤ È®ÀÎ\r\n", cnt++);
  ctx->printf("%2d.¸Þ¸ð¸® Å×½ºÆ®\r\n", cnt++);
  ctx->printf("%2d.¼¾¼­ config ÀüºÎ È®ÀÎ\r\n", cnt++);
  ctx->printf("%2d.½Ã½ºÅÛ ·Î±× È®ÀÎ\r\n", cnt++);
  ctx->printf("%2d.Å×½ºÅ© Á¤º¸\r\n", cnt++);
  ctx->printf("%2d.ÆÄÀÏ´Ù¿î »óÅÂ Á¤º¸ \r\n", cnt++);
  return cnt;
}

int32_t menu_task_info(p_shell_context_t ctx)
{
  print_task_info();
}


int32_t menu_update_info(p_shell_context_t ctc)
{
  float progress=0.0f;
  uint32_t total_bytes;
  uint32_t received_bytes;


  while(1)
  {
    total_bytes = get_download_file_size();
    received_bytes = get_received_bytes();
    if(total_bytes !=0)
    {
      progress = ((float)received_bytes/(float)total_bytes)*100.0;
    }
    debug_printf("Æß¿þ¾î ´Ù¿î:%7d/%7d [%5.2f%%]\r",received_bytes,total_bytes,progress);

    if (get_key(1000) == KEY_CODE_CTRL_Q)
    {
      break;
    }
  }

  return 0;
}
int32_t menu_developer(p_shell_context_t ctx)
{
  int32_t cnt;
  const menu_func menu[] = {
      [0] = menu_developer_interrupt, menu_developer_memory, menu_developer_sensor_config,
      menu_developer_logging,         menu_task_info,        menu_update_info};
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



const menuFunc_t menuFunc[] = {{.title = "0.diplay", .func = aws_menu_display},
                               {.title = "1.system", .func = menu_system},
                               {.title = "2.sensor", .func = menu_sensor},
                               {.title = "3.offset", .func = menu_offset},
                               {.title = "4.network", .func = menu_network},
                               {.title = "5.data", .func = menu_data},
                               {.title = "6.display panel", .func = aws_menu_display_panel},
                               {.title = "7.manage", .func = menu_manage},
                               {.title = "8.calibraion", .func = menu_calibration},
                               {.title = "9.developer", .func = menu_developer}};

int32_t print_menu_root(p_shell_context_t ctx)
{
  int i;
  for (i = 0; i < _countof(menuFunc); i++)
  {
    ctx->printf("%s\r\n", menuFunc[i].title);
  }
  return i;
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