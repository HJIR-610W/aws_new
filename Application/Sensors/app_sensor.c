/*
Àåºñ°¡ Á¦°øÇÏ´Â ¼¾¼­¸¦ Á¤ÀÇ

*/
#include "app_sensor.h"
#include "config.h"
#include "utile.h"



//Áö¿øÇÏ´Â ¼¾¼­ ¸ñ·Ï Á¤ÀÇ

const uint8_t temperatureList[]={S_T_UNSUED,
                                 S_T_ADC,
                                 S_T_TEMP_232,
                                 S_T_TEMP_485};
                                 
const uint8_t windDirectionList[]={S_T_UNSUED,
                                   S_T_ADC,
                                   S_T_WIND_SPEED_485};

const uint8_t windSpeedList[]={S_T_UNSUED,
                               S_T_ADC,
                               S_T_WIND_SPEED_485};


const uint8_t windDirectionInstantList[]={S_T_UNSUED,
                                          S_T_WIND_DIRECTION_MAX_VAL};

const uint8_t windSpeedInstantList[]={S_T_UNSUED,
                                      S_T_WIND_SPEED_MAX_VAL};



const uint8_t rainList[]={S_T_UNSUED,
                          S_T_RAIN_REED_05MM,
                          S_T_RAIN_REED_1MM,
                          S_T_RAIN_HALL_05MM,
                          S_T_RAIN_HALL_1MM,
                          S_T_RAIN_SERIAL_232};
//±â¾Ð 6
const uint8_t pressureList[]={S_T_UNSUED,
                              S_T_ADC,
                              S_T_PRESSURE_485};

const uint8_t snowList[]={S_T_UNSUED,
                          S_T_ADC,
                          S_T_SNOW_HJ_485,
                          S_T_SNOW_HJ_232};

const uint8_t rainPresentList[]={S_T_UNSUED,
                                 S_T_RAIN_PRESENT_DI};

const uint8_t humiList[]={S_T_UNSUED,
                          S_T_ADC,
                          S_T_HUMI_HJ_485};

                          
const char *sensorTypeList[]={"¹Ì»ç¿ë",         /* 0 S_T_UNSUED */
                                "ADC",            /* 1 S_T_ADC */
                                "RS232",          /* 2 S_T_TEMP_232 */
                                "RS485",          /* 3 S_T_TEMP_485 */
                                "MODBUS",         /* 4 S_T_MODBUS */
                                "HART",           /* 5 S_T_HART */
                                "FREQ_0",         /* 6 S_T_FREQ_0*/
                                "REED 0.5mm",     /* 7 S_T_RAIN_REED_05MM */
                                "REED 1mm",       /* 8 S_T_RAIN_REED_1MM */
                                "HALL 0.5mm",     /* 9 S_T_RAIN_HALL_05MM */
                                "HALL 1mm",       /* 10 S_T_RAIN_HALL_1MM */
                                "DI_0",           /* 11 S_T_DI_0 */
                                "SNOW_HJ_RS485",  /* 12 S_T_SNOW_HJ_485 */
                                "RAIN_RS232",    /* 13 S_T_RAIN_SERIAL_232 */
                                "WIND_SPEED_RS485", /* 14 S_T_WIND_SPEED_485 */
                                "WIND_DIRECTION_RS485", /* 15 S_T_WIND_DIRECTION_485 */
                                "HUMI_HJ_RS485",      /* 16 S_T_HUMI_HJ_485 */
                                "WIND_SPEED_MAX",     /* 17 S_T_WIND_SPEED_MAX_VAL */
                                "WIND_DIRECTION_MAX", /* 18 S_T_WIND_DIRECTION_MAX_VAL */
                                "PRESSURE_RS485",     /* 19 S_T_PRESSURE_485 */
                                "HUMI_RS485",         /* 20 S_T_HUMI_RS485*/       
                                "RAIN_PRESENT_DI",    /* 21 S_T_RAIN_PRESENT_DI */
                                "SNOW_HJ_RS232"};     /* 22 S_T_SNOW_HJ_232*/
                                


const char *sensorNameList[SENSOR_LIST_MAX]={ 
"±â¿Â",//0
"Ç³Çâ",//1
"Ç³¼Ó",//2
"¼ø°£Ç³Çâ",//3
"¼ø°£Ç³¼Ó",//4
"°­¼ö·®",//5
"±â¾Ð",//6
"°­¼öÀ¯¹«",//7
"Àû¼³",//8
"»ó´ë½Àµµ",//9
"°­¼ö·®(0.1mm)",//10
"ÀÏ»ç",//11
"ÀÏÁ¶",//12
"Áö¸é¿Âµµ",//13
"ÃÊ»ó¿Âµµ",//14
"ÁöÁß¿Âµµ 5cm",//15
"ÁöÁß¿Âµµ 10cm",//16
"ÁöÁß¿Âµµ 20cm",//17
"ÁöÁß¿Âµµ 30cm",//18
"ÁöÁß¿Âµµ 50cm",//19
"ÁöÁß¿Âµµ 1.0m",//20
"ÁöÁß¿Âµµ 1.5m",//21
"ÁöÁß¿Âµµ 3.0m",//22
"ÁöÁß¿Âµµ 5.0",//23
"Ãþ¿î°í",//24
"2Ãþ¿î°í",//25
"3Ãþ¿î°í",//26
"¿î·®",//27
"½ÃÁ¤",//28
"PM10",//29
"PM2",//30
"¼øº¹»ç",//31
"ÀüÃµº¹»ç",//32
"¹Ý»çº¹»ç",//33
"Á÷´Þ",//34
"ÇöÀçÀÏ±â",//35
"Åä¾ç¼öºÐ 10cm",//36
"Åä¾ç¼öºÐ 20cm",//37
"Åä¾ç¼öºÐ 30cm",//38
"Åä¾ç¼öºÐ 50cm",//39
"Á¶µµ·®",//40
"Ç³¼Ó(1.5m)",//41
"Ç³¼Ó(4.0m)",//42
"¼ø°£Ç³¼Ó(1.5m)",//43
"¼ø°£Ç³¼Ó(4.0m)",//44
"±â¿Â 0.5m",//45
"±â¿Â 4.0m",//46
"½Àµµ 0.5m",//47
"½Àµµ 4.0m",//48
"Å¸ÄÚ¹ÌÅÍ", //49
"¼öÀ§",     //50
"Ç¥¸é À¯¼Ó",//51
"À¯·®",     //52 m©ø/s
"°æ»ç1",    //53
"°æ»ç1",    //54
"°æ»ç2",    //55
"°æ»ç3",    //56
"°æ»ç4",    //57
"°æ»ç5",    //58
"°æ»ç6",    //59
"°æ»ç7",    //60
"°æ»ç8",    //61
"°æ»ç9",    //62
"ÇÔ¼öºñ1"   //63  
};


const char *dataFmtList[SENSOR_LIST_MAX]={ 
"%-5.2fC",  //±â¿Â 0
"%-6.2f(0f(B",//Ç³Çâ      1
"%-5.2fm/s", //Ç³¼Ó      2
"%-6.2f(0f(B",//¼ø°£Ç³Çâ  3
"%-5.2fm/s", //¼ø°£Ç³¼Ó  4
"%-dmm",     //°­¼ö·®    5
"%-5.2fbar", //±â¾Ð      6
"%-d",       //°­¼öÀ¯¹«  7
"%-dmm",     //Àû¼³      8
"%-5.2f",    //»ó´ë½Àµµ  9
"%-dmm",//°­¼ö·®(0.1mm)",//10
"%d",//ÀÏ»ç",//11
"%d",//"ÀÏÁ¶",//12
"%d",//"Áö¸é¿Âµµ",//13
"%d",//"ÃÊ»ó¿Âµµ",//14
"%d",//"ÁöÁß¿Âµµ 5cm",//15
"%d",//"ÁöÁß¿Âµµ 10cm",//16
"%d",//"ÁöÁß¿Âµµ 20cm",//17
"%d",//"ÁöÁß¿Âµµ 30cm",//18
"%d",//"ÁöÁß¿Âµµ 50cm",//19
"%d",//"ÁöÁß¿Âµµ 1.0m",//20
"%d",//"ÁöÁß¿Âµµ 1.5m",//21
"%d",//"ÁöÁß¿Âµµ 3.0m",//22
"%d",//"ÁöÁß¿Âµµ 5.0",//23
"%d",//"Ãþ¿î°í",//24
"%d",//"2Ãþ¿î°í",//25
"%d",//"3Ãþ¿î°í",//26
"%d",//"¿î·®",//27
"%d",//"½ÃÁ¤",//28
"%d",//"PM10",//29
"%d",//"PM2",//30
"%d",//"¼øº¹»ç",//31
"%d",//"ÀüÃµº¹»ç",//32
"%d",//"¹Ý»çº¹»ç",//33
"%d",//"Á÷´Þ",//34
"%d",//"ÇöÀçÀÏ±â",//35
"%d",//"Åä¾ç¼öºÐ 10cm",//36
"%d",//"Åä¾ç¼öºÐ 20cm",//37
"%d",//"Åä¾ç¼öºÐ 30cm",//38
"%d",//"Åä¾ç¼öºÐ 50cm",//39
"%d",//"Á¶µµ·®",//40
"%d",//"Ç³¼Ó(1.5m)",//41
"%d",//"Ç³¼Ó(4.0m)",//42
"%d",//"¼ø°£Ç³¼Ó(1.5m)",//43
"%d",//"¼ø°£Ç³¼Ó(4.0m)",//44
"%d",//"±â¿Â 0.5m",//45
"%d",//"±â¿Â 4.0m",//46
"%d",//"½Àµµ 0.5m",//47
"%d",//"½Àµµ 4.0m",//48
"%d",//"Å¸ÄÚ¹ÌÅÍ",//49
"%d",// "»ç¿ëÀÚ 1"//50
"%d",// "»ç¿ëÀÚ 2"//51
"%d",//52
"%d",//53
"%d",//54
"%d",//55
"%d",//56
"%d",//57
"%d",//58
"%d",//59
"%d",//60
"%d",//61
"%d",//62
"%d" //63
};

config_manager_t s_config;



sensor_data_t sensor_data[SENSOR_LIST_MAX];
sensor_emul_t g_sensor_emul[SENSOR_LIST_MAX];

void sensorData_init(void)
{
  sensor_data[A1_TEMPERATURE].dataType = DATA_TYPE_F;
  sensor_data[A2_WIND_DIRECTION].dataType = DATA_TYPE_F;
}

void * sensor_add(sensor_t *sensor,uint8_t sensorType)
{
  uint8_t index=0;

  switch (sensorType)
  {
    case S_T_ADC:
    if(s_config.adc_cnt < _countof(s_config.adc))//ÇÒ´ç °¡´ÉÇÑÁö ÆÇ´Ü
    {
      sensor->config[sensor->configCnt][0] = sensorType;//ÇØ´ç Å¸ÀÔÀ» Ãß°¡

      WRITE_CFG_MEM(&sensor->config[sensor->configCnt][0],sizeof(sensor->config[sensor->configCnt][0]));
      sensor->configCnt++;
      WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));

      index = s_config.adc_cnt;
      s_config.adc_cnt++;
      WRITE_S_CFG(adc_cnt);
      return &s_config.adc[index];
    }
    return 0;
    case S_T_TEMP_232:
    case S_T_RAIN_SERIAL_232:
    case S_T_SNOW_HJ_232:
    if(s_config.rs232_cnt < _countof(s_config.rs232))
    {
      index = s_config.rs232_cnt;

      sensor->config[sensor->configCnt][0] = sensorType;//ÇØ´ç Å¸ÀÔÀ» Ãß°¡
      sensor->config[sensor->configCnt][1] = index;

      WRITE_CFG_MEM(&sensor->config[sensor->configCnt][0],sizeof(sensor->config[sensor->configCnt][0]));
      sensor->configCnt++;
      WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));


      s_config.rs232_cnt++;

      WRITE_S_CFG(rs232_cnt);
      return &s_config.rs232[index];
    }
      return 0;
      break;
      case S_T_TEMP_485:
      case S_T_WIND_SPEED_485:
      case S_T_WIND_DIRECTION_485:
      case S_T_SNOW_HJ_485:
      case S_T_PRESSURE_485:
      case S_T_HUMI_RS485:
      if(s_config.rs485_cnt < _countof(s_config.rs485))
      {
        sensor->config[sensor->configCnt][0] = sensorType;//ÇØ´ç Å¸ÀÔÀ» Ãß°¡

        WRITE_CFG_MEM(&sensor->config[sensor->configCnt][0],sizeof(sensor->config[sensor->configCnt][0]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));

        index = s_config.rs485_cnt;
        s_config.rs485_cnt++;
        WRITE_S_CFG(rs485_cnt);
        return &s_config.rs485[index];
      }
      return 0;
      break;
      break;
    
    default:
      break;
    }

    return 0;
}

void * get_sensor_config(sensor_t *sensor,uint8_t sensorType)
{


  //ÀúÀåµÈ configÁ¤º¸°¡ ¾øÀ¸¸é »ý¼º¼º
  if(sensor->configCnt==0)
  {
    return sensor_add(sensor,sensorType);
  }

  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == sensorType)
    {
      switch(sensorType)
      {
        case S_T_ADC:
        return &s_config.adc[sensor->config[i][1]];
        break;
        case S_T_TEMP_232:
        case S_T_RAIN_SERIAL_232:
        case S_T_SNOW_HJ_232:
        return &s_config.rs232[sensor->config[i][1]];
        break;
        case S_T_TEMP_485:
        case S_T_WIND_SPEED_485:
        case S_T_WIND_DIRECTION_485:
        case S_T_SNOW_HJ_485:
        case S_T_PRESSURE_485:
        case S_T_HUMI_RS485:
        return &s_config.rs485[sensor->config[i][1]];
        break;
        case S_T_MODBUS:
        return &s_config.modbus[sensor->config[i][1]];
        break;
      }
    }
  }
  //ÇØ´ç ¼¾¼­ Å¸ÀÔ config°¡ ¼³Á¤µÇ¾î ÀÖÁö ¾ÊÀ¸¸é Ãß°¡ 
  return sensor_add(sensor,sensorType);
}


void add_sensor_config(sensor_t *sensor,void *config,uint8_t sensorType)
{
  uint8_t cnt=0;

  for(int i = 0 ; i< _countof(sensor->config);i++)
  {
    if(sensor->config[i][0] == sensorType)
    {
      switch(sensorType)
      {
        case S_T_ADC:
        s_config.adc[sensor->config[i][1]] = *(adc_config_t *)config;
        break;
      }

    }
  }

  if(cnt==0)
  {
    switch (sensorType)
    {
      case S_T_ADC:
      sensor->config[sensor->configCnt++][0] = S_T_ADC;
      s_config.adc[s_config.adc_cnt++] = *(adc_config_t *)config;
      break;
    
    default:
      break;
    }
  }
}

