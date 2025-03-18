/*
Àåºñ°¡ Á¦°øÇÏ´Â ¼¾¼­¸¦ Á¤ÀÇ

*/
#include <string.h>

#include "app_sensor.h"
#include "config.h"
#include "utile.h"
#include "Sensors\rain\rain.h"


//Áö¿øÇÏ´Â ¼¾¼­ ¸ñ·Ï Á¤ÀÇ

const uint8_t temperatureList[]={S_T_UNSUED,
                                 S_T_PT100_A,
                                 S_T_PT100_B,
                                 S_T_ADC};
                                 
const uint8_t windDirectionList[]={S_T_UNSUED,
                                   S_T_WIND_DIRECTION_HJ_485,
                                   S_T_ADC};

const uint8_t windSpeedList[]={S_T_UNSUED,
                               S_T_WIND_SPEED_HJ_485,
                               S_T_ADC};


const uint8_t windDirectionInstantList[]={S_T_UNSUED,
                                          S_T_WIND_DIRECTION_MAX_VAL};

const uint8_t windSpeedInstantList[]={S_T_UNSUED,
                                      S_T_WIND_SPEED_MAX_VAL};


const uint8_t rainList[]={S_T_UNSUED,
                          S_T_RAIN_REED_05MM,
                          S_T_RAIN_REED_1MM,
                          S_T_RAIN_HALL_05MM,
                          S_T_RAIN_HALL_1MM,
                          S_T_GENERAL_232};
//±â¾Ð 6
const uint8_t pressureList[]={S_T_UNSUED,
                              S_T_ADC};

const uint8_t rainPresentList[]={S_T_UNSUED,
                                 S_T_RAIN_PRESENT_DI};

const uint8_t snowList[]={S_T_UNSUED,
                          S_T_ADC,
                          S_T_SNOW_HJ_485};

const uint8_t humiList[]={S_T_UNSUED,
                          S_T_ADC};


const uint8_t sunShineList[]={S_T_UNSUED,
                              S_T_SUNSHINE,
                              S_T_ADC};

const uint8_t solarRadiationList[]={S_T_UNSUED,
                                    S_T_SOLAR_RADIATION,
                                    S_T_ADC};

const uint8_t soilTemp5cmList[]={S_T_UNSUED,
                                 S_T_SOIL_TEMP_5CM,
                                 S_T_ADC};

const uint8_t soilTemp10cmList[]={S_T_UNSUED,
                                 S_T_SOIL_TEMP_10CM,
                                 S_T_ADC};

const uint8_t soilTemp20cmList[]={S_T_UNSUED,
                                 S_T_SOIL_TEMP_20CM,
                                 S_T_ADC};
const uint8_t soilTemp30cmList[]={S_T_UNSUED,
                                 S_T_SOIL_TEMP_30CM,
                                 S_T_ADC};

const uint8_t soilTemp50cmList[]={S_T_UNSUED,
                                 S_T_SOIL_TEMP_50CM,
                                 S_T_ADC};

const uint8_t soilTemp100cmList[]={S_T_UNSUED,
                                   S_T_SOIL_TEMP_100CM,
                                   S_T_ADC};

const uint8_t soilTemp150cmList[]={S_T_UNSUED,
                                   S_T_SOIL_TEMP_150CM,
                                   S_T_ADC};

const uint8_t soilTemp300cmList[]={S_T_UNSUED,
                                   S_T_SOIL_TEMP_300CM,
                                   S_T_ADC};

const uint8_t soilTemp500cmList[]={S_T_UNSUED,
                                   S_T_SOIL_TEMP_500CM,
                                   S_T_ADC};


const uint8_t temperature50cmList[]={S_T_UNSUED,
                                     S_T_PT100_B};

const uint8_t defaultList[]={S_T_UNSUED,
                             S_T_ADC,
                             S_T_GENERAL_232,
                             S_T_GENERAL_485};



const supported_sensors_t supported_sensors[SENSOR_LIST_MAX]={{.list = temperatureList,.cnt =  sizeof(temperatureList)},      //A1_TEMPERATURE
{.list = windDirectionList,.cnt =  sizeof(windDirectionList)},   //A2_WIND_DIRECTION
{.list = windSpeedList,.cnt =  sizeof(windSpeedList)},           //A3_WIND_SPEED
{.list = windDirectionInstantList,.cnt =  sizeof(windDirectionInstantList)},   //A4_INSTANT_WIND_DIRECTION
{.list = windSpeedInstantList,.cnt =  sizeof(windSpeedInstantList)},//A5_INSTANT_WIND_SPEED
{.list = rainList,.cnt =  sizeof(rainList)},                       //A6_RAINFALL_DOT5_1MM
{.list = pressureList,.cnt =  sizeof(pressureList)},//A7_PRESSURE
{.list = rainPresentList,.cnt =  sizeof(rainPresentList)},//A8_RAIN_PRESENT
{.list = snowList,.cnt    =  sizeof(snowList)},//A9_SNOW_DEPTH
{.list = humiList,.cnt    = sizeof(humiList)},//A10_RELATIVE_HUMIDITY
{.list = defaultList,.cnt = sizeof(defaultList)},//A11_RAINFALL_DOT1MM
{.list = defaultList,.cnt = sizeof(defaultList)},//B1_SOLAR_RADIATION
{.list = defaultList,.cnt = sizeof(defaultList)},//B2_SUNSHINE_DURATION
{.list = defaultList,.cnt = sizeof(defaultList)},//B3_GROUND_TEMPERATURE
{.list = defaultList,.cnt = sizeof(defaultList)},//B4_SURFACE_TEMPERATURE
{.list = defaultList,.cnt = sizeof(defaultList)},//B5_SOIL_TEMPERATURE_5CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B6_SOIL_TEMPERATURE_10CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B7_SOIL_TEMPERATURE_20CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B8_SOIL_TEMPERATURE_30CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B9_SOIL_TEMPERATURE_50CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B10_SOIL_TEMPERATURE_100CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B11_SOIL_TEMPERATURE_150CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B12_SOIL_TEMPERATURE_300CM
{.list = defaultList,.cnt = sizeof(defaultList)},//B13_SOIL_TEMPERATURE_500CM
{.list = defaultList,.cnt = sizeof(defaultList)},//C1_CLOUD_BASE1
{.list = defaultList,.cnt = sizeof(defaultList)},//C2_CLOUD_BASE2
{.list = defaultList,.cnt = sizeof(defaultList)},//C3_CLOUD_BASE3
{.list = defaultList,.cnt = sizeof(defaultList)},//C4_CLOUD_COVER
{.list = defaultList,.cnt = sizeof(defaultList)},//C5_VISIBILITY
{.list = defaultList,.cnt = sizeof(defaultList)},//C6_PM10
{.list = defaultList,.cnt = sizeof(defaultList)},//C7_PM2DOT5
{.list = defaultList,.cnt = sizeof(defaultList)},//C8_NET_RADIATION
{.list = defaultList,.cnt = sizeof(defaultList)},//C9_TOTAL_RADIATION
{.list = defaultList,.cnt = sizeof(defaultList)},//C10_REFLECTED_RADIATION
{.list = defaultList,.cnt = sizeof(defaultList)},//C11_DIRECT_SOLAR
{.list = defaultList,.cnt = sizeof(defaultList)},//C12_CURRENT_WEATHER
{.list = defaultList,.cnt = sizeof(defaultList)},//N1_SOIL_MOISTURE_10CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N2_SOIL_MOISTURE_20CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N3_SOIL_MOISTURE_30CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N4_SOIL_MOISTURE_50CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N5_ILLUMINANCE
{.list = defaultList,.cnt = sizeof(defaultList)},//N6_WIND_VELOCITY_150CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N7_WIND_VELOCITY_400CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N8_INSTANT_VELOCITY_150CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N9_INSTANT_VELOCITY_400CM
{.list = temperature50cmList,.cnt = sizeof(temperature50cmList)},//N10_AIR_TEMPERATURE_50CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N11_AIR_TEMPERATURE_400CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N12_HUMIDITY_50CM
{.list = defaultList,.cnt = sizeof(defaultList)},//N13_HUMIDITY_400CM
{.list = defaultList,.cnt = sizeof(defaultList)},//I1_TACHOMETER
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_WATER
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SWV
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_FLOW_RATE
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_1
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_2
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_3
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_4
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_5
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_6
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_7
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_8
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_9
{.list = defaultList,.cnt = sizeof(defaultList)},//USER_SLOPE_10
{.list = defaultList,.cnt = sizeof(defaultList)}};//USER_DEFAULT


                          
const char *sensorTypeList[]={"¹Ì»ç¿ë",              /* 0 S_T_UNSUED */
                                "ADC",               /* 1 S_T_ADC */
                                "RS232",             /* 2 S_T_TEMP_232 */
                                "RS485",             /* 3 S_T_TEMP_485 */
                                "MODBUS",            /* 4 S_T_MODBUS */
                                "HART",              /* 5 S_T_HART */
                                "FREQ_0",            /* 6 S_T_FREQ_0*/
                                "REED 0.5mm",        /* 7 S_T_RAIN_REED_05MM */
                                "REED 1mm",          /* 8 S_T_RAIN_REED_1MM */
                                "HALL 0.5mm",        /* 9 S_T_RAIN_HALL_05MM */
                                "HALL 1mm",          /* 10 S_T_RAIN_HALL_1MM */
                                "DI_0",              /* 11 S_T_DI_0 */
                                "SNOW_HJ_RS485",     /* 12 S_T_SNOW_HJ_485 */
                                "GENERAL_RS232",     /* 13 S_T_GENERAL_232 */
                                "WIND_SPEED_HJ_RS485",  /* 14 S_T_WIND_SPEED_HJ_485 */
                                "WIND_DIRECTION_HJ_RS485", /* 15 S_T_WIND_DIRECTION_HJ_485 */
                                "HUMI_HJ_RS485",      /* 16 S_T_HUMI_HJ_485 */
                                "WIND_SPEED_MAX",     /* 17 S_T_WIND_SPEED_MAX_VAL */
                                "WIND_DIRECTION_MAX", /* 18 S_T_WIND_DIRECTION_MAX_VAL */
                                "PRESSURE_RS485",     /* 19 S_T_PRESSURE_485 */
                                "HUMI_RS485",         /* 20 S_T_HUMI_RS485*/       
                                "RAIN_PRESENT_DI",    /* 21 S_T_RAIN_PRESENT_DI */
                                "SNOW_HJ_RS232",      /* 22 S_T_SNOW_HJ_232 */
                                "GENERAL_485",        /* 23 S_T_GENERAL_485 */
                                "PT100_A",            /* 24 S_T_PT100_A */
                                "PT100_B",            /* 25 S_T_PT100_B */
                                "FREQ_A",             /* 26 S_T_FREQ_A */
                                "FREQ_B",             /* 27 S_T_FREQ_B */
                                "SUNSHINE",           /* 28 S_T_SUNSHINE */
                                "SOLAR_RADIATION",    /* 29 S_T_SOLAR_RADIATION */
                                "S_T_SOIL_TEMP_5CM",  /* 30 S_T_SOIL_TEMP_5CM */
                                "S_T_SOIL_TEMP_10CM", /* 31 S_T_SOIL_TEMP_10CM */
                                "S_T_SOIL_TEMP_20CM", /* 32 S_T_SOIL_TEMP_20CM */
                                "S_T_SOIL_TEMP_30CM", /* 33 S_T_SOIL_TEMP_30CM */
                                "S_T_SOIL_TEMP_50CM", /* 34 S_T_SOIL_TEMP_50CM */
                                "S_T_SOIL_TEMP_100CM",/* 35 S_T_SOIL_TEMP_100CM */
                                "S_T_SOIL_TEMP_150CM",/* 36 S_T_SOIL_TEMP_150CM */
                                "S_T_SOIL_TEMP_300CM",/* 37 S_T_SOIL_TEMP_300CM */
                                "S_T_SOIL_TEMP_500CM",/* 38 S_T_SOIL_TEMP_500CM */
                                "S_T_GENERAL"};       /* 39 S_T_GENERAL */

        


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
"±âº»"      //63  
};


const char *dataFmtList[SENSOR_LIST_MAX]={ 
"%-5.2fC",  //±â¿Â 0
"%-6.2f(0f(B      ",//Ç³Çâ      1
"%-5.2fm/s", //Ç³¼Ó      2
"%-6.2f(0f(B      ",//¼ø°£Ç³Çâ  3
"%-5.2fm/s", //¼ø°£Ç³¼Ó  4
"%-dmm",     //°­¼ö·®    5
"%-5.2fbar", //±â¾Ð      6
"%-d",       //°­¼öÀ¯¹«  7
"%-dmm",     //Àû¼³      8
"%-5.2f",    //»ó´ë½Àµµ  9
"%-dmm",//°­¼ö·®(0.1mm)",//10
"%-5.2f",//ÀÏ»ç",//11
"%-5.2f",//"ÀÏÁ¶",//12
"%-5.2f",//"Áö¸é¿Âµµ",//13
"%-5.2f",//"ÃÊ»ó¿Âµµ",//14
"%-5.2f",//"ÁöÁß¿Âµµ 5cm",//15
"%-5.2f",//"ÁöÁß¿Âµµ 10cm",//16
"%-5.2f",//"ÁöÁß¿Âµµ 20cm",//17
"%-5.2f",//"ÁöÁß¿Âµµ 30cm",//18
"%-5.2f",//"ÁöÁß¿Âµµ 50cm",//19
"%-5.2f",//"ÁöÁß¿Âµµ 1.0m",//20
"%-5.2f",//"ÁöÁß¿Âµµ 1.5m",//21
"%-5.2f",//"ÁöÁß¿Âµµ 3.0m",//22
"%-5.2f",//"ÁöÁß¿Âµµ 5.0",//23
"%-5.2f",//"Ãþ¿î°í",//24
"%-5.2f",//"2Ãþ¿î°í",//25
"%-5.2f",//"3Ãþ¿î°í",//26
"%-5.2f",//"¿î·®",//27
"%-5.2f",//"½ÃÁ¤",//28
"%-5.2f",//"PM10",//29
"%-5.2f",//"PM2",//30
"%-5.2f",//"¼øº¹»ç",//31
"%-5.2f",//"ÀüÃµº¹»ç",//32
"%-5.2f",//"¹Ý»çº¹»ç",//33
"%-5.2f",//"Á÷´Þ",//34
"%-5.2f",//"ÇöÀçÀÏ±â",//35
"%-5.2f",//"Åä¾ç¼öºÐ 10cm",//36
"%-5.2f",//"Åä¾ç¼öºÐ 20cm",//37
"%-5.2f",//"Åä¾ç¼öºÐ 30cm",//38
"%-5.2f",//"Åä¾ç¼öºÐ 50cm",//39
"%-5.2f",//"Á¶µµ·®",//40
"%-5.2f",//"Ç³¼Ó(1.5m)",//41
"%-5.2f",//"Ç³¼Ó(4.0m)",//42
"%-5.2f",//"¼ø°£Ç³¼Ó(1.5m)",//43
"%-5.2f",//"¼ø°£Ç³¼Ó(4.0m)",//44
"%-5.2fC",//"±â¿Â 0.5m",//45
"%-5.2f",//"±â¿Â 4.0m",//46
"%-5.2f",//"½Àµµ 0.5m",//47
"%-5.2f",//"½Àµµ 4.0m",//48
"%-5.2f",//"Å¸ÄÚ¹ÌÅÍ",//49
"%-5.2f",// "»ç¿ëÀÚ 1"//50
"%-5.2f",// "»ç¿ëÀÚ 2"//51
"%-5.2f",//52
"%-5.2f",//53
"%-5.2f",//54
"%-5.2f",//55
"%-5.2f",//56
"%-5.2f",//57
"%-5.2f",//58
"%-5.2f",//59
"%-5.2f",//60
"%-5.2f",//61
"%-5.2f",//62
"%-5.2f" //63
};

config_manager_t s_config;

sensor_data_t sensor_data_real[SENSOR_LIST_MAX]; // ½Ç½Ã°£
sensor_data_t sensor_data[SENSOR_LIST_MAX];     // ½Ç½Ã°£ ÀÚ·á ¿¬»ê¿ë
sensor_data_t sensor_data_1s[SENSOR_LIST_MAX];  // 1ÃÊ ¸¶´Ù °»½ÅµÇ´Â ½Ç½Ã°£ ÀÚ·á
sensor_data_t sensor_data_1min[SENSOR_LIST_MAX];// 1ºÐ ¸¶´Ù °»½ÅµÇ´Â ½Ç½Ã°£ ÀÚ·á


sensor_emul_t g_sensor_emul[SENSOR_LIST_MAX];



uint8_t sensorData_updated=0;

bool wait_sensorComplete(void)
{
  return true;
}

//1ÃÊ ÀÚ·á¸¦ ¾÷µ¥ÀÌÆ®, ½Ç½Ã°£ °ª ¿äÃ»½Ã ÀÌ °ª Àü¼Û
void update_sensorData1s(void)
{
  //¼¼¸¶Æ÷¾î pend ¿ÏÀüÈ÷ ÇÑ¹ø¿¡ ¾÷µ¥ÀÌÆ®µÈ ÀÚ·á¸¸ ÀÐµµ·Ï
  memcpy(sensor_data_1s,sensor_data,sizeof(sensor_data_1s));
 //¼¼¸¶ Æ÷¾î post
}

//1ºÐ ÀÚ·á¸¦ ¾÷µ¥ÀÌÆ®, 1ºÐ ÀÚ·á ¿äÃ»½Ã ÀÌ °ª Àü¼Û
void update_sensorData1min(void)
{
  //¼¼¸¶Æ÷¾î pend ¿ÏÀüÈ÷ ÇÑ¹ø¿¡ ¾÷µ¥ÀÌÆ®µÈ ÀÚ·á¸¸ ÀÐµµ·Ï
  memcpy(sensor_data_1min,sensor_data,sizeof(sensor_data_1min));
 //¼¼¸¶ Æ÷¾î post
}


void sensorData_init(void)
{
  for(int i = 0 ; i <_countof(sensor_data);i++)
  {
    switch(i)
    {
      case A1_TEMPERATURE:
      sensor_data[A1_TEMPERATURE].dataType = DATA_TYPE_F;
      sensor_data_1s[A1_TEMPERATURE].dataType = DATA_TYPE_F;
      break;
      case A2_WIND_DIRECTION:
      sensor_data[A2_WIND_DIRECTION].dataType = DATA_TYPE_F;
      sensor_data_1s[A2_WIND_DIRECTION].dataType = DATA_TYPE_F;
      break;
      case A6_RAINFALL_DOT5_1MM:
      sensor_data[A6_RAINFALL_DOT5_1MM].dataType = DATA_TYPE_I;
      sensor_data[A6_RAINFALL_DOT5_1MM].opt = &rain_data;
      sensor_data_1s[A6_RAINFALL_DOT5_1MM].dataType = DATA_TYPE_I;
      break;
      case A9_SNOW_DEPTH:
      sensor_data[A9_SNOW_DEPTH].dataType = DATA_TYPE_I;
      sensor_data[A9_SNOW_DEPTH].opt = &rain_data;
      sensor_data_1s[A9_SNOW_DEPTH].dataType = DATA_TYPE_I;
      break;

      default:
      sensor_data[i].dataType = DATA_TYPE_F;
      sensor_data_1s[i].dataType = DATA_TYPE_F;
      break;
    }

  }

}

/**
 * @brief ¼³Á¤°ª ÇÒ´ç
 */
void * sensor_add(sensor_t *sensor)
{
  uint8_t index=0;

  switch (sensor->type)
  {
    case S_T_ADC:
    if(s_config.adc_cnt < _countof(s_config.adc))//ÇÒ´ç °¡´ÉÇÑÁö ÆÇ´Ü
    {
      index = s_config.adc_cnt;
      
      sensor->config[sensor->configCnt][0] = sensor->type;//ÇØ´ç Å¸ÀÔÀ» Ãß°¡
      sensor->config[sensor->configCnt][1] = index;
      
      WRITE_CFG_MEM(&sensor->config[sensor->configCnt],sizeof(sensor->config[sensor->configCnt]));
      sensor->configCnt++;
      WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));

      s_config.adc_cnt++;
      WRITE_S_CFG(adc_cnt);

      return &s_config.adc[index];
    }
    case S_T_TEMP_232:
    case S_T_GENERAL_232:
    case S_T_SNOW_HJ_232:
    case S_T_HART:
    if(s_config.rs232_cnt < _countof(s_config.rs232))
    {
      index = s_config.rs232_cnt;

      sensor->config[sensor->configCnt][0] = sensor->type;//ÇØ´ç Å¸ÀÔÀ» Ãß°¡
      sensor->config[sensor->configCnt][1] = index;

      WRITE_CFG_MEM(&sensor->config[sensor->configCnt],sizeof(sensor->config[sensor->configCnt]));
      sensor->configCnt++;
      WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));


      s_config.rs232_cnt++;

      WRITE_S_CFG(rs232_cnt);
      return &s_config.rs232[index];
    }
      return 0;
      break;
      case S_T_WIND_DIRECTION_HJ_485:
      case S_T_WIND_SPEED_HJ_485:
      case S_T_TEMP_485:
      case S_T_SNOW_HJ_485:
      case S_T_PRESSURE_485:
      case S_T_HUMI_RS485:
      case S_T_GENERAL_485:
      if(s_config.rs485_cnt < _countof(s_config.rs485))
      {
        index = s_config.rs485_cnt;
        sensor->config[sensor->configCnt][0] = sensor->type;//ÇØ´ç Å¸ÀÔÀ» Ãß°¡
        sensor->config[sensor->configCnt][1] = index;
        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt,sizeof(sensor->configCnt));
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


/**
 * @brief ¼¾¼­Å¸ÀÔ¿¡ ¸Â´Â ¼³Á¤°ªÀ» °¡Á®¿È
 */
void * get_sensor_config(sensor_t *sensor)
{
  //configCnt°¡ 0ÀÌ¶õ°Ç ¾ÆÁ÷ ÀúÀåµÈ config°¡ ¾ø´Ù´Â°Í
  if(sensor->configCnt == 0)
  {
    return 0;
  }

  
  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == sensor->type)
    {
      switch(sensor->type)
      {
        case S_T_ADC:
        return &s_config.adc[sensor->config[i][1]];
        break;
        case S_T_TEMP_232:
        case S_T_GENERAL_232:
        case S_T_SNOW_HJ_232:
        case S_T_HART:
        return &s_config.rs232[sensor->config[i][1]];
        break;
        case S_T_TEMP_485:
        case S_T_WIND_SPEED_HJ_485:
        case S_T_WIND_DIRECTION_HJ_485:
        case S_T_SNOW_HJ_485:
        case S_T_PRESSURE_485:
        case S_T_HUMI_RS485:
        case S_T_GENERAL_485:
        return &s_config.rs485[sensor->config[i][1]];
        break;
        case S_T_MODBUS:
        return &s_config.modbus[sensor->config[i][1]];
        break;
      }
    }
  }
  //ÇØ´ç ¼¾¼­ Å¸ÀÔ config°¡ ¼³Á¤µÇ¾î ÀÖÁö ¾ÊÀ¸¸é Ãß°¡ 
  return 0;
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

