
/*
센서정보는 2가지 구조체를 사용한다.

1. 센서의 속성 구조체 모음
2. 센서 속성 구조체를 가르키는 인덱스

센서마다 고유의 속성을 구조체로 구현하면 새로운 모델이 추가되면 구조체길이가 변경되어
값이 틀어진다.
config에서는 속성정보의 index만 관리한다
속성 구조체는 센서가 추가되면 최후에 추가되기때문에 틀어질 일이 없다.

*/
#include "app_sensor.h"

#include <string.h>

#include "config_app.h"
#include "util_memory.h"

const char *g_sensor_model_table[] = {
#define X(name, format) format,
    SENSOR_TYPE_LIST
#undef X
};

const char *g_sensor_model_eng_table[] = {
#define X(name, format) format,
    SENSOR_TYPE_ENG_LIST
#undef X
};
  

const char *sensor_name_list[] = {
#define X(name, name2, format) name2,
    SENSOR_LIST
#undef X
};

const char *sensor_name_eng_list[] = {
#define X(name, name2, format) name2,
    SENSOR_ENG_LIST
#undef X
};


const char *sensor_format_list[] = {
#define X(name, name2, format) format,
    SENSOR_LIST
#undef X
};

// 지원하는 센서 목록 정의
const uint8_t temperatureList[] = {S_T_UNSUED, S_T_TEMPERATURE_HJ, S_T_PT100_A, S_T_PT100_B};
const uint8_t windDirectionList[] = {S_T_UNSUED, S_T_WIND_DIRECTION_HJ_485, S_T_ADC};
const uint8_t windSpeedList[] = {S_T_UNSUED, S_T_WIND_SPEED_HJ_485, S_T_FREQ, S_T_ADC};
const uint8_t rainList[] = {S_T_UNSUED,         S_T_RAIN_REED_05MM, S_T_RAIN_REED_1MM,
                            S_T_RAIN_HALL_05MM, S_T_RAIN_HALL_1MM};
const uint8_t pressureList[] = {S_T_UNSUED, S_T_ADC};
const uint8_t rainPresentList[] = {S_T_UNSUED, S_T_RAIN_PRESENT_DI};
const uint8_t snowList[] = {S_T_UNSUED, S_T_SNOW_HJ};
const uint8_t humiList[] = {S_T_UNSUED, S_T_HUMINITY_HJ, S_T_ADC};
const uint8_t solarRadiationList[] = {S_T_UNSUED, S_T_SOLAR_RADIATION_OTT_SMP3, S_T_ADC};
const uint8_t defaultList[] = {S_T_UNSUED, S_T_ADC};
//ADDMODEL:센서모델이 추가되거나 타입이 추가되면 여기 수정해야함

//ADDMODEL:센서 모델 이 추가되면 여기추가 시켜야함
const sensor_model_entry_t sensor_table[SENSOR_LIST_MAX] = {
    {.list = temperatureList, .cnt = sizeof(temperatureList)},        // A1_TEMPERATURE
    {.list = windDirectionList, .cnt = sizeof(windDirectionList)},    // A2_WIND_DIRECTION
    {.list = windSpeedList, .cnt = sizeof(windSpeedList)},            // A3_WIND_SPEED
    {.list = rainList, .cnt = sizeof(rainList)},                      // A6_RAINFALL_DOT5_1MM
    {.list = pressureList, .cnt = sizeof(pressureList)},              // A7_PRESSURE
    {.list = rainPresentList, .cnt = sizeof(rainPresentList)},        // A8_RAIN_PRESENT
    {.list = snowList, .cnt = sizeof(snowList)},                      // A9_SNOW_DEPTH
    {.list = humiList, .cnt = sizeof(humiList)},                      // A10_RELATIVE_HUMIDITY
    {.list = defaultList, .cnt = sizeof(defaultList)},                // A11_RAINFALL_DOT1MM
    {.list = solarRadiationList, .cnt = sizeof(solarRadiationList)},  // B1_SOLAR_RADIATION
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B2_SUNSHINE_DURATION
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B3_GROUND_TEMPERATURE
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B4_SURFACE_TEMPERATURE
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B5_SOIL_TEMPERATURE_5CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B6_SOIL_TEMPERATURE_10CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B7_SOIL_TEMPERATURE_20CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B8_SOIL_TEMPERATURE_30CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B9_SOIL_TEMPERATURE_50CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B10_SOIL_TEMPERATURE_100CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B11_SOIL_TEMPERATURE_150CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B12_SOIL_TEMPERATURE_300CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // B13_SOIL_TEMPERATURE_500CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C1_CLOUD_BASE1
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C2_CLOUD_BASE2
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C3_CLOUD_BASE3
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C4_CLOUD_COVER
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C5_VISIBILITY
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C6_PM10
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C7_PM2DOT5
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C8_NET_RADIATION
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C9_TOTAL_RADIATION
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C10_REFLECTED_RADIATION
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C11_DIRECT_SOLAR
    {.list = defaultList, .cnt = sizeof(defaultList)},                // C12_CURRENT_WEATHER
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N1_SOIL_MOISTURE_10CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N2_SOIL_MOISTURE_20CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N3_SOIL_MOISTURE_30CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N4_SOIL_MOISTURE_50CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N5_ILLUMINANCE
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N6_WIND_VELOCITY_150CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N7_WIND_VELOCITY_400CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N8_INSTANT_VELOCITY_150CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N9_INSTANT_VELOCITY_400CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N10_AIR_TEMPERATURE_50CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N11_AIR_TEMPERATURE_400CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N12_HUMIDITY_50CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                // N13_HUMIDITY_400CM
    {.list = defaultList, .cnt = sizeof(defaultList)}};               // I1_TACHOMETER

void sensor_add_common(sensor_t *sensor, uint8_t index)
{
  uint8_t config_cnt;

  config_cnt = sensor->configCnt;

  if (config_cnt >= SENSOR_CONFIG_TABLE_MAX)
  {
    config_cnt--;
  }
  sensor->config[config_cnt][0] = sensor->type;  // 해당 타입을 추가
  sensor->config[config_cnt][1] = index;
  WRITE_CFG_MEM(&sensor->config[config_cnt], sizeof(sensor->config[config_cnt]));
  config_cnt++;
  sensor->configCnt= config_cnt;
  WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));
}


/**
 * @brief 설정값 할당
 */
void *sensor_add(sensor_t *sensor)
{



  switch (sensor->type)
  {
    case S_T_ADC:
      {
        int cnt = g_config_sensor.adc_cnt;
        if (cnt >= _countof(g_config_sensor.adc))  // 할당 가능한지 판단
        {
          cnt--;
        }
        sensor_add_common(sensor, cnt);
        cnt++;
        g_config_sensor.adc_cnt = cnt;
        return &g_config_sensor.adc[cnt];
       }
    case S_T_WIND_SPEED_HJ_485://
      sensor_add_common(sensor, 0);
      return &g_config_sensor.hjwind_speed;
      break;
    case S_T_HUMINITY_HJ:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.hjhumi;

    case S_T_TEMPERATURE_HJ:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.hjtemp;

    case S_T_WIND_DIRECTION_HJ_485:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.hjwindDir;

    case S_T_SNOW_HJ:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.hjsnow;

    case S_T_SOLAR_RADIATION_OTT_SMP3:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.ott_smp3;
    case S_T_RAIN_PRESENT_DI:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.rain_present;
    case S_T_FREQ:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.frequency;
      break;
    default:
      break;
  }

  return 0;
}




/**
 * @brief 센서타입에 맞는 설정값을 가져옴
 */
//ADDMODEL:센서타입이 추가하면 설정값 구조체에 여기세 추가 해야함 
void *get_sensor_config(sensor_t *sensor)
{
  // configCnt가 0이란건 아직 저장된 config가 없다는것
  if (sensor->configCnt == 0)
  {
    return 0;
  }

  for (int i = 0; i < sensor->configCnt; i++)
  {
    if (sensor->config[i][0] == sensor->type)
    {
      switch (sensor->type)
      {
        case S_T_ADC:
          return &g_config_sensor.adc[sensor->config[i][1]];
        case S_T_WIND_SPEED_HJ_485:
          return &g_config_sensor.hjwind_speed;
        case S_T_WIND_DIRECTION_HJ_485:
          return &g_config_sensor.hjwindDir;
        case S_T_TEMPERATURE_HJ:
          return &g_config_sensor.hjtemp;
        case S_T_HUMINITY_HJ:
          return &g_config_sensor.hjhumi;
        case S_T_SNOW_HJ:
          return &g_config_sensor.hjsnow;
        case S_T_SOLAR_RADIATION_OTT_SMP3:
          return &g_config_sensor.ott_smp3;
        case S_T_RAIN_PRESENT_DI:
          return &g_config_sensor.rain_present;
        case S_T_FREQ:
          return &g_config_sensor.frequency;
      }
    }
  }
  // 해당 센서 타입 config가 설정되어 있지 않으면 추가
  return 0;
}

rain_present_config_t *get_rain_present_config(void)
{
  return &g_config_sensor.rain_present;
}