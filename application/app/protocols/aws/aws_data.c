#include "aws_data.h"
#include "app_sensor.h"

#include "wind_speed\wind_speed.h"
#include "temperature\temperature.h"
#include "wind_direction\wind_direction.h"
#include "humidity\humidity.h"
#include "barometer\barometer.h"

#include "task_measure.h"
#include "cmsis_os2.h"
kma_data_t g_kma_inst;//실시간, 순간자료, 평균낸 자료
kma_data_t g_kma_1min;
kma_data_t g_kma_10min;
kma_data_t g_kma_hour;

kma_data_ex_t g_kma_raw_ex;
kma_data_ex_t g_kma_inst_ex;
kma_data_ex_t g_kma_1min_ex;
kma_data_ex_t g_kma_10min_ex;
kma_data_ex_t g_kma_1Hour_ex;

rainfall_t g_rainfall;
sunshine_t g_sunshine;


osMessageQueueId_t g_kma_data_queue[2];

// 실제 수집된 데이터를 AWS에서 요구하는 형태로 저장해야한다.

#define AWS_CVT_TEMP(x) (x == TEMP_ERR_VAL ? -9999 : (x + 100) * 10)
#define AWS_CVT_HUMI(x) (x == HUMI_ERR_VAL ? -9999 : (x * 10))
#define AWS_CVT_BAROMETER(x) (x == BAROMETER_ERR_VAL ? -9999 : (x * 10))
#define AWS_CVT_WIND_DIRECTION(x) (x == WIND_DIRECTION_ERR_VAL ? -9999 : (x * 10))

#define AWS_CVT_WIND_SPEED(x) (x == WIND_SPEED_ERR_VAL ? -9999 : (x * 10))

#define AWS_CVT_DEFAULT(x) (x * 10)
#define AWS_CVT_G(x) ((x + 100) * 10)

#define UNUSED_SENSOR_VAL -999

    /**
     * @brief 센서 데이터를 AWS 자료형으로 변환환
     */
    void
    cvt_sensorToAWS(sensor_t *p_sensor, sensor_data_t *p_data, kma_data_t *p_kma)
{



  // 1. 기온 (1분 평균)  표현 범위 500~1500 [(관측값 + 100)*100]
  p_kma->temperature = p_sensor[A1_TEMPERATURE].type
                           ? (int16_t)AWS_CVT_TEMP(p_data[A1_TEMPERATURE].data.f)
                           : UNUSED_SENSOR_VAL;

  // 2. 풍향 (1분 평균) 표현범위 → 1 ～ 3599 (관측값 × 10)
  p_kma->wind_direction_avg =
      p_sensor[A2_WIND_DIRECTION].type
          ? (int16_t)AWS_CVT_WIND_DIRECTION(p_data[A2_WIND_DIRECTION].data.f)
          : UNUSED_SENSOR_VAL;

  // 3. 풍속 (1분 평균) 표현범위 → 1 ～ 1000 (관측값 × 10)
  p_kma->wind_speed_avg = p_sensor[A3_WIND_SPEED].type
                              ? (int16_t)AWS_CVT_WIND_SPEED(p_data[A3_WIND_SPEED].data.f)
                              : UNUSED_SENSOR_VAL;

  // 4. 풍향 (1분 순간) 표현범위 → 0 ～ 3599 (관측값 × 10)


  // 5. 풍속 (1분 순간) 표현범위 → 0 ～ 1000 (관측값 × 10


  // 6. 강수량 (0.5/1.0 mm) 표현범위 → 0 ～ 32767 (관측값 × 10)
  p_kma->precipitation = p_sensor[A6_RAINFALL_DOT5_1MM].type
                             ? (int16_t)(p_data[A6_RAINFALL_DOT5_1MM].data.i)
                             : UNUSED_SENSOR_VAL;

  // 7. 기압 (1분 평균 현지 기압) 표현범위 → 5000 ～ 11000 (관측값 × 10
  p_kma->pressure = p_sensor[A6_RAINFALL_DOT5_1MM].type
                        ? (int16_t)AWS_CVT_BAROMETER(p_data[A7_PRESSURE].data.f)
                        : UNUSED_SENSOR_VAL;

  if (p_sensor[A8_RAIN_PRESENT].type)
  {
    // 8. 강수 유무
    if (p_data[A8_RAIN_PRESENT].data.b == true)
    {
      p_kma->precipitation_presence = 10;
    }
    else
    {
      p_kma->precipitation_presence = 0;
    }
  }
  else
  {
    p_kma->precipitation_presence = UNUSED_SENSOR_VAL;
  }

  // 9. 적설  표현범위: 0 ~ 4095 (관측값 * 10)
  p_kma->snowfall = p_sensor[A9_SNOW_DEPTH].type
                        ? (int16_t)AWS_CVT_DEFAULT(p_data[A9_SNOW_DEPTH].data.f)
                        : UNUSED_SENSOR_VAL;

  // 10. 상대습도 (1분 평균) 표현범위: 0 ~ 1000 (관측값 * 10)
  p_kma->relative_humidity = p_sensor[A10_RELATIVE_HUMIDITY].type
                                 ? (int16_t)(AWS_CVT_HUMI(p_data[A10_RELATIVE_HUMIDITY].data.f))
                                 : UNUSED_SENSOR_VAL;

  // 11. 강수량 (0.1 mm) 표현범위 → 0 ～ 32767 (관측값 × 10)
  p_kma->precipitation_fine = p_sensor[A11_RAINFALL_DOT1MM].type
                                  ? (int16_t)(AWS_CVT_DEFAULT(p_data[A11_RAINFALL_DOT1MM].data.f))
                                  : UNUSED_SENSOR_VAL;

  // 1. 일사 (누적값)표현범위: 0 ~ 32767 [관측값(MJ/m²) * 100]
  p_kma->solar_radiation = p_sensor[B1_SOLAR_RADIATION].type
                               ? (int16_t)(p_data[B1_SOLAR_RADIATION].data.f * 100)
                               : UNUSED_SENSOR_VAL;

  // 2. 일조 (누적 시간)표현범위: 0 ~ 65535 [누적시간(초 단위)]
  p_kma->sunshine_duration = p_sensor[B2_SUNSHINE_DURATION].type
                                 ? (int16_t)(p_data[B2_SUNSHINE_DURATION].data.f)
                                 : UNUSED_SENSOR_VAL;

  // 3. 지면온도 (1분 평균)표현범위: 500 ~ 2000 [(관측값 + 100) * 10]
  p_kma->surface_temperature = p_sensor[B3_GROUND_TEMPERATURE].type
                                   ? (int16_t)AWS_CVT_G(p_data[B3_GROUND_TEMPERATURE].data.f)
                                   : UNUSED_SENSOR_VAL;

  // 4. 초상온도 (1분 평균)표현범위: 500 ~ 2000 [(관측값 + 100) * 10]
  p_kma->grass_temperature = p_sensor[B4_SURFACE_TEMPERATURE].type
                                 ? (int16_t)AWS_CVT_G(p_data[B4_SURFACE_TEMPERATURE].data.f * 1000)
                                 : UNUSED_SENSOR_VAL;

  // 5. 지중온도 (5cm, 1분 평균)
  p_kma->soil_temperature_5cm = p_sensor[B5_SOIL_TEMPERATURE_5CM].type
                                    ? (int16_t)AWS_CVT_G(p_data[B5_SOIL_TEMPERATURE_5CM].data.f)
                                    : UNUSED_SENSOR_VAL;

  // 6. 지중온도 (10cm, 1분 평균)
  p_kma->soil_temperature_10cm = p_sensor[B6_SOIL_TEMPERATURE_10CM].type
                                     ? (int16_t)AWS_CVT_G(p_data[B6_SOIL_TEMPERATURE_10CM].data.f)
                                     : UNUSED_SENSOR_VAL;

  // 7. 지중온도 (20cm, 1분 평균)
  p_kma->soil_temperature_20cm = p_sensor[B7_SOIL_TEMPERATURE_20CM].type
                                     ? (int16_t)AWS_CVT_G(p_data[B7_SOIL_TEMPERATURE_20CM].data.f)
                                     : UNUSED_SENSOR_VAL;

  // 8. 지중온도 (30cm, 1분 평균)
  p_kma->soil_temperature_30cm = p_sensor[B8_SOIL_TEMPERATURE_30CM].type
                                     ? (int16_t)AWS_CVT_G(p_data[B8_SOIL_TEMPERATURE_30CM].data.f)
                                     : UNUSED_SENSOR_VAL;

  // 9. 지중온도 (50cm, 1분 평균)
  p_kma->soil_temperature_50cm = p_sensor[B9_SOIL_TEMPERATURE_50CM].type
                                     ? (int16_t)AWS_CVT_G(p_data[B9_SOIL_TEMPERATURE_50CM].data.f)
                                     : UNUSED_SENSOR_VAL;

  // 10. 지중온도 (1.0m, 1분 평균)
  p_kma->soil_temperature_1m = p_sensor[B10_SOIL_TEMPERATURE_100CM].type
                                   ? (int16_t)AWS_CVT_G(p_data[B10_SOIL_TEMPERATURE_100CM].data.f)
                                   : UNUSED_SENSOR_VAL;

  // 11. 지중온도 (1.5m, 1분 평균)
  p_kma->soil_temperature_1_5m = p_sensor[B11_SOIL_TEMPERATURE_150CM].type
                                     ? (int16_t)AWS_CVT_G(p_data[B11_SOIL_TEMPERATURE_150CM].data.f)
                                     : UNUSED_SENSOR_VAL;

  // 12. 지중온도 (3.0m, 1분 평균)
  p_kma->soil_temperature_3m = p_sensor[B12_SOIL_TEMPERATURE_300CM].type
                                   ? (int16_t)AWS_CVT_G(p_data[B12_SOIL_TEMPERATURE_300CM].data.f)
                                   : UNUSED_SENSOR_VAL;

  // 13. 지중온도 (5.0m, 1분 평균)
  p_kma->soil_temperature_5m = p_sensor[B13_SOIL_TEMPERATURE_500CM].type
                                   ? (int16_t)AWS_CVT_G(p_data[B13_SOIL_TEMPERATURE_500CM].data.f)
                                   : UNUSED_SENSOR_VAL;

  // 1. 1층 운고 (1분 평균)표현범위: 0 ~ 8000 (관측값[m])
  p_kma->cloud_height_1st =
      p_sensor[C1_CLOUD_BASE1].type ? (int16_t)(p_data[C1_CLOUD_BASE1].data.i) : UNUSED_SENSOR_VAL;

  // 2. 2층 운고 (1분 평균)표현범위: 0 ~ 8000 (관측값[m])
  p_kma->cloud_height_2nd =
      p_sensor[C2_CLOUD_BASE2].type ? (int16_t)(p_data[C2_CLOUD_BASE2].data.i) : UNUSED_SENSOR_VAL;

  // 3. 3층 운고 (1분 평균)표현범위: 0 ~ 8000 (관측값[m])
  p_kma->cloud_height_3rd =
      p_sensor[C3_CLOUD_BASE3].type ? (int16_t)(p_data[C3_CLOUD_BASE3].data.i) : UNUSED_SENSOR_VAL;

  // 4. 운량 표현범위: 0 ~ 10 (관측값)
  p_kma->cloud_amount =
      p_sensor[C4_CLOUD_COVER].type ? (int16_t)(p_data[C4_CLOUD_COVER].data.i) : UNUSED_SENSOR_VAL;

  // 5. 시정 (1분 평균)표현범위: 0 ~ 50000 (관측값[m])
  p_kma->visibility =
      p_sensor[C5_VISIBILITY].type ? (int16_t)(p_data[C5_VISIBILITY].data.i) : UNUSED_SENSOR_VAL;

  // 6. PM10 (분진농도)표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)
  p_kma->pm10_concentration =
      p_sensor[C6_PM10].type ? (int16_t)(p_data[C6_PM10].data.f * 10) : UNUSED_SENSOR_VAL;

  // 7. PM2.5 (분진농도)표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)
  p_kma->pm25_concentration =
      p_sensor[C7_PM2DOT5].type ? (int16_t)(p_data[C7_PM2DOT5].data.f * 1000) : UNUSED_SENSOR_VAL;

  // 8. 순복사 (1분 평균)
  p_kma->net_radiation =
      p_sensor[C8_NET_RADIATION].type
          ? (int16_t)(p_data[C8_NET_RADIATION].data.f * 1000)
          : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 9. 전천복사 (1분 평균)
  p_kma->total_radiation =
      p_sensor[C9_TOTAL_RADIATION].type
          ? (int16_t)(p_data[C9_TOTAL_RADIATION].data.f * 1000)
          : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 10. 반사복사 (1분 평균)
  p_kma->reflected_radiation =
      p_sensor[C10_REFLECTED_RADIATION].type
          ? (int16_t)(p_data[C10_REFLECTED_RADIATION].data.f * 1000)
          : UNUSED_SENSOR_VAL;  // 사 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 11. 직달복사 (1분 평균)
  p_kma->direct_radiation =
      p_sensor[C11_DIRECT_SOLAR].type
          ? (int16_t)(p_data[C11_DIRECT_SOLAR].data.f * 1000)
          : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 12. 현재 일기
  p_kma->current_weather = p_sensor[C12_CURRENT_WEATHER].type
                               ? (int16_t)(p_data[C12_CURRENT_WEATHER].data.f * 1000)
                               : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 99 (관측값)

  // 1. 토양수분 (10 cm)
  p_kma->soil_moisture_10cm = p_sensor[N1_SOIL_MOISTURE_10CM].type
                                  ? (int16_t)(p_data[N1_SOIL_MOISTURE_10CM].data.f * 1000)
                                  : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 1000 (관측값 * 10)

  // 2. 토양수분 (20 cm)
  p_kma->soil_moisture_20cm = p_sensor[N2_SOIL_MOISTURE_20CM].type
                                  ? (int16_t)(p_data[N2_SOIL_MOISTURE_20CM].data.f * 1000)
                                  : UNUSED_SENSOR_VAL;  // 표현범위: 0 ~ 1000 (관측값 * 10)

  // 3. 토양수분 (30 cm)
  p_kma->soil_moisture_30cm = p_sensor[N3_SOIL_MOISTURE_30CM].type
                                  ? (int16_t)(p_data[N3_SOIL_MOISTURE_30CM].data.f * 1000)
                                  : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 1000 (관측값 * 10)

  // 4. 토양수분 (50 cm)
  p_kma->soil_moisture_50cm = p_sensor[N4_SOIL_MOISTURE_50CM].type
                                  ? (int16_t)(p_data[N4_SOIL_MOISTURE_50CM].data.f * 1000)
                                  : UNUSED_SENSOR_VAL;  // 표현범위: 0 ~ 1000 (관측값 * 10)

  // 5. 조도량 (1분 평균)
  p_kma->illuminance = p_sensor[N5_ILLUMINANCE].type
                           ? (int16_t)(p_data[N5_ILLUMINANCE].data.f * 1000)
                           : UNUSED_SENSOR_VAL;  // 표현범위: 0 ~ 32767 (관측값 * 100)

  // 6. 풍속 (1.5 m, 1분 평균)
  p_kma->wind_speed_1_5m = p_sensor[N6_WIND_VELOCITY_150CM].type
                               ? (int16_t)(p_data[N6_WIND_VELOCITY_150CM].data.f * 1000)
                               : UNUSED_SENSOR_VAL;  //  표현범위: 0 ~ 1000 (관측값 * 10)

  // 7. 풍속 (4.0 m, 1분 평균)
  p_kma->wind_speed_4m = p_sensor[N7_WIND_VELOCITY_400CM].type
                             ? (int16_t)(p_data[N7_WIND_VELOCITY_400CM].data.f * 1000)
                             : UNUSED_SENSOR_VAL;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드),
                                                   // 표현범위: 0 ~ 1000 (관측값 * 10)

  // 8. 순간 풍속 (1.5 m)
  p_kma->instant_wind_speed_1_5m =
      p_sensor[N8_INSTANT_VELOCITY_150CM].type
          ? (int16_t)(p_data[N8_INSTANT_VELOCITY_150CM].data.f * 1000)
          : UNUSED_SENSOR_VAL;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 9. 순간 풍속 (4.0 m)
  p_kma->instant_wind_speed_4m =
      p_sensor[N9_INSTANT_VELOCITY_400CM].type
          ? (int16_t)(p_data[N9_INSTANT_VELOCITY_400CM].data.f * 1000)
          : UNUSED_SENSOR_VAL;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 10. 기온 (0.5 m)
  p_kma->temperature_0_5m =
      p_sensor[N10_AIR_TEMPERATURE_50CM].type
          ? (int16_t)(p_data[N10_AIR_TEMPERATURE_50CM].data.f * 1000)
          : UNUSED_SENSOR_VAL;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                // 1500 [(관측값 + 100) * 10]

  // 11. 기온 (4.0 m)
  p_kma->temperature_4m = p_sensor[N11_AIR_TEMPERATURE_400CM].type
                              ? (int16_t)(p_data[N11_AIR_TEMPERATURE_400CM].data.f * 1000)
                              : UNUSED_SENSOR_VAL;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드),
                                                    // 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

  // 12. 습도 (0.5 m, 1분 평균)// 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
  // (관측값 * 10)
  p_kma->humidity_0_5m = p_sensor[N12_HUMIDITY_50CM].type
                             ? (int16_t)(p_data[N12_HUMIDITY_50CM].data.f * 10)
                             : UNUSED_SENSOR_VAL;

  // 13. 습도 (4.0 m, 1분 평균) // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
  // (관측값 * 10)
  p_kma->humidity_4m = p_sensor[N13_HUMIDITY_400CM].type
                           ? (int16_t)(p_data[N13_HUMIDITY_400CM].data.f * 10)
                           : UNUSED_SENSOR_VAL;

  p_kma->tacometer = p_sensor[I1_TACHOMETER].type ? (int16_t)(p_data[I1_TACHOMETER].data.f * 1000)
                                                  : UNUSED_SENSOR_VAL;

  p_kma->sensorStatus[0] = 0;
  p_kma->sensorStatus[1] = 0;
  p_kma->sensorStatus[2] = 0;
  p_kma->sensorStatus[3] = 0;
  p_kma->sensorStatus[4] = 0;
  p_kma->sensorStatus[5] = 0;
  p_kma->sensorStatus[6] = 0;
  p_kma->sensorStatus[7] = 0;

  p_kma->volateStatus = 0;

  p_kma->crc = 0;
  p_kma->init = true;
}

rainfall_t *get_rainfall(void)
{
  return &g_rainfall;
}

void set_rainfall_1min(float rainfall) { g_rainfall.rainfall_1min = rainfall; }
void set_rainfall_10min(float rainfall) { g_rainfall.rainfall_10min = rainfall; }
void set_rainfall_hourly(float rainfall) { g_rainfall.rainfall_hourly = rainfall; }
void set_rainfall_today(float rainfall) { g_rainfall.rainfall_today = rainfall; }
void set_rainfall_monthly(float rainfall) { g_rainfall.rainfall_monthly = rainfall; }
void set_rainfall_yesterday(float rainfall) { g_rainfall.rainfall_yesterday = rainfall; }
void set_rainfall_yearly(float rainfall) { g_rainfall.rainfall_yearly = rainfall; }




sunshine_t *get_sunshine(void)
{
  return &g_sunshine;
}

void set_sunshine_yesterday(uint32_t sunshine) { g_sunshine.sunshine_yesterday = sunshine; }

void set_sunshine_today(uint32_t sunshine) { g_sunshine.sunshine_today = sunshine; }

void set_sunshine_hourly(uint32_t sunshine) { g_sunshine.sunshine_hourly = sunshine; }

void set_sunshine_monthly(uint32_t sunshine) { g_sunshine.sunshine_monthly = sunshine; }

void set_sunshine_yearly(uint32_t sunshine) { g_sunshine.sunshine_yearly = sunshine; }

kma_data_ex_t *get_kma_data(eAWS_DATA_MIN_t min)
{
  kma_data_ex_t *p_kma_data = NULL;

  switch (min)
  {
    case eAWS_DATA_AVG:
      p_kma_data = &g_kma_inst_ex;
      break;
    case eAWS_DATA_1MIN:
      p_kma_data = &g_kma_1min_ex;
      break;
    case eAWS_DATA_10MIN:
      p_kma_data = &g_kma_10min_ex;
      break;
    case eAWS_DATA_HOUR:
      p_kma_data = &g_kma_1Hour_ex;
      break;
      case eAWS_DATA_RAW:
      p_kma_data = &g_kma_raw_ex;
      break;
      default:
      break;
  }

#if 0 
  p_kma_data->temperature.enable = 1;
  p_kma_data->wind_direction_avg.enable = 1;
  p_kma_data->wind_speed_avg.enable = 1;
  p_kma_data->wind_direction_instant.enable = 1;
  p_kma_data->wind_speed_instant.enable = 1;
  p_kma_data->precipitation.enable = 1;
  p_kma_data->pressure.enable = 1;

  p_kma_data->precipitation_presence.enable = 1;

  p_kma_data->snowfall.enable = 1;
  p_kma_data->relative_humidity.enable = 1;

  p_kma_data->solar_radiation.enable = 1;
  p_kma_data->sunshine_duration.enable = 1;
  p_kma_data->soil_temperature_5cm.enable = 1;
  p_kma_data->soil_temperature_10cm.enable = 1;

  p_kma_data->soil_temperature_20cm.enable = 1;
  p_kma_data->soil_temperature_30cm.enable = 1;
  p_kma_data->soil_temperature_50cm.enable = 1;
  p_kma_data->soil_temperature_1m.enable = 1;
  p_kma_data->soil_temperature_1_5m.enable = 1;
  p_kma_data->soil_temperature_3m.enable = 1;
#endif
  return p_kma_data;
}

//실시간 값 저장용
void kma_data_q_init(void)
{
  g_kma_data_queue[KMA_DATA_Q_AVG] = osMessageQueueNew(1, sizeof(kma_data_ex_t), NULL);
  g_kma_data_queue[KMA_DATA_Q_1MIN] = osMessageQueueNew(1, sizeof(kma_data_ex_t), NULL);
}

int32_t read_kma_data(int kma_data_num, kma_data_ex_t *p_kma_data)
{
  if(osMessageQueueGet(g_kma_data_queue[kma_data_num], p_kma_data, NULL, 0) == osOK)
  {
    return 0;
  }

  return 1;
}

void send_kma_data(int kma_data_num, kma_data_ex_t *p_kma_data)
{
  osMessageQueuePut(g_kma_data_queue[kma_data_num], p_kma_data, 0, 0);
  
}
