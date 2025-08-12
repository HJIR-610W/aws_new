#include "aws_data.h"
#include "app_sensor.h"

#include "wind_speed\wind_speed.h"
#include "temperature\temperature.h"
#include "wind_direction\wind_direction.h"
#include "humidity\humidity.h"
#include "barometer\barometer.h"

#include "task_measure.h"
#include "cmsis_os2.h"



kma_data_ex_t g_kma_raw_ex;
kma_data_ex_t g_kma_inst_ex;
kma_data_ex_t g_kma_1min_ex;
kma_data_ex_t g_kma_10min_ex;
kma_data_ex_t g_kma_1Hour_ex;

rainfall_t g_rainfall;
sunshine_t g_sunshine;
sunshine_r_t g_sunshine_r;
min_buffer_t g_min_buffer;

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




void set_rainfall_1min(float rainfall) { g_rainfall.min = rainfall; }
void set_rainfall_10min(float rainfall) { g_rainfall.ten_min = rainfall; }
void set_rainfall_hourly(float rainfall) { g_rainfall.hourly = rainfall; }
void set_rainfall_today(float rainfall) { g_rainfall.today = rainfall; }
void set_rainfall_monthly(float rainfall) { g_rainfall.monthly = rainfall; }
void set_rainfall_yesterday(float rainfall) { g_rainfall.yesterday = rainfall; }
void set_rainfall_yearly(float rainfall) { g_rainfall.yearly = rainfall; }







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

/**
 * @brief AI,AB 요청시 업데이트되는 값을 응답하여 생기는 공유자원 충돌 방지 목적
 * dual_port task에서 값이 갱신되는데 갱신중에 aws_hander에서 그 값을 사용하지 않고 
 * 완전히 갱신된 값을 상용하기 위함
 * 갱신된 값을 q에 넣고 AI,AB호출시 q에서 데이터 꺼내서 응답
 */
void kma_data_q_init(void)
{
  g_kma_data_queue[eKMA_DATA_Q_AVG] = osMessageQueueNew(1, sizeof(kma_data_ex_t), NULL);
  g_kma_data_queue[eKMA_DATA_Q_1MIN] = osMessageQueueNew(1, sizeof(kma_data_ex_t), NULL);
}

int32_t read_kma_data(eKMA_DATA_Q_t kma_data_num, kma_data_ex_t *p_kma_data)
{
  if(osMessageQueueGet(g_kma_data_queue[kma_data_num], p_kma_data, NULL, 0) == osOK)
  {
    return 0;
  }

  return 1;
}

void send_kma_data(eKMA_DATA_Q_t kma_data_num, kma_data_ex_t *p_kma_data)
{
  osMessageQueuePut(g_kma_data_queue[kma_data_num], p_kma_data, 0, 0);
  
}
