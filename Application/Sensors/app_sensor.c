
#include "app_sensor.h"

#include <string.h>


#include "config_app.h"
#include "util_memory.h"


const char *g_sensor_model_list[] = {
#define X(name, format) format,
    SENSOR_MODEL_LIST
#undef X
};

const char *sensor_name_list[] = {
#define X(name, name2, format) name2,
    SENSOR_LIST
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
const uint8_t windDirectionInstantList[] = {S_T_UNSUED, S_T_WIND_DIRECTION_MAX_VAL};
const uint8_t windSpeedInstantList[] = {S_T_UNSUED, S_T_WIND_SPEED_MAX_VAL};
const uint8_t rainList[] = {S_T_UNSUED,         S_T_RAIN_REED_05MM, S_T_RAIN_REED_1MM,
                            S_T_RAIN_HALL_05MM, S_T_RAIN_HALL_1MM};
const uint8_t pressureList[] = {S_T_UNSUED, S_T_ADC};
const uint8_t rainPresentList[] = {S_T_UNSUED, S_T_RAIN_PRESENT_DI};
const uint8_t snowList[] = {S_T_UNSUED, S_T_SNOW_HJ};
const uint8_t humiList[] = {S_T_UNSUED, S_T_HUMINITY_HJ, S_T_ADC};
const uint8_t sunShineList[] = {S_T_UNSUED, S_T_SUNSHINE, S_T_ADC};
const uint8_t solarRadiationList[] = {S_T_UNSUED, S_T_SOLAR_RADIATION_OTT_SMP3, S_T_ADC};
const uint8_t soilTemp5cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_5CM, S_T_ADC};
const uint8_t soilTemp10cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_10CM, S_T_ADC};
const uint8_t soilTemp20cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_20CM, S_T_ADC};
const uint8_t soilTemp30cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_30CM, S_T_ADC};
const uint8_t soilTemp50cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_50CM, S_T_ADC};
const uint8_t soilTemp100cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_100CM, S_T_ADC};
const uint8_t soilTemp150cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_150CM, S_T_ADC};
const uint8_t soilTemp300cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_300CM, S_T_ADC};
const uint8_t soilTemp500cmList[] = {S_T_UNSUED, S_T_SOIL_TEMP_500CM, S_T_ADC};
const uint8_t temperature50cmList[] = {S_T_UNSUED, S_T_PT100_B};
const uint8_t defaultList[] = {S_T_UNSUED, S_T_ADC};

const supported_sensors_t supported_sensors[SENSOR_LIST_MAX] = {
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
    {.list = temperature50cmList, .cnt = sizeof(temperature50cmList)},  // N10_AIR_TEMPERATURE_50CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                  // N11_AIR_TEMPERATURE_400CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                  // N12_HUMIDITY_50CM
    {.list = defaultList, .cnt = sizeof(defaultList)},                  // N13_HUMIDITY_400CM
    {.list = defaultList, .cnt = sizeof(defaultList)}};                  // I1_TACHOMETER


void sensor_add_common(sensor_t *sensor, uint8_t index)
{
  sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
  sensor->config[sensor->configCnt][1] = index;
  WRITE_CFG_MEM(&sensor->config[sensor->configCnt], sizeof(sensor->config[sensor->configCnt]));
  sensor->configCnt++;
  WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));
}


/**
 * @brief 설정값 할당
 */
void *sensor_add(sensor_t *sensor)
{
  uint8_t index = 0;

  switch (sensor->type)
  {
    case S_T_ADC:
      if (g_config_sensor.adc_cnt < _countof(g_config_sensor.adc))  // 할당 가능한지 판단
      {
        index = g_config_sensor.adc_cnt;

        sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
        sensor->config[sensor->configCnt][1] = index;

        
        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],
                      sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));

        g_config_sensor.adc_cnt++;
        WRITE_CFG_SENSOR(adc_cnt);

        return &g_config_sensor.adc[index];
      }
    case S_T_TEMP_232:
    case S_T_GENERAL_232:
    case S_T_HART:
      if (g_config_sensor.rs232_cnt < _countof(g_config_sensor.rs232))
      {
        index = g_config_sensor.rs232_cnt;

        sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
        sensor->config[sensor->configCnt][1] = index;

        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],
                      sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));

        g_config_sensor.rs232_cnt++;

        WRITE_CFG_SENSOR(rs232_cnt);
        return &g_config_sensor.rs232[index];
      }
      return 0;
      break;

    case S_T_TEMP_485:
    case S_T_PRESSURE_485:
    case S_T_HUMI_RS485:
    case S_T_GENERAL_485:
      if (g_config_sensor.rs485_cnt < _countof(g_config_sensor.rs485))
      {
        index = g_config_sensor.rs485_cnt;
        sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
        sensor->config[sensor->configCnt][1] = index;
        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],
                      sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));
        g_config_sensor.rs485_cnt++;
        WRITE_CFG_SENSOR(rs485_cnt);
        return &g_config_sensor.rs485[index];
      }
      return 0;
      break;
    case S_T_WIND_SPEED_HJ_485://
        index = 0;
        sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
        sensor->config[sensor->configCnt][1] = index;//타입 배열에 인덱스 값
        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],
                      sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));
        g_config_sensor.hjwind_speed_cnt=1;
        WRITE_CFG_SENSOR(hjwind_speed_cnt);
        return &g_config_sensor.hjwind[index];


      break;
    case S_T_HUMINITY_HJ:
    case S_T_TEMPERATURE_HJ:
      if (g_config_sensor.hjtemp_cnt < _countof(g_config_sensor.hjtemp))
      {
        index = g_config_sensor.hjtemp_cnt;
        sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
        sensor->config[sensor->configCnt][1] = index;
        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],
                      sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));
        g_config_sensor.hjtemp_cnt++;
        WRITE_CFG_SENSOR(hjtemp_cnt);
        return &g_config_sensor.hjtemp[index];
      }
      break;
    case S_T_WIND_DIRECTION_HJ_485:
        index = 0;
        sensor->config[sensor->configCnt][0] = sensor->type;  // 해당 타입을 추가
        sensor->config[sensor->configCnt][1] = index;
        WRITE_CFG_MEM(&sensor->config[sensor->configCnt],
                      sizeof(sensor->config[sensor->configCnt]));
        sensor->configCnt++;
        WRITE_CFG_MEM(&sensor->configCnt, sizeof(sensor->configCnt));
        g_config_sensor.hjwindDir_cnt=1;
        WRITE_CFG_SENSOR(hjwindDir_cnt);
        return &g_config_sensor.hjwindDir[index];

      break;

    case S_T_SNOW_HJ:
      if (g_config_sensor.hjsnow_cnt < _countof(g_config_sensor.hjwindDir))
      {
        index = g_config_sensor.hjsnow_cnt;
        sensor_add_common(sensor, index);
        g_config_sensor.hjsnow_cnt++;
        WRITE_CFG_SENSOR(hjsnow_cnt);
        return &g_config_sensor.hjsnow[index];
      }
      break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
      if (g_config_sensor.ott_smp3_cnt < _countof(g_config_sensor.ott_smp3))
      {
        index = g_config_sensor.ott_smp3_cnt;
        sensor_add_common(sensor, index);
        g_config_sensor.ott_smp3_cnt++;
        WRITE_CFG_SENSOR(ott_smp3_cnt);
        return &g_config_sensor.ott_smp3[index];
      }
      break;
    case S_T_RAIN_PRESENT_DI:
      sensor_add_common(sensor, 0);
      return &g_config_sensor.rain_present;
      break;
    case S_T_FREQ:
      sensor_add_common(sensor, 0);
      g_config_sensor.frequency_cnt++;
      WRITE_CFG_SENSOR(frequency_cnt);
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
          break;
        case S_T_TEMP_232:
        case S_T_GENERAL_232:
        case S_T_HART:
          return &g_config_sensor.rs232[sensor->config[i][1]];
          break;
        case S_T_TEMP_485:
        case S_T_PRESSURE_485:
        case S_T_HUMI_RS485:
        case S_T_GENERAL_485:
          return &g_config_sensor.rs485[sensor->config[i][1]];
          break;
        case S_T_MODBUS:
          return &g_config_sensor.modbus[sensor->config[i][1]];
          break;
        case S_T_WIND_SPEED_HJ_485:
          return &g_config_sensor.hjwind[0];
          break;
        case S_T_WIND_DIRECTION_HJ_485:
          return &g_config_sensor.hjwindDir[0];
          break;
        case S_T_TEMPERATURE_HJ:
        case S_T_HUMINITY_HJ:
          return &g_config_sensor.hjtemp[sensor->config[i][1]];
          break;


        case S_T_SNOW_HJ:
          return &g_config_sensor.hjsnow[sensor->config[i][1]];
          break;
        case S_T_SOLAR_RADIATION_OTT_SMP3:
          return &g_config_sensor.ott_smp3[0];
          break;
        case S_T_RAIN_PRESENT_DI:
          return &g_config_sensor.rain_present;
          break;
        case S_T_FREQ:
          return &g_config_sensor.frequency;
          break;
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