
#include <stdbool.h>
#include <stdio.h>
#include <math.h>


#include "aws_data.h"
#include "app_console.h"
#include "app_version.h"
#include "app_sensor.h"
#include "app_rs232.h"
#include "app_rs485.h"
#include "app_adc.h"
#include "app_flash.h"
#include "boot_version.h"
#include "cmsis_os.h"
#include "config.h"
#include "dev_io.h"
#include "mcu_debug.h"
#include "system_err.h"
#include "utile_time.h"
#include "utile.h"
#include "vt100_command.h"
#include "ymodem.h"
#include "terminal.h"


#define EXIT_PROGRAM -3
#define EXIT_BACK    -1

#define ITEM_LIST(cnt,list) cnt>=_countof(list)?g_unknown:(char *)list[cnt] 
//»ç¿ë°¡´ÉÇÑ ½Ì±Û Ã¤³Î ¼³Á¤Á¤
const bool single_en[32]={1,1,1,0,
                          1,1,1,0,
                          1,1,0,0,
                          1,1,0,0,
                          1,1,0,0,
                          1,1,0,0,
                          1,1,0,0,
                          1,1,0,0};
      

typedef   int32_t (*menu_func)(p_shell_context_t);

typedef struct menuFunc_s
{
  const char *title;
  menu_func func;
}menuFunc_t;

typedef enum val_e
{
  eUINT8,
  eUINT16,
  eUINT32,
  eFLOAT
}eVAL_TYPE_t;

typedef struct select_menu_s
{
  p_shell_context_t ctx;
  const char **list;
  int32_t (*func)(p_shell_context_t ctx);
  uint8_t cnt;
  bool show;
  const menuFunc_t *menuFunc;
}select_menu_t;






const char *protocolList[]={"kma ver 1","kma ver 2"};
const char *cdmaModellList[]={"TX700","NTLE9607"};
const char *panelList[]={"model a","model b"};
const char *doorStatusList[]={"´ÝÈû","¿­¸²"};
const char *linkStatusList[]={"up","down"};
const char *g_chgList[]={"smart charger",
                         "aws charger"};
const char *g_unknown="unknown";
const char *generalStatusList[]={"Á¤»ó","ºñÁ¤»ó"};
     const char *unusedList[]={"¹Ì»ç¿ë"};
const char *adcChModeList[]={"single","diff"};
const char *rs232ParityList[]={"none","even","odd"};
const char *enableList[]={"¹Ì»ç¿ë","»ç¿ë"};


int32_t print_common_cfg(p_shell_context_t ctx,sensor_t *sensor,uint8_t c);

void make_comList(char *out,uint16_t outsize)
{
  int32_t len = 0 ;
  if(config.eth_use)
  {
    len = snprintf(&out[len],outsize-len,"[ETH]");
  }
  if(config.cdma_use)
  {
    len += snprintf(&out[len],outsize-len,"[CDMA]");
  }
    if(config.direct_use)
  {
    len += snprintf(&out[len],outsize-len,"[DIRECT]");
  }

  if(len==0)
  {
snprintf(&out[len],outsize-len,"¹Ì»ç¿ë");
  }
}

void update_val(void *val,void *target,eVAL_TYPE_t type)
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

void print_items(char *title,char *items[],uint8_t itmeCnt)
{
  debug_printf("\r\n");
  debug_printf("(0lqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqk(B\r\n");
  debug_printf("(0x(B %s(0x(B\r\n",title);
  debug_printf("(0tqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqu(B\r\n");
  for(int i = 0 ; i< itmeCnt;i++)
  {
    debug_printf("(0x(B %s(0x(B\r\n",items[i]);
  }
  debug_printf("(0mqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqj(B\r\n");
}

int32_t select_indexFromList(p_shell_context_t ctx,const char *list[],
                            int32_t (*func)(p_shell_context_t),uint16_t listCnt,bool number)
{
  int cnt;
  int index=0;
  int funcCnt=0;
  int indexMax;
  do
  {
    if(list)
    {
      //¸ñ·ÏÀ» Ãâ·ÂÇÑ´Ù.
      for(int i = 0 ; i< listCnt; i++)
      {
        if(number == true)
        {
          ctx->printf("%d.%s\r\n",i,list[i]);
        }
        else
        {
          ctx->printf("%s\r\n",list[i]);
        }
      }
      indexMax = listCnt;
    }
    if(func)
    {
      funcCnt = func(ctx);
      indexMax = funcCnt;
    }

    if(func == NULL && list ==NULL)
    {
      indexMax = listCnt;
    }

    vt100_printfColor(GREEN,"¹øÈ£¸¦ ¼±ÅÃÇØ ÁÖ¼¼¿ä:");
    //»ç¿ëÀÚ·Î ºÎÅÍ ¸Þ´º¸¦ ¼±ÅÃ ¹Þ´Â´Ù.
    cnt = console_scanf("%d",&index);//-1 ctrl+c, 0 enter esc
    ctx->printf("\r\n");
    if(cnt==1)
    {
      if(index < indexMax)
      break;
    }
    else if(cnt == EXIT_BACK)
    {
      return EXIT_BACK;
    }
    else if(cnt == EXIT_PROGRAM)
    {
      return EXIT_PROGRAM;
    }
  //ctx->printf("Invalid input. Please select a menu number again.\r\n");
  vt100_printfColor(RED,"À¯È¿ÇÑ ¹øÈ£°¡ ¾Æ´Õ´Ï´Ù\r\n");
  }while(1);

  return  (index+1);
}


int32_t select_indexMenu(p_shell_context_t ctx,sensor_t *sensor)
{
  int cnt;
  int index=0;
  int funcCnt=0;
  int indexMax;

  do
  {
    funcCnt = print_common_cfg(ctx,sensor,0);
    indexMax = funcCnt;
 

    vt100_printfColor(GREEN,"¹øÈ£¸¦ ¼±ÅÃÇØ ÁÖ¼¼¿ä:");
    //»ç¿ëÀÚ·Î ºÎÅÍ ¸Þ´º¸¦ ¼±ÅÃ ¹Þ´Â´Ù.
    cnt = console_scanf("%d",&index);//-1 ctrl+c, 0 enter esc
    ctx->printf("\r\n");
    if(cnt==1)
    {
      if(index < indexMax)
      break;
    }
    else if(cnt == EXIT_BACK)
    {
      return EXIT_BACK;
    }
    else if(cnt == EXIT_PROGRAM)
    {
      return EXIT_PROGRAM;
    }
  //ctx->printf("Invalid input. Please select a menu number again.\r\n");
  vt100_printfColor(RED,"À¯È¿ÇÑ ¹øÈ£°¡ ¾Æ´Õ´Ï´Ù\r\n");
  }while(1);

  return  (index+1);
}












bool wait_break(uint32_t timeoutms)
{
  int32_t ch;
  osDelay(timeoutms);
  ch = DbgConsole_GetcharNonBlocking();
  if(ch ==-1)
  {
    return true;
  }

return false;
}


#define DISP_WIDTH    25


  extern   uint32_t elased_time;
int32_t print_systemInfo(uint16_t row,uint16_t column)
{

  uint8_t line=row+3;
  char buff[30];

  snprintf(buff,sizeof(buff),"%04d-%02d-%02d %02d:%02d:%02d\r\n",Date_Time.Year,Date_Time.Month,
  Date_Time.Day,Date_Time.Hour,Date_Time.Min,Date_Time.Sec);


  vt100_print_frame(row   ,column,"½Ã½ºÅÛ", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"%s\r\n",buff);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¹® »óÅÂ  :%s\r\n",ITEM_LIST(System.doorStatus,doorStatusList));

  vt100_print_line(line++,column,'+', '-', DISP_WIDTH);

    
  return 4+2;
}

int32_t print_chargerInfo(uint16_t row,uint16_t column)
{

  uint8_t line=row+3;

  vt100_print_frame(row   ,column,"ÃæÀü±â", '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"»óÅÂ           :%s\r\n",ITEM_LIST(System.charger_status,generalStatusList));
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÃæÀü Àü¾Ð(v)   :%d\r\n",0);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¹èÅÍ¸® Àü¾Ð(v) :%d\r\n",0);
  vt100_print_line(line++,column,'+', '-', DISP_WIDTH);
    
  return 4+2;
}
int32_t print_ethInfo(uint16_t row,uint16_t column)
{
    char buff[30];

    uint8_t line=row+3;

    make_comList(buff,sizeof(buff));
    vt100_print_frame(row   ,column,"ÀÌ´õ³Ý", '+', '|', '-', DISP_WIDTH, WHITE);
    vt100_print_bar(line++ ,column,-DISP_WIDTH,"¸µÅ©  :%s\r\n",ITEM_LIST(System.eth_link_status,linkStatusList));
    vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼Û½Å  :%d\r\n",System.eth_tx_cnt);
    vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼ö½Å  :%d\r\n",System.eth_rx_cnt);
    vt100_print_line(line++,column,'+', '-', DISP_WIDTH);
    
    return 5+2;
}




int32_t print_awsRealLefinfo(uint16_t row,uint16_t column,uint8_t mode,void* arg)
{
  char buff[50];

  uint8_t line=row+3;
  //sensor_t *sensor;
  sensor_data_t *pdata;
  const char *aswTitleList[]={"½Ç½Ã°£(1s)","1ºÐ","10ºÐ","ÇÑ½Ã°£"};

  snprintf(buff,sizeof(buff),"AWS %s %.2fms",aswTitleList[mode],(float)elased_time/1000.0f);
  
  pdata = sensor_data;

#define P_WIDTH 27
#if 1
  vt100_print_frame(row   ,column,buff, '+', '|', '-', P_WIDTH, WHITE);

  for(int i = 0 ;i < _countof(config.sensor);i++)
  {
    if(config.sensor[i].type)//¼¾¼­°¡ »ç¿ëÁß
    {
      snprintf(buff,sizeof(buff),"%-13s:%s\r\n",sensorNameList[i],dataFmtList[i]);
      if(pdata[i].dataType == DATA_TYPE_I)
      {
        vt100_print_bar(line++ ,column,-P_WIDTH,buff,pdata[i].data.i );
      }
      else
      {
        vt100_print_bar(line++ ,column,-P_WIDTH,buff,pdata[i].data.f );
      }
    }
  } 


  vt100_print_line(line++,column,'+', '-', P_WIDTH);

  #endif

#if 0 
  vt100_print_frame(row   ,column,buff, '+', '|', '-', DISP_WIDTH, WHITE);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"±â¿Â          :%5.2f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Ç³Çâ          :%f\r\n",pdata[A2_WIND_DIRECTION].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Ç³¼Ó          :%f\r\n",pdata[A3_WIND_SPEED].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼ø°£ Ç³Çâ     :%f\r\n",pdata[A4_INSTANT_WIND_DIRECTION].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼ø°£ Ç³¼Ó     :%f\r\n",pdata[A5_INSTANT_WIND_SPEED].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"°­¼ö·®        :%f\r\n",pdata[A6_RAINFALL_DOT5_1MM].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"±â¾Ð          :%f\r\n",pdata[A7_PRESSURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"°­¼ö À¯¹«     :%f\r\n",pdata[A8_RAIN_PRESENT].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Àû¼³          :%f\r\n",pdata[A9_SNOW_DEPTH].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"»ó´ë½Àµµ      :%\r\n",pdata[A10_RELATIVE_HUMIDITY].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"°­¼ö·®        :%fd\r\n",pdata[A11_RAINFALL_DOT1MM].data.f);

  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÀÏ»ç          :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÀÏÁ¶          :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Áö¸é¿Âµµ      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÃÊ»ó¿Âµµ      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 5cm  :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 10cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 20cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 30cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 50cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 1m   :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 1.5m :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 3m   :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÁöÁß¿Âµµ 5cm  :%f\r\n",pdata[A1_TEMPERATURE].data.f);

  vt100_print_bar(line++ ,column,-DISP_WIDTH,"1Ãþ ¿î°í      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"2Ãþ ¿î°í      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"3Ãþ ¿î°í      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¿î·®          :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"½ÃÁ¤          :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"PM10          :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"PM2.5         :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼øº¹»ç        :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÀüÃµº¹»ç      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¹Ý»çº¹»ç      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Á÷´ÞÀÏ»ç      :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"ÇöÀçÀÏ±â      :%f\r\n",pdata[A1_TEMPERATURE].data.f);

  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Åä¾ç¼öºÐ 10cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Åä¾ç¼öºÐ 20cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Åä¾ç¼öºÐ 30cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Åä¾ç¼öºÐ 50cm :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Á¶µµ·®        :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Ç³¼Ó 1.5m     :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"Ç³¼Ó 4m       :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼ø°£Ç³¼Ó 1.5m :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"¼ø°£Ç³¼Ó 4m   :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"±â¿Â 0.5m     :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"±â¿Â 4m       :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"½Àµµ 0.5m     :%f\r\n",pdata[A1_TEMPERATURE].data.f);
  vt100_print_bar(line++ ,column,-DISP_WIDTH,"½Àµµ 4m       :%f\r\n",pdata[A1_TEMPERATURE].data.f);

  vt100_print_line(line++,column,'+', '-', DISP_WIDTH);
#endif
    return 5+2;
}

#define KEY_UP    0x41
#define KEY_DOWN  0x42
#define KEY_LEFT  0x44
#define KEY_RIGHT 0x43
#define KEY_ENTER 0x0D

#define AWS_MODE_MAX 3
int32_t menu_display(p_shell_context_t ctx)
{

  char ch;

  uint8_t awsMode=0;

  debug_printf(VT100_CLEAR_SCREEN);
  debug_printf(VT100_CURSOR_OFF);


  do
  {
    debug_printf(VT100_CURSOR_HOME);
    debug_printf("\r\n");
    print_systemInfo(1,0);
    print_ethInfo(1,30);
    print_chargerInfo(20,0);
    print_awsRealLefinfo(1,60,awsMode,NULL);

      ch=0;
     debug_recv(&ch,1,1000);


    if(ch == KEY_RIGHT)
    {
      if(awsMode<AWS_MODE_MAX)
      {
        awsMode++;
      }
    }
    else if(ch == KEY_LEFT)
    {
      if(awsMode>0)
      {
        awsMode--;
      }
    }
    
        if(ch ==ASCII_CODE_CTRL_Q)
    {
      break;
    }
  } while (1);
  vt100_print(50,0,"\r\n");
  debug_printf(VT100_CURSOR_ON);
return 0;
}








int32_t input_date(p_shell_context_t ctx,DATE_TIME_BUF *nt)
{
  int year;
  int month;
  int day;
  int hour;
  int min;
  int sec;
  int cnt;

  ctx->printf("format:YYYY-MM-DD hh:mm:ss,2020-01-01 00:11:22\r\n");

  cnt = console_scanf("%04d-%02d-%02d %02d:%02d:%02d",&year,&month,&day,&hour,&min,&sec);

  if(cnt==6)
  {
    return 6;
  }
  
  return cnt;
}

int input_decimal(p_shell_context_t ctx,int32_t start,int32_t stop,int32_t *dec)
{
  int32_t cnt;

  ctx->printf("¹üÀ§:%d~%d\r\n",start,stop);
  vt100_printfColor(GREEN,"°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%d",dec);
  if(cnt==1)
  {
    if(*dec >=start && *dec<= stop)
    {
      return 1;
    }
    else
    {
      vt100_printfColor(RED,"ÀÔ·Â°ªÀÇ ¹üÀ§¸¦ È®ÀÎÇØ ÁÖ¼¼¿ä\r\n");
      return 0;
    }
  }

  return cnt;
}


int32_t input_use(p_shell_context_t ctx,bool *en)
{
  int32_t cnt;
  int32_t dec;
  ctx->printf("0:¹Ì»ç¿ë\r\n");
  ctx->printf("1:»ç¿ë\r\n");
  vt100_printfColor(GREEN,"¹øÈ£¸¦ ¼±ÅÃÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%d",&dec);
  if(cnt==1)
  {
    if(dec >=0 && dec<= 1)
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


int input_digit(p_shell_context_t ctx,int32_t start,int32_t stop,void *target,eVAL_TYPE_t type)
{
  int32_t cnt;
  int8_t i8val;
  int16_t i16val;
  int32_t i32val;

  ctx->printf("\r\n¹üÀ§:%d~%d\r\n",start,stop);
  vt100_printfColor(GREEN,"°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  cnt = console_scanf("%d",&i32val);
  if(cnt==1)
  {
    if(i32val<start || i32val>stop)
    {
      vt100_printfColor(RED,"ÀÔ·Â°ªÀ» ¹üÀ§¸¦ È®ÀÎÇØ ÁÖ¼¼¿ä\r\n");
      return 0;
    }
    switch(type)
    {
      case eUINT8:
      i8val = i32val;
      update_val(&i8val,target,type);
      break;
      case eUINT16:
      i16val = i32val;
      update_val(&i16val,target,type);
      break;
      case eUINT32:
      update_val(&i32val,target,type);
      break;
    }

    return 1;
  }

  return cnt;
}


int32_t print_menu_system(p_shell_context_t ctx)
{
  char buff[50];
  int cnt=0;

  make_timeToStr(&Date_Time,buff,sizeof(buff));
  ctx->printf("%2d.time        :%s\r\n",cnt++,buff);
  ctx->printf("%2d.id          :%d\r\n",cnt++,config.id);
  ctx->printf("%2d.password    :%d\r\n",cnt++,config.password);
  ctx->printf("%2d:charger type:%s\r\n",cnt++,ITEM_LIST(config.chgType,g_chgList));

  return cnt;
}

int32_t menu_system(p_shell_context_t ctx)
{
  int cnt;
  int32_t dec;
  DATE_TIME_BUF nt;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_system,0,false);
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    switch(cnt)
    {
      case 0:
        cnt = input_date(ctx,&nt);
        if(cnt>0)
        {
          
        }
      break;
      case 1://id
      cnt = input_decimal(ctx,0,9999,&dec);
      if(cnt)
      {
        config.id = dec;
        WRITE_CFG(id);
      }
      break;
       case 2://password
      cnt = input_decimal(ctx,0,9999,&dec);
      if(cnt)
      {
          config.password = dec;
          WRITE_CFG(password);
      }
      break;
      case 3://charger type
      cnt = select_indexFromList(ctx,g_chgList,NULL,sizeof(g_chgList)/sizeof(g_chgList[0]),true);
      if(cnt>0)
      {
        cnt--;
        config.chgType = cnt;
        WRITE_CFG(chgType);
      }
      break;
    }
  } while (1);
  

}


void make_option(sensor_t *sensor,char *out,uint16_t outSize)
{
  void *cfg;
  const char *list[10];

  out[0]=0;

  cfg = get_sensor_config(sensor);

  if(cfg==NULL)
  {
    return; 
  }

  switch(sensor->type)
  {
    case S_T_SNOW_HJ_232:
    case S_T_TEMP_232:
    case S_T_GENERAL_232:
    {
    rs232_config_t *rs232_cfg = (rs232_config_t*)cfg;
    rs232_get_portList(list,sizeof(list));
    snprintf(out,outSize,"[%s]",list[rs232_cfg->port]);
    }
    break;
      case S_T_HUMI_HJ_485:
    case S_T_WIND_DIRECTION_485:
    case S_T_WIND_SPEED_485:
    case S_T_TEMP_485:
    case S_T_GENERAL_485:
    case S_T_SNOW_HJ_485:

    {
    rs485_config_t *rs485_cfg = (rs485_config_t*)cfg;
    rs485_get_portList(list,sizeof(list));
    snprintf(out,outSize,"[%s.%d]",list[rs485_cfg->port],rs485_cfg->id);
    }
    break;
    case S_T_ADC:
      {
    adc_config_t *adc_cfg = (adc_config_t *)cfg;
    snprintf(out,outSize,"[%s.%d]",adcChModeList[adc_cfg->mode],adc_cfg->channel);
      }
    break;
    default:
    out[0]=0;
    break;
  }
}
//¼¾¼­ Ãâ·Â
int32_t print_menu_sensor(p_shell_context_t ctx)
{
  char opt[20];
  int32_t cnt=0;
  int i=0;

  ctx->printf("\r\n");

#if 1
  cnt = _countof(sensorNameList)/2;

  for( i = 0 ; i< cnt;i++)
  {
    make_option(&config.sensor[i],opt,sizeof(opt));
    ctx->printf("%2d.%-15s:%-20s %-15s,  ",i,sensorNameList[i]  ,ITEM_LIST(config.sensor[i].type,sensorTypeList),opt);
    make_option(&config.sensor[i+cnt],opt,sizeof(opt));
    ctx->printf("%2d.%-15s:%-20s %-15s\r\n",i+cnt,sensorNameList[i+cnt],ITEM_LIST(config.sensor[i+cnt].type,sensorTypeList),opt);
  }

#endif
  cnt =  _countof(sensorNameList);
  return cnt;
}

uint8_t print_rs232_cfg(p_shell_context_t ctx,rs232_config_t *rs232_config,uint8_t cnt)
{
  const char *portNameList[10];

  rs232_get_portList(portNameList,_countof(portNameList));
  ctx->printf("%2d.port       :%s\r\n",cnt++,portNameList[rs232_config->port]);    
  ctx->printf("%2d.baud       :%d\r\n",cnt++,rs232_config->baud);
  ctx->printf("%2d:paraity    :%s\r\n",cnt++,ITEM_LIST(rs232_config->parityIdx,rs232ParityList));
  return cnt;
}

uint8_t print_rs485_cfg(p_shell_context_t ctx,rs485_config_t *rs485_config,uint8_t cnt)
{
  const char *portNameList[10];

  rs485_get_portList(portNameList,_countof(portNameList));
  ctx->printf("%2d.id         :%d\r\n",cnt++,rs485_config->id);    
  ctx->printf("%2d.port       :%s\r\n",cnt++,portNameList[rs485_config->port]);    
  ctx->printf("%2d.baud       :%d\r\n",cnt++,rs485_config->baud);
  ctx->printf("%2d:paraity    :%s\r\n",cnt++,ITEM_LIST(rs485_config->parityIdx,rs232ParityList));
  return cnt;
}

uint8_t print_adc_cfg(p_shell_context_t ctx,adc_config_t *adc_config,uint8_t cnt)
{
  ctx->printf("%2d.adc mode   :%s\r\n",cnt++,ITEM_LIST(adc_config->mode,adcChModeList));    
  ctx->printf("%2d.channel    :%d\r\n",cnt++,adc_config->channel);
  ctx->printf("%2d.high scale :%d\r\n",cnt++,adc_config->highScale);
  ctx->printf("%2d.low scale  :%d\r\n",cnt++,adc_config->lowScale);
  return cnt;
}




int32_t select_item(p_shell_context_t ctx,const char *list[],int listCnt,void *target,eVAL_TYPE_t type)
{
  int cnt;

  cnt = select_indexFromList(ctx,list,NULL,listCnt,true);
  if(cnt>0)
  {
    cnt--;
    update_val(&cnt,target,type);
    return 0;
  }

  return cnt;
  
}

/**
 * @brief index·Î ÀúÀåµÈ ¼¾¼­ ¸ñ·ÏÀ» ¹®ÀÚ¿­ ¸ñ·ÏÀ¸·Î °¡Á®¿À±â
 */
uint16_t gen_sensorItemList(const char **itemListOut,const uint8_t *idxList,uint8_t listCnt)
{
  int  i;

  for(  i = 0; i< listCnt;i++)
  {
    itemListOut[i] = sensorTypeList[idxList[i]];
  }

  return i;
}

#define ADC_SET_CH_MODE   0
#define ADC_SET_CHANNLEL  1
#define ADC_SET_HIGHSCALE 2
#define ADC_SET_LOWSCALE  3

void adc_config_set(p_shell_context_t ctx, sensor_t *sensor, uint8_t cnt)
{
  int32_t dec;
  adc_config_t *adc;

  adc = get_sensor_config(sensor);
  switch(cnt)
  {
    case ADC_SET_CH_MODE://1.Ã¤³Î ¸ðµå
    cnt = select_indexFromList(ctx,adcChModeList,NULL,_countof(adcChModeList),true);
    if(cnt>0)
    {
      adc->mode = (cnt-1);
      write_s_config();
    }
    break;
    case ADC_SET_CHANNLEL://channel;
    cnt = input_decimal(ctx,1,18,&dec);
    if(cnt)
    {
      adc->channel = dec;
      write_s_config();
    }
    break;
    case ADC_SET_HIGHSCALE://hish cale;
    cnt = input_decimal(ctx,-100000,100000,&dec);
    if(cnt)
    {
      adc->highScale = dec;
      write_s_config();
     }
      break;
    case ADC_SET_LOWSCALE://low cale;
      cnt = input_decimal(ctx,-100000,100000,&dec);
      if(cnt)
      {
        adc->lowScale = dec;
        write_s_config();
      }
    break;
  }
}

//232¼³Á¤

#define RS232_SET_PORT   0
#define RS232_SET_BAUD   1
#define RS232_SET_PARITY 2

void rs232_config_set(p_shell_context_t ctx, sensor_t *sensor,uint8_t cnt)
{
  int32_t dec;
  rs232_config_t *rs232;
   const char *portList[10];
  rs232= get_sensor_config(sensor);
  if(rs232==0)
  {
    debug_printf("rs232 err\r\n");
  }
  switch (cnt)
  {
  case RS232_SET_PORT:

    cnt = rs232_get_portList(portList,_countof(portList));

    cnt = select_indexFromList(ctx,portList,NULL,cnt,true);
    if(cnt)
    {
      if(rs232_is_opened((eRS232_PORT_t)rs232->port))//¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
      {
        ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
        ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
        rs232_close((eRS232_PORT_t)rs232->port);
      }

      rs232->port = cnt-1;
      if(rs232_is_opened((eRS232_PORT_t)rs232->port))//¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
      {
        ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n",cnt-1);
      }
      else
      { 
        uart_config_t uart_cfg;
        uart_cfg.baud = rs232->baud;
        uart_cfg.parityIdx = rs232->parityIdx;

        rs232_open((eRS232_PORT_t)rs232->port,&uart_cfg);
        ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n",portList[rs232->port]);
      }

      write_s_config();
    }
    break;

  case RS232_SET_BAUD:
    cnt = input_decimal(ctx,9600,115200,&dec);
    if(cnt)
    {
      rs232->baud = dec;
      write_s_config();
    }

  break;
  case RS232_SET_PARITY:
        cnt = select_indexFromList(ctx,rs232ParityList,NULL,_countof(rs232ParityList),true);
        {
          rs232->parityIdx = cnt-1;
          write_s_config();
        }
  break;
  default:
    break;
  }
}

#define RS485_SET_ID     0
#define RS485_SET_PORT   1
#define RS485_SET_BAUD   2
#define RS485_SET_PARITY 3

void rs485_config_set(p_shell_context_t ctx, sensor_t *sensor,uint8_t cnt)
{
  int32_t dec;
  rs485_config_t *rs485;
   const char *portList[10];
   
  rs485= get_sensor_config(sensor);
  if(rs485 == NULL)
  {
    return;
  }
  switch (cnt)
  {
    case RS485_SET_ID:
     cnt = input_decimal(ctx,0,255,&dec);
    if(cnt>0)
    {
      rs485->id = dec;
      write_s_config();
    }
    break;
  case RS485_SET_PORT:

    cnt = rs485_get_portList(portList,_countof(portList));

    cnt = select_indexFromList(ctx,portList,NULL,cnt,true);
    if(cnt)
    {
      if(rs485_is_opened((eRS485_PORT_t)rs485->port))//¸¸¾à ÀÌ¹Ì ¿­¸° Æ÷Æ®ÀÎµ¥ º¯°æÇÏ·Á°í ÇÏ¸é ¿À·ù
      {
        ctx->printf("Æ÷Æ®°¡ ¿­¸° »óÅÂ¿¡¼­ º¯°æÀ» ½ÃµµÇÕ´Ï´Ù.\r\n");
        ctx->printf("¿­¸° Æ÷Æ®´Â ´ÝÈü´Ï´Ù\r\n");
        rs485_close((eRS485_PORT_t)rs485->port);
      }

      rs485->port = cnt-1;
      if(rs485_is_opened((eRS485_PORT_t)rs485->port))//¿­·Á°í ÇÏ´Â Æ÷Æ®°¡ ÀÌ¹Ì ¿­·ÁÀÖ´Ù¸é
      {
        ctx->printf("PORT:%d ÀÌ¹Ì ¿­·ÁÀÖ½À´Ï´Ù.Åë½Å¼Óµµ´Â º¯°æµÇÁö ¾Ê½À´Ï´Ù.\r\n",cnt-1);
      }
      else
      { 
        uart_config_t uart_config;
        uart_config.baud = rs485->baud;
        uart_config.parityIdx = rs485->parityIdx;
        uart_config.stop_bit = 0;
        rs485_open((eRS485_PORT_t)rs485->port,&uart_config);

        ctx->printf("%s »õ·Ó°Ô ¿­·È½À´Ï´Ù.\r\n",portList[rs485->port]);
      }

      write_s_config();
    }
    break;

  case RS485_SET_BAUD:
    cnt = input_decimal(ctx,9600,115200,&dec);
    if(cnt)
    {
      rs485->baud = dec;
      write_s_config();
    }

  break;
  case RS485_SET_PARITY:
        cnt = select_indexFromList(ctx,rs232ParityList,NULL,_countof(rs232ParityList),true);
        {
          rs485->parityIdx = cnt-1;
          write_s_config();
        }
  break;
  default:
    break;
  }
}

void rain_reed_config_set(p_shell_context_t ctx, sensor_t *sensor,uint8_t cnt)
{

}

void rain_hall_config_set(p_shell_context_t ctx, sensor_t *sensor,uint8_t cnt)
{

}



void rain_reed_config_set(p_shell_context_t ctx, sensor_t *sensor,uint8_t cnt);
void rain_hall_config_set(p_shell_context_t ctx, sensor_t *sensor,uint8_t cnt);





typedef struct
{
  uint8_t sensorType;
  void (*config_set)(p_shell_context_t , sensor_t *, uint8_t);
}config_sen_func_t;

/*
¼¾¼­ ¸ðµ¨°ú ¸ðµ¨ ¼³Á¤ ÇÔ¼ö ¿¬°á
*/
const config_sen_func_t sen_func[]=
            {{.sensorType = S_T_ADC,     .config_set  = adc_config_set},
            {.sensorType = S_T_TEMP_232, .config_set = rs232_config_set},
            {.sensorType = S_T_TEMP_485, .config_set = rs485_config_set},
            {.sensorType = S_T_RAIN_HALL_05MM,.config_set = rain_hall_config_set},
            {.sensorType = S_T_RAIN_HALL_1MM,.config_set  = rain_hall_config_set},
            {.sensorType = S_T_RAIN_REED_05MM,.config_set = rain_reed_config_set},
            {.sensorType = S_T_RAIN_REED_1MM,.config_set  = rain_reed_config_set},
            {.sensorType = S_T_GENERAL_232,.config_set = rs232_config_set},
            {.sensorType = S_T_WIND_DIRECTION_485,.config_set = rs485_config_set},
            {.sensorType = S_T_WIND_SPEED_485,.config_set = rs485_config_set},
            {.sensorType = S_T_WIND_SPEED_MAX_VAL,.config_set = 0},
            {.sensorType = S_T_WIND_DIRECTION_MAX_VAL,.config_set = 0},
            {.sensorType = S_T_SNOW_HJ_485,.config_set = rs485_config_set},
            {.sensorType = S_T_HUMI_HJ_485,.config_set= rs485_config_set},
            {.sensorType = S_T_SNOW_HJ_232,.config_set= rs232_config_set},
            {.sensorType = S_T_GENERAL_485,.config_set= rs485_config_set}};




int32_t print_common_cfg(p_shell_context_t ctx,sensor_t *sensor,uint8_t c)
{
  int32_t cnt=0;

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  
  switch(sensor->type)
  {
    case S_T_ADC://ADC
      cnt = print_adc_cfg( ctx,get_sensor_config(sensor),cnt);
    break;
    case S_T_GENERAL_232:
    case S_T_SNOW_HJ_232:
      cnt = print_rs232_cfg(ctx,get_sensor_config(sensor),cnt);
    break;
    case S_T_GENERAL_485:
    case S_T_SNOW_HJ_485:
    case S_T_HUMI_RS485:
      cnt = print_rs485_cfg(ctx,get_sensor_config(sensor),cnt);
    break;
  }
  return cnt;
}
int32_t print_menu_sensor_temp(p_shell_context_t ctx)
{
  int cnt = 0;
  sensor_t *sensor;
  sensor = &config.sensor[A1_TEMPERATURE];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);

  if(sensor->type != S_T_UNSUED)
  {
    ctx->printf("%2d.scale      :%d\r\n",cnt++,sensor->scale);
    cnt = print_common_cfg(ctx,sensor,cnt);  
  }

  return cnt;
}

void set_type(sensor_t *sensor)
{
  WRITE_CFG_MEM(&sensor->type,sizeof(sensor->type));
  if(sensor->type != S_T_UNSUED)
  {
    void *cfg = get_sensor_config(sensor);
    if(cfg == NULL)
    {
      sensor_add(sensor);
    }
  }
}

//¿Âµµ ¼³Á¤
int32_t menu_sensor_temp(p_shell_context_t ctx)
{
  const char *itemList[10];

  int32_t cnt;
  uint8_t itemListCnt;


  sensor_t *sensor = &config.sensor[A1_TEMPERATURE];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_temp,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      itemListCnt = gen_sensorItemList(itemList,temperatureList,_countof(temperatureList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      {
        sensor->type = (eSENSOR_MODEL_t)temperatureList[cnt-1];
        set_type(sensor);
      }
    }

    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-2);
          break;
        }
      }
    }
  }while(1);
}

//Ç³Çâ ¼³Á¤Á¤
int32_t print_menu_sensor_windDirection(p_shell_context_t ctx)
{

  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A2_WIND_DIRECTION];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}
int32_t menu_sensor_windDirection(p_shell_context_t ctx)
{

  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &config.sensor[A2_WIND_DIRECTION];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_windDirection,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      //¹æÇâ ¼¾¼­ ¸ñ·Ï °¡Á®¿È
      itemListCnt = gen_sensorItemList(itemList,windDirectionList,_countof(windDirectionList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)windDirectionList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}


//Ç³¼Ó ¼³Á¤Á¤
int32_t print_menu_sensor_windSpeed(p_shell_context_t ctx)
{
 // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A3_WIND_SPEED];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}
int32_t menu_sensor_windSpeed(p_shell_context_t ctx)
{

  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &config.sensor[A3_WIND_SPEED];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_windSpeed,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      //¹æÇâ ¼¾¼­ ¸ñ·Ï °¡Á®¿È
      itemListCnt = gen_sensorItemList(itemList,windDirectionList,_countof(windDirectionList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)windDirectionList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}



int32_t print_menu_sensor_windDirectionInstanct(p_shell_context_t ctx)
{
 // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A4_INSTANT_WIND_DIRECTION];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);

  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}
int32_t menu_sensor_windDirectionInstant(p_shell_context_t ctx)
{

  int32_t cnt;
  uint8_t itemListCnt;
 const  char *itemList[10];

  sensor_t *sensor = &config.sensor[A4_INSTANT_WIND_DIRECTION];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_windDirectionInstanct,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      itemListCnt = gen_sensorItemList(itemList,windDirectionInstantList,_countof(windDirectionInstantList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)windDirectionInstantList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}


int32_t print_menu_sensor_windSpeedInstanct(p_shell_context_t ctx)
{

  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A5_INSTANT_WIND_SPEED];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);

  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}
int32_t menu_sensor_windSpeedInstant(p_shell_context_t ctx)
{

  int32_t cnt;
  uint8_t itemListCnt;
 const  char *itemList[10];

  sensor_t *sensor = &config.sensor[A5_INSTANT_WIND_SPEED];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_windSpeedInstanct,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      itemListCnt = gen_sensorItemList(itemList,windDirectionInstantList,_countof(windDirectionInstantList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)windDirectionInstantList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}

//Àû¼³ ¼³Á¤
int32_t print_menu_sensor_snow(p_shell_context_t ctx)
{
 // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A9_SNOW_DEPTH];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}
int32_t menu_sensor_snow(p_shell_context_t ctx)
{

  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &config.sensor[A9_SNOW_DEPTH];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_snow,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      itemListCnt = gen_sensorItemList(itemList,snowList,_countof(snowList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)snowList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}

int32_t print_menu_sensor_rain(p_shell_context_t ctx)
{
 // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A6_RAINFALL_DOT5_1MM];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}

int32_t menu_sensor_rain(p_shell_context_t ctx)
{
  //int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &config.sensor[A6_RAINFALL_DOT5_1MM];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_rain,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      //°­¿ì ¼¾¼­ÀÇ ¹®ÀÚ¿­ ¸ñ·ÏÀ» °¡Á®¿Â´Ù.
      itemListCnt = gen_sensorItemList(itemList,rainList,_countof(rainList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)rainList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}

//±â¾Ð ¼³Á¤
int32_t print_menu_sensor_pressure(p_shell_context_t ctx)
{
  //uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A7_PRESSURE];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}

int32_t menu_sensor_pressure(p_shell_context_t ctx)
{
  //int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &config.sensor[A7_PRESSURE];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_pressure,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {
      //°­¿ì ¼¾¼­ÀÇ ¹®ÀÚ¿­ ¸ñ·ÏÀ» °¡Á®¿Â´Ù.
      itemListCnt = gen_sensorItemList(itemList,pressureList,_countof(pressureList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)pressureList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}


//½Àµµ ¼³Á¤
int32_t print_menu_sensor_humi(p_shell_context_t ctx)
{
 // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A10_RELATIVE_HUMIDITY];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}

int32_t menu_sensor_humi(p_shell_context_t ctx)
{
  //int32_t dec;
  int32_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  sensor_t *sensor = &config.sensor[A10_RELATIVE_HUMIDITY];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_humi,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {

      itemListCnt = gen_sensorItemList(itemList,humiList,_countof(humiList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)humiList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}


//°­¼öÀ¯¹« ¼³Á¤
int32_t print_menu_sensor_rainPresent(p_shell_context_t ctx)
{
 // uint8_t type;
  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A8_RAIN_PRESENT];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  switch(sensor->type)
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

  sensor_t *sensor = &config.sensor[A8_RAIN_PRESENT];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_rainPresent,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    if(cnt==0)
    {

      itemListCnt = gen_sensorItemList(itemList,rainPresentList,_countof(rainPresentList));
      cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
      if(cnt > 0)
      { 
        sensor->type = (eSENSOR_MODEL_t)rainPresentList[cnt-1];
        set_type(sensor);
      }
    }
    else
    {
      for(int i = 0 ; i< _countof(sen_func);i++)
      {
        if(sen_func[i].sensorType == sensor->type)
        {
          sen_func[i].config_set(ctx,sensor,cnt-1);
          break;
        }
      }
    }
  }while(1);
}





void sensor_set(p_shell_context_t ctx,sensor_t *sensor,uint8_t cnt)
{
    for(int i = 0 ; i< _countof(sen_func);i++)
    {
      if(sen_func[i].sensorType == sensor->type)
      {
        sen_func[i].config_set(ctx,sensor,cnt-1);
        break;
      }
    }
}

/**
 * @brief ¼¾¼­ ¸ðµ¨ º¯°æ
*/
void sensor_type_set(p_shell_context_t ctx,sensor_t *sensor,const uint8_t *list,uint8_t listCnt)
{
  uint8_t cnt;
  uint8_t itemListCnt;
  const char *itemList[10];

  itemListCnt = gen_sensorItemList(itemList,list,listCnt);
  cnt = select_indexFromList(ctx,itemList,NULL,itemListCnt,true);
  if(cnt > 0)
  { 
    sensor->type =(eSENSOR_MODEL_t) list[cnt-1];
    set_type(sensor);
  }

}

//±âº» ¼³Á¤
int32_t print_menu_sensor_default(p_shell_context_t ctx)
{

  int cnt = 0;
  sensor_t *sensor;

  sensor = &config.sensor[A11_RAINFALL_DOT1MM];

  ctx->printf("%2d.type       :%s\r\n",cnt++,sensorTypeList[sensor->type]);
  cnt = print_common_cfg(ctx,sensor,cnt);
  return cnt;
}

int32_t menu_sensor_default(p_shell_context_t ctx)
{
  int32_t cnt;
  sensor_t *sensor = &config.sensor[A11_RAINFALL_DOT1MM];

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor_humi,0,false);
    
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    switch(cnt)
    {
      case 0://Å¸ÀÔ
      sensor_type_set(ctx,sensor,humiList,_countof(humiList));
      break; 
      default:
      sensor_set(ctx,sensor,cnt-1);
      break;
    }
  }while(1);
}

//¼¾¼­ ¸ñ·Ï·Ï
const menu_func g_sensorMenu[SENSOR_LIST_MAX]={[A1_TEMPERATURE] = menu_sensor_temp,
                                  [A2_WIND_DIRECTION] = menu_sensor_windDirection,
                                  [A3_WIND_SPEED] = menu_sensor_windSpeed,
                                  [A4_INSTANT_WIND_DIRECTION]=menu_sensor_windDirectionInstant,
                                  [A5_INSTANT_WIND_SPEED]    =menu_sensor_windSpeedInstant,
                                  [A6_RAINFALL_DOT5_1MM] = menu_sensor_rain,
                                  [A7_PRESSURE] = menu_sensor_pressure,
                                  [A8_RAIN_PRESENT] = menu_sensor_rainPresent,
                                  [A9_SNOW_DEPTH] = menu_sensor_snow,
                                  [A10_RELATIVE_HUMIDITY]=menu_sensor_humi,
                                  [A11_RAINFALL_DOT1MM]=menu_sensor_humi,
                                  [B1_SOLAR_RADIATION]=menu_sensor_default,
                                  [B2_SUNSHINE_DURATION]=menu_sensor_default,
                                  [B3_GROUND_TEMPERATURE]=menu_sensor_default,
                                  [B4_SURFACE_TEMPERATURE]=menu_sensor_default,
                                  [B5_SOIL_TEMPERATURE_5CM]=menu_sensor_default,
                                  [B6_SOIL_TEMPERATURE_10CM]=menu_sensor_default,
                                  [B7_SOIL_TEMPERATURE_20CM]=menu_sensor_default,
                                  [B8_SOIL_TEMPERATURE_30CM]=menu_sensor_default,
                                  [B9_SOIL_TEMPERATURE_50CM]=menu_sensor_default,
                                  [B10_SOIL_TEMPERATURE_100CM]=menu_sensor_default,
                                  [B11_SOIL_TEMPERATURE_150CM]=menu_sensor_default,
                                  [B12_SOIL_TEMPERATURE_300CM]=menu_sensor_default,
                                  [B13_SOIL_TEMPERATURE_500CM]=menu_sensor_default,
                                  [C1_CLOUD_BASE1]=menu_sensor_default,
                                  [C2_CLOUD_BASE2]=menu_sensor_default,
                                  [C3_CLOUD_BASE3]=menu_sensor_default,
                                  [C4_CLOUD_COVER]=menu_sensor_default,
                                  [C5_VISIBILITY]=menu_sensor_default,
                                  [C6_PM10]=menu_sensor_default,
                                  [C7_PM2DOT5]=menu_sensor_default,
                                  [C8_NET_RADIATION]=menu_sensor_default,
                                  [C9_TOTAL_RADIATION]=menu_sensor_default,
                                  [C10_REFLECTED_RADIATION]=menu_sensor_default,
                                  [C11_DIRECT_SOLAR]=menu_sensor_default,
                                  [C12_CURRENT_WEATHER]=menu_sensor_default,
                                  [N1_SOIL_MOISTURE_10CM]=menu_sensor_default,
                                  [N2_SOIL_MOISTURE_20CM]=menu_sensor_default,
                                  [N3_SOIL_MOISTURE_30CM]=menu_sensor_default,
                                  [N4_SOIL_MOISTURE_50CM]=menu_sensor_default,
                                  [N5_ILLUMINANCE]=menu_sensor_default,
                                  [N6_WIND_VELOCITY_150CM]=menu_sensor_default,
                                  [N7_WIND_VELOCITY_400CM]=menu_sensor_default,
                                  [N8_INSTANT_VELOCITY_150CM]=menu_sensor_default,
                                  [N9_INSTANT_VELOCITY_400CM]=menu_sensor_default,
                                  [N10_AIR_TEMPERATURE_50CM]=menu_sensor_default,
                                  [N11_AIR_TEMPERATURE_400CM]=menu_sensor_default,
                                  [N12_HUMIDITY_50CM]=menu_sensor_default,
                                  [N13_HUMIDITY_400CM]=menu_sensor_default,
                                  [I1_TACHOMETER]=menu_sensor_default,
                                  [USER_WATER]=menu_sensor_default,
                                  [USER_SWV]=menu_sensor_default,
                                  [USER_FLOW_RATE]=menu_sensor_default,
                                  [USER_SLOPE_1]=menu_sensor_default,
                                  [USER_SLOPE_2]=menu_sensor_default,
                                  [USER_SLOPE_3]=menu_sensor_default,
                                  [USER_SLOPE_4]=menu_sensor_default,
                                  [USER_SLOPE_5]=menu_sensor_default,
                                  [USER_SLOPE_6]=menu_sensor_default,
                                  [USER_SLOPE_7]=menu_sensor_default,
                                  [USER_SLOPE_8]=menu_sensor_default,
                                  [USER_SLOPE_9]=menu_sensor_default,
                                  [USER_SLOPE_10]=menu_sensor_default,
                                  [USER_DEFAULT]=menu_sensor_default};


int32_t menu_sensor_default_2(p_shell_context_t ctx,eSENSOR_LIST_t list)
{
  int32_t cnt=0;
  sensor_t *sensor = &config.sensor[(int)list];
  do
  {
  //  cnt = print_common_cfg(ctx,sensor);

  //  cnt = select_indexFromList(ctx,NULL,NULL,cnt,false);
    
   cnt =  select_indexMenu(ctx,sensor);

    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM && cnt<= 0)
    {
      return cnt;
    }
    cnt--;
    
    switch(cnt)
    {
      case 0://Å¸ÀÔ
      sensor_type_set(ctx,sensor,supported_sensors[list].list,supported_sensors[list].cnt);
      break; 
      default:
      sensor_set(ctx,sensor,cnt);
      break;
    }
  }while(1);
}



/**
 * @brief ¼¾¼­ ¸Þ´º
 * @retval 
 */
int32_t menu_sensor(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_sensor,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM)
    {
      break;
    }
#if 0 
    if(g_sensorMenu[cnt-1])
    {
      cnt = g_sensorMenu[cnt-1](ctx);
    }
#endif
    cnt = menu_sensor_default_2(ctx,(eSENSOR_LIST_t)(cnt-1));
  }while(cnt != EXIT_PROGRAM);

  return cnt;
}






int32_t print_net_use(p_shell_context_t ctx)
{
  int32_t cnt=3;

  ctx->printf(" 0.ÀÌ´õ³Ý  :%s\r\n",ITEM_LIST((int)config.eth_use,enableList));
  ctx->printf(" 1.CDMA    :%s\r\n",ITEM_LIST((int)config.cdma_use,enableList));
  ctx->printf(" 2.Á÷Á¢Åë½Å:%s\r\n",ITEM_LIST((int)config.direct_use,enableList));

  return cnt;
}



int32_t menu_net_use(p_shell_context_t ctx)
{
  int32_t cnt;


  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_use,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK )
    {
      return cnt;
    }
    cnt--;

    switch(cnt)
    {
      case 0:
      if(input_use(ctx,&config.eth_use))
      {
        WRITE_CFG(eth_use);
      }
      break;
          case 1:
      if(input_use(ctx,&config.cdma_use))
      {
        WRITE_CFG(cdma_use);
      }
      break;
      case 2:
      if(input_use(ctx,&config.direct_use))
      {
        WRITE_CFG(direct_use);
      }
      break;
    }   
  } while (1);

}


int32_t print_net_eth_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  ctx->printf("%2d.¿ø°Ý ¼­¹ö Á¤º¸\r\n",cnt++);
  ctx->printf("%2d.±âº» ±¸¼º\r\n",cnt++);

  return cnt;
}





int32_t print_net_eth_remote_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  uint8_t *ip = config.eth_server_ip;

  ctx->printf("%2d.ip      :%d.%d.%d.%d\r\n",cnt++,ip[0],ip[1],ip[2],ip[3]);
  ctx->printf("%2d.port    :%d\r\n",cnt++,config.eth_server_port);
  ctx->printf("%2d.protocol:%s\r\n",cnt++,ITEM_LIST(config.eth_protocol,protocolList));

  return cnt;
}



int32_t menu_net_eth_remote_set(p_shell_context_t ctx)
{

  int32_t cnt;
  int32_t a,b,c,d;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_eth_remote_set,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch(cnt)
    { 
      case 0:
      ctx->printf("xxx.xxx.xxx.xxx:");
      if(console_scanf("%d.%d.%d.%d", &a,&b,&c,&d)==4)
      {
        config.eth_server_ip[0] = a;
        config.eth_server_ip[1] = b;
        config.eth_server_ip[2] = c;
        config.eth_server_ip[3] = d;
        WRITE_CFG(eth_server_ip);
      }
      break;
      case 1:
      if(input_decimal(ctx,0,60000,&dec))
      {
        config.eth_server_port = dec;
        WRITE_CFG(eth_server_port);
      }
      break;
      case 2://ÇÁ·ÎÅäÄÝ
        cnt = select_indexFromList(ctx,protocolList,NULL,_countof(protocolList),true);
        if(cnt>0)
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

  ctx->printf(" 0.ip      :%d.%d.%d.%d\r\n",ip[0],ip[1],ip[2],ip[3]);
  ctx->printf(" 1.subnet  :%d.%d.%d.%d\r\n",subnet[0],subnet[1],subnet[2],subnet[3]);
  ctx->printf(" 2.gateway :%d.%d.%d.%d\r\n",gw[0],gw[1],gw[2],gw[3]);

  return cnt;
}


int32_t menu_net_eth_default_set(p_shell_context_t ctx)
{

  int32_t cnt;
  int32_t a,b,c,d;


  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_eth_default_set,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch(cnt)
    { 
      case 0://ip
      ctx->printf("xxx.xxx.xxx.xxx:");
      if(console_scanf("%d.%d.%d.%d", &a,&b,&c,&d)==4)
      {
        config.eth_ip[0] = a;
        config.eth_ip[1] = b;
        config.eth_ip[2] = c;
        config.eth_ip[3] = d;
        WRITE_CFG(eth_ip);
      }
      break;
      case 1://subnet
      ctx->printf("xxx.xxx.xxx.xxx:");
      if(console_scanf("%d.%d.%d.%d", &a,&b,&c,&d)==4)
      {
        config.eth_subnet[0] = a;
        config.eth_subnet[1] = b;
        config.eth_subnet[2] = c;
        config.eth_subnet[3] = d;
        WRITE_CFG(eth_subnet);
      }
      break;
      case 2://gateway
      ctx->printf("xxx.xxx.xxx.xxx:");
      if(console_scanf("%d.%d.%d.%d", &a,&b,&c,&d)==4)
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


int32_t menu_net_eth_set(p_shell_context_t ctx)
{
  int32_t cnt;

  const menu_func menu[]={menu_net_eth_remote_set,
                          menu_net_eth_default_set};
  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_eth_set,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if(cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
  } while (1);

}


int32_t print_net_cdma_set(p_shell_context_t ctx)
{
  int32_t cnt = 0;
  uint8_t *ip  = config.cdma_server_ip;
  int32_t port = config.cdma_port;

  ctx->printf("%2d.ip      :%d.%d.%d.%d\r\n",cnt++,ip[0],ip[1],ip[2],ip[3]);
  ctx->printf("%2d.port    :%d\r\n",cnt++,port);
  ctx->printf("%2d.protocol:%s\r\n",cnt++,ITEM_LIST(config.cdma_protocol,protocolList));
  ctx->printf("%2d.model   :%s\r\n",cnt++,ITEM_LIST(config.cdmaType,cdmaModellList));

  return cnt;
}

int32_t menu_net_cdma_set(p_shell_context_t ctx)
{

  int32_t cnt;
  int32_t a,b,c,d;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_cdma_set,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
    {
      return cnt;
    }

    cnt--;
    switch(cnt)
    { 
      case 0:
      ctx->printf("xxx.xxx.xxx.xxx:");
      if(console_scanf("%d.%d.%d.%d", &a,&b,&c,&d)==4)
      {
        config.cdma_server_ip[0] = a;
        config.cdma_server_ip[1] = b;
        config.cdma_server_ip[2] = c;
        config.cdma_server_ip[3] = d;
        WRITE_CFG(cdma_server_ip);
      }
      break;
      case 1:
      if(input_decimal(ctx,0,60000,&dec))
      {
        config.cdma_port = dec;
        WRITE_CFG(cdma_port);
      }
      break;
      case 2://ÇÁ·ÎÅäÄÝ
        cnt = select_indexFromList(ctx,protocolList,NULL,_countof(protocolList),true);
        if(cnt>0)
        {
          cnt--;
          config.cdma_protocol = cnt;
          WRITE_CFG(cdma_protocol);
        }
      break;
      case 3://¸ðµ¨ 
        cnt = select_indexFromList(ctx,cdmaModellList,NULL,_countof(cdmaModellList),true);
        if(cnt>0)
        {
          cnt--;
          config.cdmaType = cnt;
          WRITE_CFG(cdmaType);
        }
      break;
    }
  } while (1);
}


int32_t print_net_direct_set(p_shell_context_t ctx)
{
  int32_t cnt = 2;

  ctx->printf(" 0.baud     :%d\r\n",cnt++,config.direct_baud);
  ctx->printf(" 1.protocol :%s\r\n",cnt++,ITEM_LIST(config.direct_protocol,protocolList));

  return cnt;
}

int32_t menu_net_direct_set(p_shell_context_t ctx)
{

  int32_t cnt;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_direct_set,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK)
    {
      return cnt;
    }

    cnt--;
    switch(cnt)
    { 
      case 0://baud
      if(input_decimal(ctx,0,115200,&dec))
      {
        config.direct_baud = dec;
        WRITE_CFG(direct_baud);
      }
      break;
      case 2://ÇÁ·ÎÅäÄÝ
        cnt = select_indexFromList(ctx,protocolList,NULL,_countof(protocolList),true);
        if(cnt>0)
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

  ctx->printf("%2d.ÀÌ´õ³Ý\r\n",cnt++);
  ctx->printf("%2d.CDMA\r\n",cnt++);
  ctx->printf("%2d.Á÷Á¢ Åë½Å\r\n",cnt++);

  return cnt;
}

int32_t menu_net_set(p_shell_context_t ctx)
{
  int32_t cnt;
  const menu_func menu[]={menu_net_eth_set,
                          menu_net_cdma_set,
                          menu_net_direct_set};
  do
  {
    cnt = select_indexFromList(ctx,NULL,print_net_set,0,false);
    if(cnt == EXIT_PROGRAM || cnt == EXIT_BACK || cnt <= 0)
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
  int32_t cnt=0;
  
  ctx->printf("%2d.±×·ì         :%d\r\n",cnt++,config.vhf_group);
  ctx->printf("%2d.VHF ID       :%d\r\n",cnt++,config.vhf_id);
  ctx->printf("%2d.Áß°è ID      :%d\r\n",cnt++,config.vhf_repeater_id);
  ctx->printf("%2d.ÅëÁ¦ ID      :%d\r\n",cnt++,config.vhf_host_id);
  ctx->printf("%2d.PTT ½Ã°£(ms) :%d\r\n",cnt++,config.vhf_ptt_delay);
  ctx->printf("%2d.VHF °¡»ó ¼³Á¤\r\n",cnt++);
  ctx->printf("%2d.VHF ·çÇÁ Å×½ºÆ®\r\n",cnt++);
  ctx->printf("%2d.VHF Åæ Å×½ºÆ®\r\n",cnt++);

  return cnt;

}

int32_t menu_net_vhf_vir_set(p_shell_context_t ctx)
{
  
  return 0;

}
int32_t menu_net_vhf_loop_test(p_shell_context_t ctx)
{
  
  return 0;
}

int32_t menu_net_vhf_tone_test(p_shell_context_t ctx)
{
  
  return 0;
}


int32_t menu_net_vhf(p_shell_context_t ctx)
{
  int32_t cnt;
//  int32_t ret;
  int32_t dec;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_vhf,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
    case 0://±×·ì
      if(input_decimal(ctx,0,255,&dec))
      {
        config.vhf_group = dec;
        WRITE_CFG(vhf_group);
      }
      break;
    case 1://id
      if(input_decimal(ctx,0,255,&dec))
      {
        config.vhf_id = dec;
        WRITE_CFG(vhf_id);
      }
      break;
    case 2://Áß°è
      if(input_decimal(ctx,0,255,&dec))
      {
        config.vhf_repeater_id = dec;
        WRITE_CFG(vhf_repeater_id);
      }
      break;
    case 3://È£½ºÆ®
      if(input_decimal(ctx,0,255,&dec))
      {
        config.vhf_host_id = dec;
        WRITE_CFG(vhf_host_id);
      }
      break;
    case 4://ptt
      if(input_decimal(ctx,0,255,&dec))
      {
        config.vhf_ptt_delay = dec;
        WRITE_CFG(vhf_ptt_delay);
      }
      break;
    case 5://
     menu_net_vhf_vir_set(ctx);
    break;
    default:
      break;
    }


  }while(cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t print_menu_net(p_shell_context_t ctx)
{
  int32_t cnt=0;
  char buff[50]={0};
  
  make_comList(buff,sizeof(buff));

  ctx->printf("%2d.Åë½Å ¹æ½Ä:%s\r\n",cnt++,buff);
  ctx->printf("%2d.Åë½Å ¼³Á¤\r\n",cnt++);
  ctx->printf("%2d.VHF\r\n",cnt++);

  return cnt;

}

int32_t menu_network(p_shell_context_t ctx)
{
  int32_t cnt;

  const menu_func menu[]={menu_net_use,
                          menu_net_set,
                          menu_net_vhf};
  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_net,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
  }while(cnt != EXIT_PROGRAM);

  return cnt;
}

int32_t menu_data(p_shell_context_t ctx)
{

  return 0;
}


int32_t print_menu_panel(p_shell_context_t ctx)
{
  int32_t cnt=0;

  ctx->printf("%2d.ÆÐ³Î Á¾·ù:%s\r\n",cnt++,ITEM_LIST(config.panelType,panelList));

  return cnt;

}
 int32_t menu_display_panel(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_panel,0,false);

    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt--;

    switch (cnt)
    {
      case 0:
      cnt = select_indexFromList(ctx,panelList,NULL,_countof(panelList),false);

      if(cnt > 0)
      {
        cnt--;
        config.panelType = cnt;
        WRITE_CFG(panelType);
      }
      break;
    }

  }while(1);


}


int32_t menu_manage_version(p_shell_context_t ctx)
{
  char buff[30];

  uint8_t a,b,c,d;
  DATE_TIME_BUF ct;

    get_appVer(&a,&b,&c,&d);
    get_appBuild(&ct);

    ctx->printf("App:%d.%d.%d.%d\r\n",a,b,c,d);
    make_timeToStr(&ct,buff,sizeof(buff));
    ctx->printf("App build:%s\r\n",buff);

    get_bootVer(&a,&b,&c,&d);
    get_bootBuild(&ct);

    ctx->printf("Boot:%d.%d.%d.%d\r\n",a,b,c,d);
    make_timeToStr(&ct,buff,sizeof(buff));
    ctx->printf("Boot build:%s\r\n",buff);
  return 0;

}
int32_t download_file(int32_t (*write_file)(char *path,uint32_t offset,uint8_t *data,uint32_t dataLen),
                        char *path,uint32_t offset,uint32_t *len,uint32_t limit);


int32_t write_file(char *path,uint32_t offset,uint8_t *data,uint32_t dataLen)
{

  flash_write(offset,data,dataLen);

  return 0;
}

int32_t menu_manage_update(p_shell_context_t ctx)
{
  char buff[20];
  uint32_t len;

  ctx->printf("10ÃÊµÚ¿¡ ÆÄÀÏÀ» Àü¼ÛÇØÁÖ¼¼¿ä\r\n");
  osDelay(10000);

  if(download_file(write_file,buff,0,&len,512*1024) ==0)
  {
    ctx->printf("ÆÄÀÏ Å©±â:%d\r\n",len);
  }
  else
  {
    ctx->printf("ÆÄÀÏ ¼ö½Å ¿À·ù\r\n");
  }



  return 0;
}
int32_t menu_manage_device_reset(p_shell_context_t ctx)
{
  
  
  reset_system(0,"console reset");
  return 0;
}
//ÃÊ±âÈ­
int32_t menu_manage_config_reset(p_shell_context_t ctx)
{


  memset(config.sensor,0,sizeof(config.sensor));

  WRITE_CFG(sensor);
  memset(&s_config,0,sizeof(s_config));
  write_s_config();
  return 0;
}

int32_t menu_manage_print_config_all(p_shell_context_t ctx)
{
  ctx->printf("ID               :%d\r\n",config.id);
  ctx->printf("ºñ¹Ð¹øÈ£         :%d\r\n",config.password);
  ctx->printf("ÃæÀü±â Á¾·ù      :%s\r\n",ITEM_LIST(config.chgType,g_chgList));
  ctx->printf("·Î±× Ä«¿îÆ®      :%d\r\n",config.logCnt);

  ctx->printf("ÀÌ´õ³Ý ¼­ºê³Ý    :%d.%d.%d.%d\r\n",config.eth_subnet[0],config.eth_subnet[1],
  config.eth_subnet[2],config.eth_subnet[3]);
  ctx->printf("ÀÌ´õ³Ý °ÔÀÌÆ®¿þÀÌ:%d.%d.%d.%d\r\n",config.eth_gateway[0],config.eth_gateway[1],
  config.eth_gateway[2],config.eth_gateway[3]);
  ctx->printf("ÀÌ´õ³Ý IP        :%d.%d.%d.%d\r\n",config.eth_ip[0],config.eth_ip[1],
  config.eth_ip[2],config.eth_ip[3]);
  ctx->printf("ÀÌ´õ³Ý ¿ø°Ý ¼­¹ö :%d.%d.%d.%d\r\n",config.eth_server_ip[0],config.eth_server_ip[1],
  config.eth_server_ip[2],config.eth_server_ip[3]);
;

  ctx->printf("ÀÌ´õ³Ó Æ÷Æ®      :%d\r\n",config.eth_server_port);
  ctx->printf("ÀÌ´õ³Ý ÇÁ·ÎÅäÄÝ  :%s\r\n",ITEM_LIST(config.eth_protocol,protocolList));
  ctx->printf("CDMA ¿ø°Ý ¼­¹ö   :%d.%d.%d.%d\r\n",config.cdma_server_ip[0],config.cdma_server_ip[1],
  config.cdma_server_ip[2],config.cdma_server_ip[3]);
  ctx->printf("CDMA Æ÷Æ®        :%d\r\n",config.cdma_port);
  ctx->printf("CDMA ÇÁ·ÎÅäÄÝ    :%s\r\n",ITEM_LIST(config.cdma_protocol,protocolList));
  ctx->printf("CDMA Á¾·ù        :%s\r\n",ITEM_LIST(config.cdmaType,cdmaModellList));
  ctx->printf("ÀÌ´õ³Ý »ç¿ë      :%s\r\n", ITEM_LIST((int32_t)config.eth_use,enableList));
  ctx->printf("CDMA »ç¿ë        :%s\r\n",ITEM_LIST((int32_t)config.cdma_use,enableList));
  ctx->printf("Á÷Á¢Åë½Å         :%s\r\n",ITEM_LIST((int32_t)config.direct_use,enableList));
  ctx->printf("Á÷Á¢Åë½Å ÇÁ·ÎÅäÄÝ:%s\r\n",ITEM_LIST(config.direct_protocol,protocolList));
  ctx->printf("Á÷Á¢Åë½Å ¼Óµµ    :%d\r\n",config.direct_baud);
  ctx->printf("ÆÐ³Î Á¾·ù        :%s\r\n",ITEM_LIST(config.panelType,panelList));
  ctx->printf("VHF ID           :%d\r\n",config.vhf_id);
  ctx->printf("VHF ±×·ì         :%d\r\n",config.vhf_group);
  ctx->printf("VHF HOST         :%d\r\n",config.vhf_host_id);
  ctx->printf("VHF Áß°è         :%d\r\n",config.vhf_repeater_id);
  ctx->printf("VHF PTT Áö¿¬     :%d\r\n",config.vhf_ptt_delay);

  return 0;
}
menu_func g_manageMenu[]={[0]=menu_manage_version,
                              menu_manage_update,
                              menu_manage_device_reset,
                              menu_manage_config_reset,
                              menu_manage_print_config_all};

int32_t print_menu_manage(p_shell_context_t ctx)
{
  int32_t cnt=0;
  
  ctx->printf("\r\n");
  ctx->printf(" 0.version\r\n");
  ctx->printf(" 1.fw update\r\n");
  ctx->printf(" 2.device reset\r\n");
  ctx->printf(" 3.config factory reset\r\n");
  ctx->printf(" 4.print config all\r\n");
  cnt = 5;
  return cnt;
}

int32_t menu_manage(p_shell_context_t ctx)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_manage,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = g_manageMenu[cnt](ctx);
    if(cnt == EXIT_PROGRAM )
    {
      return cnt;
    }
  }while(1);

  //return 0;//
}




int32_t print_menu_cali_adc(p_shell_context_t ctx)
{
  int32_t cnt=0;
  
  ctx->printf("\r\n");

  ctx->printf(" 0.offset  :%d\r\n",g_adc_cali_config.single[0].offset);
  ctx->printf(" 1.fullset :%d\r\n",g_adc_cali_config.single[0].fullset);

  cnt = 2;
  return cnt;
}


int32_t print_menu_cali_single(p_shell_context_t ctx)
{
  int32_t cnt=0;
  
  ctx->printf("\r\n");
  for(int i = 0 ; i < 18; i++)
  {
    ctx->printf("%2d.single channel %d\r\n",i,i);
  }
  cnt = 18;
  return cnt;
}


int32_t  inpu_adc_cali(p_shell_context_t ctx,int adcMode,int channel,int32_t cfg_adc,int32_t cfg_ref,int32_t *adc_data,int32_t *ref_vol)
{
  uint8_t err=0;
  int32_t ch;
  int32_t adc;
  int32_t voltage=0;


  ctx->printf("ADC %s,ch:%d\r\n",adcChModeList[adcMode],channel);
  ctx->printf("config adc:%d, ref:%d\r\n",cfg_adc,cfg_ref);
  do
  {
    adc = 0;
    if(adcMode == 0)//single
    {
      adc = adc_read_single_avg(channel,&err,10);
    }
    else
    {
      adc =  adc_read_diff_avg( channel,&err,10);
    }
    if(err)
    {
      ctx->printf("adc error:%d\r",err) ;
    }
    else 
    {
      ctx->printf("current adc:%7d\r",adc) ;
    }

  osDelay(1000);
  ch = DbgConsole_GetcharNonBlocking();
  if(ch !=-1)
  {
    break;
  }

  } while(1);

  ctx->printf("\r\n") ;
  vt100_printfColor(GREEN,"ADC °ªÀ» ¼öµ¿À¸·Î ÀÔ·ÂÇØ ÁÖ¼¼¿ä:");
  if(input_digit(ctx,-8388607,8388607,&adc,eUINT32) != 1)
  {
    return 1;
  }

  ctx->printf("\r\n") ;
  vt100_printfColor(GREEN,"ÀÔ·ÂµÈ Àü¾Ð°ªÀ» ÀÔ·ÂÇØ ÁÖ¼¼¿ä(mV):");
  if(input_digit(ctx,0,5000,&voltage,eUINT32) != 1)
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
    cnt = select_indexFromList(ctx,NULL,print_menu_cali_single,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    channel = cnt;

    while(1)
    {
      ctx->printf(" 0.offset  :%d\r\n",g_adc_cali_config.single[channel].offset);
      ctx->printf(" 1.fullset :%d\r\n",g_adc_cali_config.single[channel].fullset);
      ctx->printf("Please enter a number:");

      cnt = console_scanf("%d",&index);

      if(cnt==EXIT_BACK )
      {
        break;
      }
      if(cnt==EXIT_PROGRAM)
      {
        return cnt;
      }

        switch (index)
        {
        case 0://offset
        if(inpu_adc_cali(ctx,0,channel,g_adc_cali_config.single[channel].offset,
                                      g_adc_cali_config.single[channel].offset_input,
                                      &adc,&voltage)==0)
        {
          ctx->printf("offset:%d, voltage:%d\r\n",adc,voltage);
          g_adc_cali_config.single[channel].offset = adc;
          g_adc_cali_config.single[channel].offset_input = voltage;
          WRITE_CFG_CALI(single[channel].offset);
          WRITE_CFG_CALI(single[channel].offset_input);
        }
        break;

      case 1://fullset
      if(inpu_adc_cali(ctx,0,channel,g_adc_cali_config.single[channel].fullset,
                                    g_adc_cali_config.single[channel].fullset_input,
                                    &adc,&voltage)==0)
        {
          ctx->printf("offset:%d, voltage:%d\r\n",adc,voltage);
          g_adc_cali_config.single[channel].fullset = adc;
          g_adc_cali_config.single[channel].fullset_input = voltage;
          WRITE_CFG_CALI(single[channel].fullset);
          WRITE_CFG_CALI(single[channel].fullset_input);
        }
        break;    
        }
    }
  }while(1);

  //return 0;//
}



int32_t print_menu_cali_diff(p_shell_context_t ctx)
{
  int32_t cnt=0;
  
  ctx->printf("\r\n");
  for(int i = 0 ; i < 8; i++)
  {
    ctx->printf("%2d.diff channel %d\r\n",i,i);
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
    cnt = select_indexFromList(ctx,NULL,print_menu_cali_diff,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    channel = cnt;

    ctx->printf(" 0.offset  :%d\r\n",g_adc_cali_config.diff[channel].offset);
    ctx->printf(" 1.fullset :%d\r\n",g_adc_cali_config.diff[channel].fullset);

    ctx->printf("num:");

    cnt = console_scanf("%d",&index);

    if(cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }


    switch (index)
    {
    case 0://offset
    if(inpu_adc_cali(ctx,0,channel,g_adc_cali_config.diff[channel].offset,
                                   g_adc_cali_config.diff[channel].offset_input,
                                   &adc,&voltage)==0)
    {
      ctx->printf("offset:%d, voltage:%d\r\n",adc,voltage);
      g_adc_cali_config.diff[channel].offset = adc;
      g_adc_cali_config.diff[channel].offset_input = voltage;
      WRITE_CFG_CALI(diff[channel].offset);
      WRITE_CFG_CALI(diff[channel].offset_input);
    }
    break;
    case 1://fullset
    if(inpu_adc_cali(ctx,0,channel,g_adc_cali_config.diff[channel].fullset,
                                   g_adc_cali_config.diff[channel].fullset_input,
                                   &adc,&voltage)==0)
    {
      ctx->printf("offset:%d, voltage:%d\r\n",adc,voltage);
      g_adc_cali_config.diff[channel].fullset = adc;
      g_adc_cali_config.diff[channel].fullset_input = voltage;
      WRITE_CFG_CALI(diff[channel].fullset);
      WRITE_CFG_CALI(diff[channel].fullset_input);
    }
    break;    
    }


    if(cnt == EXIT_PROGRAM )
    {
      return cnt;
    }
  }while(1);

  //return 0;//
}







int32_t menu_cali_config_all(p_shell_context_t ctx)
{
  float voltage;
  int32_t adc;
  uint8_t err = 0;
  int32_t off,full,off_in,full_in;


  ctx->printf(VT100_CLEAR_SCREEN);
  ctx->printf(VT100_CURSOR_OFF);

  do
  {
    ctx->printf(VT100_CURSOR_HOME);
    
    ctx->printf("%-10s %-2s %-7s %-10s %-10s %-10s %-12s %-10s\r\n","Mode","Ch","offset","fullset","o_in(mv)","f_in(mv)","adc_avg(10)","voltage(v)");

    for(int i = 0 ; i < 18; i++)
    {
      adc = adc_read_single_avg(i,&err,10);
      off = g_adc_cali_config.single[i].offset;
      full = g_adc_cali_config.single[i].fullset;
      off_in =g_adc_cali_config.single[i].offset_input;
      full_in =g_adc_cali_config.single[i].fullset_input;
      voltage = cvt_adcToVol(adc,off,full,off_in,full_in);
      if(err)
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12s %-10s \r\n","Single",
        i+1,off,full,off_in,full_in,"error"," ");;
      }
      else
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12d %-8.6f \r\n","Single",
        i+1,off,full,off_in,full_in,adc,voltage/1000.0);;
      }



  }

  for(int i = 0 ; i < 8; i++)
  {
      adc     = adc_read_diff_avg(i,&err,10);
      off     = g_adc_cali_config.diff[i].offset;
      full    = g_adc_cali_config.diff[i].fullset;
      off_in  = g_adc_cali_config.diff[i].offset_input;
      full_in = g_adc_cali_config.diff[i].fullset_input;
      voltage = cvt_adcToVol(adc,off,full,off_in,full_in);
      if(err)
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12s %-10s \r\n","Diff",
        i+1,off,full,off_in,full_in,"error"," ");;
      }
      else
      {
        ctx->printf("%-10s %-2d %-7d %-10d %-10d %-10d %-12d %-8.6f \r\n","Diff",
        i+1,off,full,off_in,full_in,adc,voltage/1000.0);;
      }

  }
  }while(wait_break(1000));

  return 0;
}

bool check_password(void)
{
  int32_t password;

  debug_printf("Please enter the password:\r\n");

  if(console_scanf("%d",&password)==1)
  {
    if(password==7777)
    {
      return true;
    }
  }

  debug_printf("The password does not match\r\n");
  return false;
}

int32_t menu_cali_config_factory(p_shell_context_t ctx)
{

  if(check_password()!=true)
  {
    return 0;
  }

  for(int i = 0 ;i <18; i++)
  {
    g_adc_cali_config.single[i].offset = -2559;
    g_adc_cali_config.single[i].offset_input = 0;
    g_adc_cali_config.single[i].fullset = 8384783;
    g_adc_cali_config.single[i].fullset_input = 5000;
  }

  for(int i = 0 ;i <8; i++)
  {
    g_adc_cali_config.diff[i].offset = 63325;
    g_adc_cali_config.diff[i].offset_input = 0;
    g_adc_cali_config.diff[i].fullset = 8319265;
    g_adc_cali_config.diff[i].fullset_input = 5000;
  }

  config_write_adcCalibraion();


  debug_printf("+config facory:ok\r\n");
  return 0;
}

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
  eADC_CH_TYPE_t  adcMode;
  int32_t start,stop;
  ctx->printf("Ã¤³Î ¸ðµå¸¦ ¼±ÅÃÇØÁÖ¼¼¿ä\r\n");
  
  cnt = select_indexFromList(ctx,adcChModeList,NULL,_countof(adcChModeList),true);

  if(cnt<=0)
  {
    return 0;
  }

  cnt--;
  adcMode = (eADC_CH_TYPE_t)cnt;


  //ctx->printf("Ã¤³Î ¹øÈ£¸¦ ÀÔ·ÂÇØÁÖ¼¼¿ä\r\n");
  vt100_printfColor(GREEN,"Ã¤³Î ¹øÈ£¸¦ ÀÔ·ÂÇØÁÖ¼¼¿ä:");

  if(adcMode == eSINGLE_ADC)
  {
    start = 1;
    stop = 18;
  }
  else
  {
    start = 1;
    stop = 8;
  }

  if(input_decimal(ctx,start,stop,&channel)==1)
  {
    channel--;//0±âÁØÀ¸·Î 
    do{
        if(adcMode==eSINGLE_ADC)//single
        {
          adc = adc_read_single_avg(channel,&err,5);
        }
        else//diff
        {
          adc = adc_read_diff_avg(channel,&err,5);
        }

        voltage = adc_chToVoltage(adcMode,channel,adc);
        make_timeToStr(&Date_Time,buff,sizeof(buff));
        ctx->printf("%s MODE:%s CH:%d ADC:%d %8.6f\r\n",buff,adcMode==eSINGLE_ADC?"s":"d",channel+1,adc,voltage);
    }while(wait_break(1000));
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
    make_timeToStr(&Date_Time,buff,sizeof(buff));
    ctx->printf("%s,",buff);
   
   for(int i = 0; i< 18; i++)
   {
      adc = adc_read_single_avg(i,&err,5);

      if(err)
      {
        voltage = NAN;
      }
      else
      {
      voltage = adc_chToVoltage(0,i,adc);
      }

      ctx->printf("%2d:%7d,%8.6f ",i+1,adc,voltage);
    }
     ctx->printf("\r\n");

  } while (wait_break(1000));
  

  return 0;


}


menu_func g_calibraionMenu[]={[0]=menu_cali_single,
                                   menu_cali_diff,
                                   menu_cali_config_all,
                                   menu_cali_print_adc,
                                   menu_cali_print_adc_all,
                                   menu_cali_config_factory};

int32_t print_menu_calibration(p_shell_context_t ctx)
{
  int32_t cnt=0;
  
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
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_calibration,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = g_calibraionMenu[cnt](ctx);
    if(cnt == EXIT_PROGRAM )
    {
      return cnt;
    }
  }while(1);

  //return 0;//
}

int32_t menu_developer_interrupt(p_shell_context_t ctx)
{
  PrintAllInterrupts();
  return 0;
}



void print_flash(uint32_t start,uint32_t size,uint32_t width)
{
    uint8_t buff[512];
    uint32_t quot;
    uint32_t rem;
    uint32_t i;

    quot = size/512;
    rem = size % 512;

    for(i = 0;i<quot;i++)
    {
        flash_read(start+i*512,buff,512,512);
        LOG_MEM(buff,sizeof(buff),start+i*512,width);
        
    }

    if(rem)
    {
        flash_read(start+i*512,buff,512,rem);
        LOG_MEM(buff,rem,start+i*512,width);
    }

}

int32_t menu_developer_memory(p_shell_context_t ctx)
{
  int32_t cnt;
  int32_t inCnt;
  int32_t start,size,len;

  const char *memList[]={"flash","fram"};

  cnt = select_indexFromList(ctx,memList,NULL,_countof(memList),true);

  if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
  {
    return cnt;
  }

  ctx->printf("start,size,len>>");

  inCnt = console_scanf("%d,%d,%d",&start,&size,&len);
  if(inCnt == EXIT_BACK || inCnt==EXIT_PROGRAM && cnt <= 0)
  {
    return inCnt;
  }

  cnt--;
  switch(cnt)
  {
    case 0://flash;
      print_flash(start,size,len);
    break;
    case 1://fram
    break;
  }
return 0;
}




int32_t print_developer_sensor(p_shell_context_t ctx)
{
  int32_t cnt=0;
//  int32_t i=0;

  ctx->printf("\r\n");
  for(int i = 0 ;i<_countof(sensorNameList);i++)
  {
  //  ctx->printf("%2d.%-15s:%s,%d\r\n",i,sensorNameList[i], ITEM_LIST(g_sensor_emul[i].use,enableList),g_sensor_emul[i].data);
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
  while(1)
  {


  cnt = select_indexFromList(ctx,NULL,print_developer_sensor,0,true);

  if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
  {
    return cnt;
  }

  cnt--;

  ctx->printf("use,data:");
  if(console_scanf("%d,%d",&use,&dec)==2)
  {
    g_sensor_emul[cnt].enable = use;
    g_sensor_emul[cnt].data.i = dec;
  }
}

}

int32_t print_menu_developer(p_shell_context_t ctx)
{
  int32_t cnt=0;
  
  ctx->printf("\r\n");
  ctx->printf("%2d.interrupt\r\n",cnt++);
  ctx->printf("%2d.memory\r\n",cnt++);
  ctx->printf("%2d.sensor emul\r\n",cnt++);
  ctx->printf("%2d.print sensor config\r\n",cnt++);
  return cnt;
}

int32_t menu_developer_sensor_config(p_shell_context_t ctx)
{
 // char opt[20];
  int32_t cnt=0;
  int i=0;

  ctx->printf("\r\n");

#if 1
  cnt = _countof(sensorNameList);

  for( i = 0 ; i< cnt;i++)
  {
    ctx->printf("%2d:%d",i,config.sensor[i].configCnt);
    for(int j=0; j<4; j++)
    {
        ctx->printf("[%-15s.%d]",ITEM_LIST(config.sensor[i].config[j][0],sensorTypeList),config.sensor[i].config[j][1]);
    }
    
  ctx->printf("\r\n");

  }

#endif
  cnt =  _countof(sensorNameList);
  return cnt;
}

int32_t menu_developer_test_rs232(p_shell_context_t ctx)
{
  
  

  return 0;
}

int32_t print_menu_developer_test(p_shell_context_t ctx)
{
  int32_t cnt=0;

  ctx->printf("%2d:RS232",cnt++);

  return cnt;
}

int32_t menu_developer_test(p_shell_context_t ctx)
{
  int32_t cnt;
  
  const menu_func menu[]={[0]= menu_developer_test_rs232};

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_developer_test,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if(cnt == EXIT_PROGRAM )
    {
      return cnt;
    }
  }while(1);
}

int32_t menu_developer(p_shell_context_t ctx)
{
  int32_t cnt;
const menu_func menu[]={[0]= menu_developer_interrupt,
                             menu_developer_memory,
                             menu_developer_sensor,
                             menu_developer_sensor_config};
  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_developer,0,false);
    if(cnt == EXIT_BACK || cnt==EXIT_PROGRAM && cnt <= 0)
    {
      return cnt;
    }
    cnt--;
    cnt = menu[cnt](ctx);
    if(cnt == EXIT_PROGRAM )
    {
      return cnt;
    }
  }while(1);
}



const menuFunc_t menuFunc[]={{.title = "0.diplay",.func = menu_display},
                             {.title = "1.system",.func = menu_system},
                             {.title = "2.sensor",.func = menu_sensor},
                             {.title = "3.network",.func = menu_network},
                             {.title = "4.data",   .func = menu_data},
                             {.title = "5.display panel",.func = menu_display_panel},
                             {.title = "6.manage",.func = menu_manage},
                             {.title = "7.calibraion",.func = menu_calibration},
                             {.title = "8.developer", .func = menu_developer}};


int32_t print_menu_root(p_shell_context_t ctx)
{
  int i;
  for( i = 0; i<_countof(menuFunc);i++)
  {
    ctx->printf("%s\r\n",menuFunc[i].title);
  }
  return i;
}

 int32_t menu_root_(p_shell_context_t ctx, int32_t argc, char** argv)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(ctx,NULL,print_menu_root,0,false);
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt = menuFunc[cnt-1].func(ctx);
  }while(cnt != EXIT_PROGRAM);

  return cnt;
}



int32_t select_menu(select_menu_t *select_menu)
{
  int32_t cnt;

  do
  {
    cnt = select_indexFromList(select_menu->ctx,
                               select_menu->list,
                               select_menu->func,
                               select_menu->cnt,
                               select_menu->show);
    if(cnt == EXIT_BACK || cnt == EXIT_PROGRAM)
    {
      return cnt;
    }
    cnt = select_menu->menuFunc[cnt-1].func(select_menu->ctx);
  }while(cnt != EXIT_PROGRAM);

  return cnt;
}

 int32_t menu_root(p_shell_context_t ctx, int32_t argc, char** argv)
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