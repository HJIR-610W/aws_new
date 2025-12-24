
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



const supported_sensors_t supported_sensors[SENSOR_LIST_MAX] =
    {
        [A1_TEMPERATURE] = true,
        [A2_WIND_DIRECTION] = true,
        [A3_WIND_SPEED] = true,
        [A6_RAINFALL_DOT5_1MM] = true,
        [A7_PRESSURE] = true,
        [A8_RAIN_PRESENT] = true,
        [A9_SNOW_DEPTH] = true,
        [A10_RELATIVE_HUMIDITY] = true,
        [B1_SOLAR_RADIATION] = true,
        [B2_SUNSHINE_DURATION] = true,
        [B5_SOIL_TEMPERATURE_5CM] = true,
        [B6_SOIL_TEMPERATURE_10CM] = true,
        [B7_SOIL_TEMPERATURE_20CM] = true,
        [B8_SOIL_TEMPERATURE_30CM] = true,
        [B9_SOIL_TEMPERATURE_50CM] = true,
        [B10_SOIL_TEMPERATURE_100CM] = true,
        [B11_SOIL_TEMPERATURE_150CM] = true,
        [B12_SOIL_TEMPERATURE_300CM] = true,
        [B13_SOIL_TEMPERATURE_500CM] = true,
        };

// 지원하는 센서 목록 정의
const uint8_t temperature_list[] = {S_T_UNSUED, S_T_TEMPERATURE_HJ, S_T_PT100};
const uint8_t wind_direction_list[] = {S_T_UNSUED, S_T_WIND_DIRECTION_HJ_MODBUS,S_T_WIND_DIRECTION_HJ_485, S_T_WIND_DIRECTION_RMYOUNG_05103V, S_T_ADC};
const uint8_t wind_speed_list[] = {S_T_UNSUED, S_T_WIND_SPEED_HJ_MODBUS,S_T_WIND_SPEED_HJ_485, S_T_WIND_SPEED_RMYOUNG_05103V,S_T_FREQ};
const uint8_t rainfall_list[] = {S_T_UNSUED,         S_T_RAIN_REED,  S_T_RAIN_HALL};
const uint8_t pressure_list[] = {S_T_UNSUED, S_T_BARO_RMYOUNG_61402V, S_T_BARO_JINSUNG_SJGP215,S_T_BARO_RMYOUNG_61402V_RS232, S_T_ADC};
const uint8_t rain_present_list[] = {S_T_UNSUED, S_T_RAIN_PRESENT_DI,S_T_RAIN_PRESENT_ANALOG};
const uint8_t snow_list[] = {S_T_UNSUED, S_T_SNOW_HJ};
const uint8_t humi_list[] = {S_T_UNSUED, S_T_HUMINITY_HJ, S_T_ADC};
const uint8_t solar_radiation_list[] = {S_T_UNSUED, S_T_SOLAR_RADIATION_OTT_SMP3, S_T_ADC};
const uint8_t solar_duration_list[] ={S_T_UNSUED,S_T_SOLAR_DURATION_CSD3,S_T_ADC};
const uint8_t default_list[] = {S_T_UNSUED};
const uint8_t soil_temp_list[] = {S_T_UNSUED, S_T_ADC};

//ADDMODEL:센서모델이 추가되거나 타입이 추가되면 여기 수정해야함

//ADDMODEL:센서 모델 이 추가되면 여기추가 시켜야함
const sensor_model_entry_t sensor_table[SENSOR_LIST_MAX] = {
    {.list = temperature_list, .cnt = sizeof(temperature_list)},         // A1_TEMPERATURE
    {.list = wind_direction_list, .cnt = sizeof(wind_direction_list)},     // A2_WIND_DIRECTION
    {.list = wind_speed_list, .cnt = sizeof(wind_speed_list)},             // A3_WIND_SPEED
    {.list = rainfall_list, .cnt = sizeof(rainfall_list)},                       // A6_RAINFALL_DOT5_1MM
    {.list = pressure_list, .cnt = sizeof(pressure_list)},               // A7_PRESSURE
    {.list = rain_present_list, .cnt = sizeof(rain_present_list)},         // A8_RAIN_PRESENT
    {.list = snow_list, .cnt = sizeof(snow_list)},                       // A9_SNOW_DEPTH
    {.list = humi_list, .cnt = sizeof(humi_list)},                       // A10_RELATIVE_HUMIDITY
    {.list = default_list, .cnt = sizeof(default_list)},                 // A11_RAINFALL_DOT1MM
    {.list = solar_radiation_list, .cnt = sizeof(solar_radiation_list)},   // B1_SOLAR_RADIATION
    {.list = solar_duration_list, .cnt = sizeof(solar_duration_list)}, // B2_SUNSHINE_DURATION
    {.list = default_list, .cnt = sizeof(default_list)},                 // B3_GROUND_TEMPERATURE
    {.list = default_list, .cnt = sizeof(default_list)},                 // B4_SURFACE_TEMPERATURE
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B5_SOIL_TEMPERATURE_5CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B6_SOIL_TEMPERATURE_10CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B7_SOIL_TEMPERATURE_20CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B8_SOIL_TEMPERATURE_30CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B9_SOIL_TEMPERATURE_50CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B10_SOIL_TEMPERATURE_100CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B11_SOIL_TEMPERATURE_150CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B12_SOIL_TEMPERATURE_300CM
    {.list = soil_temp_list, .cnt = sizeof(soil_temp_list)},           // B13_SOIL_TEMPERATURE_500CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // C1_CLOUD_BASE1
    {.list = default_list, .cnt = sizeof(default_list)},                 // C2_CLOUD_BASE2
    {.list = default_list, .cnt = sizeof(default_list)},                 // C3_CLOUD_BASE3
    {.list = default_list, .cnt = sizeof(default_list)},                 // C4_CLOUD_COVER
    {.list = default_list, .cnt = sizeof(default_list)},                 // C5_VISIBILITY
    {.list = default_list, .cnt = sizeof(default_list)},                 // C6_PM10
    {.list = default_list, .cnt = sizeof(default_list)},                 // C7_PM2DOT5
    {.list = default_list, .cnt = sizeof(default_list)},                 // C8_NET_RADIATION
    {.list = default_list, .cnt = sizeof(default_list)},                 // C9_TOTAL_RADIATION
    {.list = default_list, .cnt = sizeof(default_list)},                 // C10_REFLECTED_RADIATION
    {.list = default_list, .cnt = sizeof(default_list)},                 // C11_DIRECT_SOLAR
    {.list = default_list, .cnt = sizeof(default_list)},                 // C12_CURRENT_WEATHER
    {.list = default_list, .cnt = sizeof(default_list)},                 // N1_SOIL_MOISTURE_10CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N2_SOIL_MOISTURE_20CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N3_SOIL_MOISTURE_30CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N4_SOIL_MOISTURE_50CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N5_ILLUMINANCE
    {.list = default_list, .cnt = sizeof(default_list)},                 // N6_WIND_VELOCITY_150CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N7_WIND_VELOCITY_400CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N8_INSTANT_VELOCITY_150CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N9_INSTANT_VELOCITY_400CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N10_AIR_TEMPERATURE_50CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N11_AIR_TEMPERATURE_400CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N12_HUMIDITY_50CM
    {.list = default_list, .cnt = sizeof(default_list)},                 // N13_HUMIDITY_400CM
    {.list = default_list, .cnt = sizeof(default_list)}};                // I1_TACHOMETER





rain_present_config_t *get_rain_present_config(void)
{
  return &g_config_sensor.rain_present;
}



/**
 * @brief 센서타입에 맞는 설정값을 가져옴
 */
//ADDMODEL:센서타입이 추가하면 설정값 구조체에 여기세 추가 해야함 
void *get_sensor_config(  eSENSOR_TYPE_t type,eSENSOR_TYPE_MODEL_t model)
{
  switch(type)
  {
    case A1_TEMPERATURE:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.temp.adc;
        case S_T_TEMPERATURE_HJ:
        return &g_config_sensor.temp.hj;
        case S_T_PT100:
        return &g_config_sensor.temp.pt100;
      }
    break;
    case A2_WIND_DIRECTION:
      switch(model)
      {
        case S_T_WIND_DIRECTION_HJ_485:
        return &g_config_sensor.wind_direction.hj;
        case S_T_WIND_DIRECTION_HJ_MODBUS:
        return &g_config_sensor.wind_direction.hj_modbus;
        case S_T_WIND_DIRECTION_RMYOUNG_05103V:
        return &g_config_sensor.wind_direction.rmyoung_05103v;
      }
    break;
    case A3_WIND_SPEED:
      switch(model)
      {
        case S_T_WIND_SPEED_HJ_485:
        return &g_config_sensor.wind_speed.hj;
        case S_T_WIND_SPEED_HJ_MODBUS:
        return &g_config_sensor.wind_speed.hj_modbus;
        case S_T_WIND_SPEED_RMYOUNG_05103V:
        return &g_config_sensor.wind_speed.rmyoung_05103v;
        case S_T_FREQ:
        return &g_config_sensor.wind_speed.frequency;
      }
    break;
    case A6_RAINFALL_DOT5_1MM:
      switch(model)
      {
        case S_T_RAIN_REED:
        return &g_config_sensor.rain.reed;
        case S_T_RAIN_HALL:
        return &g_config_sensor.rain.hall;
      }
    break;
    case A7_PRESSURE:
      switch(model)
      {
        case S_T_BARO_JINSUNG_SJGP215:
        return &g_config_sensor.baromater.jinsung_sjgp215;
        case S_T_BARO_RMYOUNG_61402V:
        return &g_config_sensor.baromater.rmyoung_61402v_barometer;
        case S_T_BARO_RMYOUNG_61402V_RS232:
        return &g_config_sensor.baromater.rmyoung_61402v_rs232;
        case S_T_ADC:
        return &g_config_sensor.baromater.adc;
        
      }
    break;
    case A8_RAIN_PRESENT:
      switch(model)
      {
        case S_T_RAIN_PRESENT_DI:
        case S_T_RAIN_PRESENT_ANALOG:
        return &g_config_sensor.rain_present;
      }
    break;
    case A9_SNOW_DEPTH:
      switch(model)
      {
        case S_T_SNOW_HJ:
        return &g_config_sensor.snow.hj;
      }
    break;
    case A10_RELATIVE_HUMIDITY:
      switch(model)
      {
        case S_T_HUMINITY_HJ:
        return &g_config_sensor.humi.hj;
        case S_T_ADC:
        return &g_config_sensor.humi.adc;
      }
    break;
    case B1_SOLAR_RADIATION:
      switch(model)
      {
        case S_T_SOLAR_RADIATION_OTT_SMP3:
        return &g_config_sensor.solar_radication.ott_smp3;
        case S_T_ADC:
        return &g_config_sensor.solar_radication.adc;
      }
    break;
    case B2_SUNSHINE_DURATION:
      switch(model)
      {
        case S_T_SOLAR_DURATION_CSD3:
        return &g_config_sensor.sunshine.solar_duration_csd3;
        case S_T_ADC:
        return &g_config_sensor.sunshine.adc;
      }
    break;
    case B5_SOIL_TEMPERATURE_5CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[0].adc;
      }
    break;
    case B6_SOIL_TEMPERATURE_10CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[1].adc;
      }
    break;
    case B7_SOIL_TEMPERATURE_20CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[2].adc;
      }
    break;
    case B8_SOIL_TEMPERATURE_30CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[3].adc;
      }
    break;
    case B9_SOIL_TEMPERATURE_50CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[4].adc;
      }
    break;
    case B10_SOIL_TEMPERATURE_100CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[5].adc;
      }
    break;
    case B11_SOIL_TEMPERATURE_150CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[6].adc;
      }
    break;
    case B12_SOIL_TEMPERATURE_300CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[7].adc;
      }
    break;
    case B13_SOIL_TEMPERATURE_500CM:
      switch(model)
      {
        case S_T_ADC:
        return &g_config_sensor.soil_temp[8].adc;
      }
    break;
    default:
    break;
  }

  return 0;
}