/*
¿Â∫Ò∞° ¡¶∞¯«œ¥¬ ºæº≠∏¶ ¡§¿«

*/
#include "app_sensor.h"
#include "config.h"
#include "utile.h"



//¡ˆø¯«œ¥¬ ºæº≠ ∏Ò∑œ ¡§¿«

const uint8_t temperatureList[]={S_T_UNSUED,
                                 S_T_ADC,
                                 S_T_GENERAL_232,
                                 S_T_GENERAL_485};
                                 
const uint8_t windDirectionList[]={S_T_UNSUED,
                                   S_T_ADC,
                                   S_T_GENERAL_232};

const uint8_t windSpeedList[]={S_T_UNSUED,
                               S_T_ADC,
                               S_T_GENERAL_232};


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
//±‚æ– 6
const uint8_t pressureList[]={S_T_UNSUED,
                              S_T_ADC,
                              S_T_GENERAL_232};

const uint8_t rainPresentList[]={S_T_UNSUED,
                                 S_T_RAIN_PRESENT_DI};

const uint8_t snowList[]={S_T_UNSUED,
                          S_T_ADC,
                          S_T_SNOW_HJ_485,
                          S_T_SNOW_HJ_232};

const uint8_t humiList[]={S_T_UNSUED,
                          S_T_ADC,
                          S_T_GENERAL_232};

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
{.list = defaultList,.cnt = sizeof(defaultList)},//N10_AIR_TEMPERATURE_50CM
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







                          
const char *sensorTypeList[]={"πÃªÁøÎ",         /* 0 S_T_UNSUED */
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
                                "GENERAL_RS232",    /* 13 S_T_GENERAL_232 */
                                "WIND_SPEED_RS485", /* 14 S_T_WIND_SPEED_485 */
                                "WIND_DIRECTION_RS485", /* 15 S_T_WIND_DIRECTION_485 */
                                "HUMI_HJ_RS485",      /* 16 S_T_HUMI_HJ_485 */
                                "WIND_SPEED_MAX",     /* 17 S_T_WIND_SPEED_MAX_VAL */
                                "WIND_DIRECTION_MAX", /* 18 S_T_WIND_DIRECTION_MAX_VAL */
                                "PRESSURE_RS485",     /* 19 S_T_PRESSURE_485 */
                                "HUMI_RS485",         /* 20 S_T_HUMI_RS485*/       
                                "RAIN_PRESENT_DI",    /* 21 S_T_RAIN_PRESENT_DI */
                                "SNOW_HJ_RS232",      /* 22 S_T_SNOW_HJ_232 */
                                "GENERAL_485"};       /* 23 S_T_GENERAL_485 */
                                


const char *sensorNameList[SENSOR_LIST_MAX]={ 
"±‚ø¬",//0
"«≥«‚",//1
"«≥º”",//2
"º¯∞£«≥«‚",//3
"º¯∞£«≥º”",//4
"∞≠ºˆ∑Æ",//5
"±‚æ–",//6
"∞≠ºˆ¿Øπ´",//7
"¿˚º≥",//8
"ªÛ¥ÎΩ¿µµ",//9
"∞≠ºˆ∑Æ(0.1mm)",//10
"¿œªÁ",//11
"¿œ¡∂",//12
"¡ˆ∏Èø¬µµ",//13
"√ ªÛø¬µµ",//14
"¡ˆ¡ﬂø¬µµ 5cm",//15
"¡ˆ¡ﬂø¬µµ 10cm",//16
"¡ˆ¡ﬂø¬µµ 20cm",//17
"¡ˆ¡ﬂø¬µµ 30cm",//18
"¡ˆ¡ﬂø¬µµ 50cm",//19
"¡ˆ¡ﬂø¬µµ 1.0m",//20
"¡ˆ¡ﬂø¬µµ 1.5m",//21
"¡ˆ¡ﬂø¬µµ 3.0m",//22
"¡ˆ¡ﬂø¬µµ 5.0",//23
"√˛øÓ∞Ì",//24
"2√˛øÓ∞Ì",//25
"3√˛øÓ∞Ì",//26
"øÓ∑Æ",//27
"Ω√¡§",//28
"PM10",//29
"PM2",//30
"º¯∫πªÁ",//31
"¿¸√µ∫πªÁ",//32
"π›ªÁ∫πªÁ",//33
"¡˜¥ﬁ",//34
"«ˆ¿Á¿œ±‚",//35
"≈‰æÁºˆ∫– 10cm",//36
"≈‰æÁºˆ∫– 20cm",//37
"≈‰æÁºˆ∫– 30cm",//38
"≈‰æÁºˆ∫– 50cm",//39
"¡∂µµ∑Æ",//40
"«≥º”(1.5m)",//41
"«≥º”(4.0m)",//42
"º¯∞£«≥º”(1.5m)",//43
"º¯∞£«≥º”(4.0m)",//44
"±‚ø¬ 0.5m",//45
"±‚ø¬ 4.0m",//46
"Ω¿µµ 0.5m",//47
"Ω¿µµ 4.0m",//48
"≈∏ƒ⁄πÃ≈Õ", //49
"ºˆ¿ß",     //50
"«•∏È ¿Øº”",//51
"¿Ø∑Æ",     //52 m©¯/s
"∞ÊªÁ1",    //53
"∞ÊªÁ1",    //54
"∞ÊªÁ2",    //55
"∞ÊªÁ3",    //56
"∞ÊªÁ4",    //57
"∞ÊªÁ5",    //58
"∞ÊªÁ6",    //59
"∞ÊªÁ7",    //60
"∞ÊªÁ8",    //61
"∞ÊªÁ9",    //62
"±‚∫ª"      //63  
};


const char *dataFmtList[SENSOR_LIST_MAX]={ 
"%-5.2fC",  //±‚ø¬ 0
"%-6.2f(0f(B      ",//«≥«‚      1
"%-5.2fm/s", //«≥º”      2
"%-6.2f(0f(B      ",//º¯∞£«≥«‚  3
"%-5.2fm/s", //º¯∞£«≥º”  4
"%-dmm",     //∞≠ºˆ∑Æ    5
"%-5.2fbar", //±‚æ–      6
"%-d",       //∞≠ºˆ¿Øπ´  7
"%-dmm",     //¿˚º≥      8
"%-5.2f",    //ªÛ¥ÎΩ¿µµ  9
"%-dmm",//∞≠ºˆ∑Æ(0.1mm)",//10
"%d",//¿œªÁ",//11
"%d",//"¿œ¡∂",//12
"%d",//"¡ˆ∏Èø¬µµ",//13
"%d",//"√ ªÛø¬µµ",//14
"%d",//"¡ˆ¡ﬂø¬µµ 5cm",//15
"%d",//"¡ˆ¡ﬂø¬µµ 10cm",//16
"%d",//"¡ˆ¡ﬂø¬µµ 20cm",//17
"%d",//"¡ˆ¡ﬂø¬µµ 30cm",//18
"%d",//"¡ˆ¡ﬂø¬µµ 50cm",//19
"%d",//"¡ˆ¡ﬂø¬µµ 1.0m",//20
"%d",//"¡ˆ¡ﬂø¬µµ 1.5m",//21
"%d",//"¡ˆ¡ﬂø¬µµ 3.0m",//22
"%d",//"¡ˆ¡ﬂø¬µµ 5.0",//23
"%d",//"√˛øÓ∞Ì",//24
"%d",//"2√˛øÓ∞Ì",//25
"%d",//"3√˛øÓ∞Ì",//26
"%d",//"øÓ∑Æ",//27
"%d",//"Ω√¡§",//28
"%d",//"PM10",//29
"%d",//"PM2",//30
"%d",//"º¯∫πªÁ",//31
"%d",//"¿¸√µ∫πªÁ",//32
"%d",//"π›ªÁ∫πªÁ",//33
"%d",//"¡˜¥ﬁ",//34
"%d",//"«ˆ¿Á¿œ±‚",//35
"%d",//"≈‰æÁºˆ∫– 10cm",//36
"%d",//"≈‰æÁºˆ∫– 20cm",//37
"%d",//"≈‰æÁºˆ∫– 30cm",//38
"%d",//"≈‰æÁºˆ∫– 50cm",//39
"%d",//"¡∂µµ∑Æ",//40
"%d",//"«≥º”(1.5m)",//41
"%d",//"«≥º”(4.0m)",//42
"%d",//"º¯∞£«≥º”(1.5m)",//43
"%d",//"º¯∞£«≥º”(4.0m)",//44
"%d",//"±‚ø¬ 0.5m",//45
"%d",//"±‚ø¬ 4.0m",//46
"%d",//"Ω¿µµ 0.5m",//47
"%d",//"Ω¿µµ 4.0m",//48
"%d",//"≈∏ƒ⁄πÃ≈Õ",//49
"%d",// "ªÁøÎ¿⁄ 1"//50
"%d",// "ªÁøÎ¿⁄ 2"//51
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

/**
 * @brief º≥¡§∞™ «“¥Á
 */
void * sensor_add(sensor_t *sensor)
{
  uint8_t index=0;

  switch (sensor->type)
  {
    case S_T_ADC:
    if(s_config.adc_cnt < _countof(s_config.adc))//«“¥Á ∞°¥…«—¡ˆ ∆«¥‹
    {
      index = s_config.adc_cnt;
      
      sensor->config[sensor->configCnt][0] = sensor->type;//«ÿ¥Á ≈∏¿‘¿ª √ﬂ∞°
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
    if(s_config.rs232_cnt < _countof(s_config.rs232))
    {
      index = s_config.rs232_cnt;

      sensor->config[sensor->configCnt][0] = sensor->type;//«ÿ¥Á ≈∏¿‘¿ª √ﬂ∞°
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
      case S_T_TEMP_485:
      case S_T_WIND_SPEED_485:
      case S_T_WIND_DIRECTION_485:
      case S_T_SNOW_HJ_485:
      case S_T_PRESSURE_485:
      case S_T_HUMI_RS485:
      case S_T_GENERAL_485:
      if(s_config.rs485_cnt < _countof(s_config.rs485))
      {
        index = s_config.rs485_cnt;
        sensor->config[sensor->configCnt][0] = sensor->type;//«ÿ¥Á ≈∏¿‘¿ª √ﬂ∞°
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



void * get_sensor_config(sensor_t *sensor)
{
  //¿˙¿Âµ» config¡§∫∏∞° æ¯¿∏∏È ª˝º∫º∫
  if(sensor->configCnt==0)
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
        return &s_config.rs232[sensor->config[i][1]];
        break;
        case S_T_TEMP_485:
        case S_T_WIND_SPEED_485:
        case S_T_WIND_DIRECTION_485:
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
  //«ÿ¥Á ºæº≠ ≈∏¿‘ config∞° º≥¡§µ«æÓ ¿÷¡ˆ æ ¿∏∏È √ﬂ∞° 
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

