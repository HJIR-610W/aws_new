/**
 * @file           task_measure.c
 * @brief          하드웨어가 제공가능한 센서를 250ms,1초마다 측정
 * @author         t
 * @date           2025-01-01
 * @version        v1.0.0
 *
 * @note
 *  - 센서 값은 1초 주기로 읽어 메시지 큐로 전달됨
 *
 * @details
 * ### Change Log
 * | Version  | Date       | Description             |
 * |----------|------------|-------------------------|
 * | v1.0.0   | 2025-04-15 |                         |
 */

#include "task_measure.h"

#include <math.h>
#include <string.h>

#include "Sensors\barometer\barometer.h"
#include "Sensors\general\general_adc.h"
#include "Sensors\general\sensor_general.h"
#include "Sensors\humidity\humidity.h"
#include "Sensors\rain\rain.h"
#include "Sensors\rain_present\rain_present.h"
#include "Sensors\snow\snow.h"
#include "Sensors\soil_temperature\soil_temperature.h"
#include "Sensors\solar_radiation\solar_radiation.h"
#include "Sensors\sunshine\sunshine.h"
#include "Sensors\temperature\temperature.h"
#include "Sensors\wind_direction\wind_direction.h"
#include "Sensors\wind_speed\wind_speed.h"
#include "app_adc.h"
#include "app_bsp.h"
#include "app_dataLogging.h"
#include "app_file.h"
#include "app_measure.h"
#include "app_rtc.h"
#include "aws_data.h"
#include "cmsis_os2.h"
#include "config.h"
#include "driver_do.h"
#include "task_logging.h"
#include "usDelay.h"
#include "utile.h"
#include "utile_time.h"
#include "aws_processor.h"

#define MEASURE_PERIOD_MS 250

const osThreadAttr_t kMeasureTask_attributes = {
    .name = "measureTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityRealtime1,
};

const uint32_t kMesaureTimeOutMs = 50;

static sensor_t g_sensor_copy[SENSOR_COUNT_MAX];  // config 센서의 복사본
static driver_t *g_sensor_driver[SENSOR_COUNT_MAX];

osMessageQueueId_t g_measure_queue;//센서 측정 데이터 송순 Q
uint32_t g_debug_start_time; //task 실행시간 측정용
uint32_t g_debug_elased_time;  // task 실행시간 측정용
uint32_t g_debug_elased_max;   // task 실행시간 측정용

measure_data_t g_reading;

/**
 * @brief 측정 데이터 전송송
 */
void
    send_measurement(void *data)
{
  osStatus_t status;

  status = osMessageQueuePut(g_measure_queue, data, 0, kMesaureTimeOutMs);

  if(status != osOK)
  {
    debug_printf("os_send_measureData fail %d\r\n",status);
  }
}

/**
 * @brief 측정 데이터 확인
 */
bool is_measurement(void *data)
{
  osStatus_t status;

  if(g_measure_queue== NULL)
  {
    osDelay(100);
    return false;
  } 
  
  status = osMessageQueueGet(g_measure_queue, data, NULL, osWaitForever);

  if (status != osOK)
  {
    debug_printf("is_measurement fail %d\r\n", status);
    return false;
  }

  return true;

}

/**
 *  센서 모델에 해당하는 드라이버 번호를 넘겨준다.
 *  예)
 *  measure테스트가 제공가능한 센서목록이 있고
 *  센서에 해당하는 실제 모델이 있다. 그것을 연결시켜준다
 *  type:온도센서 -> pt100 을 사용하겠다.
 *  
 * */
int32_t get_driverNum(eSENSOR_MODEL_t type)
{
  int32_t num = -1;  // 항목 없음

  switch (type)
  {
    case S_T_ADC:
      num = GENERAL_ADC;
      break;
    case S_T_PT100_A:
      num = TEMP_PT100_A;
      break;
    case S_T_PT100_B:
      num = TEMP_PT100_B;
      break;
    case S_T_WIND_SPEED_HJ_485:
    case S_T_WIND_DIRECTION_HJ_485:
      num = WIND_HJ;
      break;
    case S_T_SNOW_HJ_485:
      num = SNOW_HJ_485;
      break;
    case S_T_SNOW_HJ_232:
      num = SNOW_HJ_232;
      break;
    case S_T_RAIN_REED_05MM:
      num = RAIN_REED_05MM;
      break;
    case S_T_RAIN_REED_1MM:
      num = RAIN_REED_1MM;
      break;
    case S_T_RAIN_HALL_05MM:
      num = RAIN_HALL_05MM;
      break;
    case S_T_RAIN_HALL_1MM:
      num = RAIN_HALL_1MM;
      break;
    case S_T_TEMPERATURE_HJ_485:
      num = TEMP_HJ_TEMPERATURE;
      break;
  }
  return num;
}

/**
 * @brief 사용하는 센서 초기화
 */
void sensor_init(void)
{
  uint8_t num;
  void *para = NULL;
  sensor_t *p_sensor;

  memcpy(g_sensor_copy, config.sensor, sizeof(g_sensor_copy));

  p_sensor = g_sensor_copy;

  adc_init();  // ADC 항상 초기화

  for (int i = 0; i < SENSOR_COUNT_MAX; i++)
  {
    if (p_sensor[i].type)  // 사용으로 설정되었는 확인
    {
      switch (i)
      {
        case A1_TEMPERATURE:
          num = get_driverNum(p_sensor[A1_TEMPERATURE].type);
          para = get_sensor_config(&p_sensor[A1_TEMPERATURE]);
          g_sensor_driver[A1_TEMPERATURE] = temperature_open(num, para);
          break;
        case A10_RELATIVE_HUMIDITY:
          num = get_driverNum(p_sensor[A10_RELATIVE_HUMIDITY].type);
          para = get_sensor_config(&p_sensor[A10_RELATIVE_HUMIDITY]);
          g_sensor_driver[A10_RELATIVE_HUMIDITY] = humidity_open(num, para);
          break;
        case A3_WIND_SPEED:
          num = get_driverNum(p_sensor[A3_WIND_SPEED].type);
          para = get_sensor_config(&p_sensor[A3_WIND_SPEED]);
          g_sensor_driver[A3_WIND_SPEED] = windSpeed_open(num, para);
          break;
        case A2_WIND_DIRECTION:
          num = get_driverNum(p_sensor[A2_WIND_DIRECTION].type);
          para = get_sensor_config(&p_sensor[A2_WIND_DIRECTION]);
          g_sensor_driver[A2_WIND_DIRECTION] = windSpeed_open(num, para);
          break;
        case A9_SNOW_DEPTH:
          num = get_driverNum(p_sensor[A9_SNOW_DEPTH].type);
          para = get_sensor_config(&p_sensor[A9_SNOW_DEPTH]);
          g_sensor_driver[A9_SNOW_DEPTH] = snow_open(num, para);
          break;
        case A6_RAINFALL_DOT5_1MM:
          num = get_driverNum(p_sensor[A6_RAINFALL_DOT5_1MM].type);
          g_sensor_driver[A6_RAINFALL_DOT5_1MM] = rain_open(num, 0);
          break;
        case A8_RAIN_PRESENT:
          g_sensor_driver[A8_RAIN_PRESENT] = rainPresent_open(RAIN_PRESENT_DI, 0);
          break;
        case A7_PRESSURE:
          num = get_driverNum(p_sensor[A7_PRESSURE].type);
          para = get_sensor_config(&p_sensor[A7_PRESSURE]);
          g_sensor_driver[A7_PRESSURE] = barometer_open(num, para);
          break;
        case B5_SOIL_TEMPERATURE_5CM:
          num = get_driverNum(p_sensor[B5_SOIL_TEMPERATURE_5CM].type);
          para = get_sensor_config(&p_sensor[B5_SOIL_TEMPERATURE_5CM]);
          g_sensor_driver[B5_SOIL_TEMPERATURE_5CM] = barometer_open(num, para);
          break;
        case B6_SOIL_TEMPERATURE_10CM:
          num = get_driverNum(p_sensor[B6_SOIL_TEMPERATURE_10CM].type);
          para = get_sensor_config(&p_sensor[B6_SOIL_TEMPERATURE_10CM]);
          g_sensor_driver[B6_SOIL_TEMPERATURE_10CM] = barometer_open(num, para);
          break;
        case B7_SOIL_TEMPERATURE_20CM:
          num = get_driverNum(p_sensor[B7_SOIL_TEMPERATURE_20CM].type);
          para = get_sensor_config(&p_sensor[B7_SOIL_TEMPERATURE_20CM]);
          g_sensor_driver[B7_SOIL_TEMPERATURE_20CM] = barometer_open(num, para);
          break;
        case B8_SOIL_TEMPERATURE_30CM:
          num = get_driverNum(p_sensor[B8_SOIL_TEMPERATURE_30CM].type);
          para = get_sensor_config(&p_sensor[B8_SOIL_TEMPERATURE_30CM]);
          g_sensor_driver[B8_SOIL_TEMPERATURE_30CM] = barometer_open(num, para);
          break;
        case B9_SOIL_TEMPERATURE_50CM:
          num = get_driverNum(p_sensor[B9_SOIL_TEMPERATURE_50CM].type);
          para = get_sensor_config(&p_sensor[B9_SOIL_TEMPERATURE_50CM]);
          g_sensor_driver[B9_SOIL_TEMPERATURE_50CM] = barometer_open(num, para);
          break;
        case B10_SOIL_TEMPERATURE_100CM:
          num = get_driverNum(p_sensor[B10_SOIL_TEMPERATURE_100CM].type);
          para = get_sensor_config(&p_sensor[B10_SOIL_TEMPERATURE_100CM]);
          g_sensor_driver[B10_SOIL_TEMPERATURE_100CM] = barometer_open(num, para);
          break;
        case B11_SOIL_TEMPERATURE_150CM:
          num = get_driverNum(p_sensor[B11_SOIL_TEMPERATURE_150CM].type);
          para = get_sensor_config(&p_sensor[B11_SOIL_TEMPERATURE_150CM]);
          g_sensor_driver[B11_SOIL_TEMPERATURE_150CM] = barometer_open(num, para);
          break;
        case B12_SOIL_TEMPERATURE_300CM:
          num = get_driverNum(p_sensor[B12_SOIL_TEMPERATURE_300CM].type);
          para = get_sensor_config(&p_sensor[B12_SOIL_TEMPERATURE_300CM]);
          g_sensor_driver[B12_SOIL_TEMPERATURE_300CM] = barometer_open(num, para);
          break;
        case B13_SOIL_TEMPERATURE_500CM:
          num = get_driverNum(p_sensor[B13_SOIL_TEMPERATURE_500CM].type);
          para = get_sensor_config(&p_sensor[B13_SOIL_TEMPERATURE_500CM]);
          g_sensor_driver[B13_SOIL_TEMPERATURE_500CM] = barometer_open(num, para);
          break;
        case B2_SUNSHINE_DURATION:
          num = get_driverNum(p_sensor[B2_SUNSHINE_DURATION].type);
          para = get_sensor_config(&p_sensor[B2_SUNSHINE_DURATION]);
          g_sensor_driver[B2_SUNSHINE_DURATION] = sunshine_open(num, para);
          break;
        case B1_SOLAR_RADIATION:
          num = get_driverNum(p_sensor[B1_SOLAR_RADIATION].type);
          para = get_sensor_config(&p_sensor[B1_SOLAR_RADIATION]);
          g_sensor_driver[B1_SOLAR_RADIATION] = solarRadiation_open(num, para);
          break;
        case N10_AIR_TEMPERATURE_50CM:
          num = get_driverNum(p_sensor[N10_AIR_TEMPERATURE_50CM].type);
          g_sensor_driver[N10_AIR_TEMPERATURE_50CM] = temperature_open(num, 0);
          break;
      }
    }
  }

  sensor_data_t *pa_reading = g_reading.data;

  for (int i = 0; i < SENSOR_COUNT_MAX; i++)
  {
    if (p_sensor[i].type)  // 사용으로 설정되었는지지 확인
    {
      pa_reading[i].enable = 1;
    }
  }
    
  pa_reading[A1_TEMPERATURE].dataType = DATA_TYPE_F;
  pa_reading[A2_WIND_DIRECTION].dataType = DATA_TYPE_F;
  pa_reading[A3_WIND_SPEED].dataType = DATA_TYPE_F;
  pa_reading[A4_INSTANT_WIND_DIRECTION].dataType = DATA_TYPE_F;
  pa_reading[A5_INSTANT_WIND_SPEED].dataType = DATA_TYPE_F;
  pa_reading[A6_RAINFALL_DOT5_1MM].dataType = DATA_TYPE_I;
  pa_reading[A7_PRESSURE].dataType = DATA_TYPE_F;
  pa_reading[A8_RAIN_PRESENT].dataType = DATA_TYPE_B;
  pa_reading[A9_SNOW_DEPTH].dataType = DATA_TYPE_I;

  pa_reading[A10_RELATIVE_HUMIDITY].dataType = DATA_TYPE_F;
  pa_reading[A11_RAINFALL_DOT1MM].dataType = DATA_TYPE_F;
  pa_reading[B1_SOLAR_RADIATION].dataType = DATA_TYPE_F;
  pa_reading[B2_SUNSHINE_DURATION].dataType = DATA_TYPE_F;

  pa_reading[B3_GROUND_TEMPERATURE].dataType = DATA_TYPE_F;
  pa_reading[B4_SURFACE_TEMPERATURE].dataType = DATA_TYPE_F;
  pa_reading[B5_SOIL_TEMPERATURE_5CM].dataType = DATA_TYPE_F;
  pa_reading[B6_SOIL_TEMPERATURE_10CM].dataType = DATA_TYPE_F;

  pa_reading[B7_SOIL_TEMPERATURE_20CM].dataType = DATA_TYPE_F;
  pa_reading[B8_SOIL_TEMPERATURE_30CM].dataType = DATA_TYPE_F;
  pa_reading[B9_SOIL_TEMPERATURE_50CM].dataType = DATA_TYPE_F;
  pa_reading[B10_SOIL_TEMPERATURE_100CM].dataType = DATA_TYPE_F;

  pa_reading[B11_SOIL_TEMPERATURE_150CM].dataType = DATA_TYPE_F;
  pa_reading[B12_SOIL_TEMPERATURE_300CM].dataType = DATA_TYPE_F;
  pa_reading[B13_SOIL_TEMPERATURE_500CM].dataType = DATA_TYPE_F;
  pa_reading[C1_CLOUD_BASE1].dataType = DATA_TYPE_F;
  pa_reading[C2_CLOUD_BASE2].dataType = DATA_TYPE_F;
    }

    /**
     * @brief 250ms마다 측정
     */
    void measure_250ms(void)
    {
      uint8_t err_wind_spd;
      uint8_t err_wind_dir;
      sensor_t *p_sensor = g_sensor_copy;
      wind_t wind;
      float speed = 0.0f;
      float direction = 0.0f;
      sensor_data_t *pa_reading = g_reading.data;

      if (p_sensor[A2_WIND_DIRECTION].type || p_sensor[A3_WIND_SPEED].type)
      {
        direction =
            wind_read(g_sensor_driver[A2_WIND_DIRECTION], WIND_CHANNEL_DIRECTION, &err_wind_dir);
        pa_reading[A2_WIND_DIRECTION].data.f = direction;
        pa_reading[A2_WIND_DIRECTION].err = err_wind_dir;

        speed = wind_read(g_sensor_driver[A3_WIND_SPEED], WIND_CHANNEL_SPEED, &err_wind_spd);
        pa_reading[A3_WIND_SPEED].data.f = speed;
        pa_reading[A3_WIND_SPEED].err = err_wind_spd;

        wind.direction = direction;
        wind.speed = speed;

      //  wind_process_250ms(&wind, err_wind_spd, err_wind_dir);
      }
    }

    void measure_1s(DATE_TIME_BUF * ct)
    {
      bool bData;
      uint8_t read_err;
      uint16_t i;
      uint16_t sensor_cnt;
      int32_t iData;
      float adc;
      float fData;
      eSENSOR_MODEL_t model;
      eSENSOR_LIST_t sensor_type;
      sensor_data_t *pa_reading = g_reading.data;
      sensor_t *sensor = g_sensor_copy;

      sensor_cnt = _countof(g_sensor_copy);

      // AWS센서만 처리
      for (sensor_type = A1_TEMPERATURE; sensor_type <= I1_TACHOMETER;
           (eSENSOR_LIST_t)sensor_type++)
      {
        model = sensor[sensor_type].type;

        if (model)  // 모델이 존재하면 사용함을 의미
        {
          adc = 0;
          switch (sensor_type)
          {
            case A1_TEMPERATURE:
              adc = temperature_read(g_sensor_driver[A1_TEMPERATURE], &read_err);
              pa_reading[A1_TEMPERATURE].data.f = adc;
              pa_reading[A1_TEMPERATURE].err = read_err;
              break;
            case A6_RAINFALL_DOT5_1MM:
              iData = read_sensor_rain(g_sensor_driver[A6_RAINFALL_DOT5_1MM], &read_err);
              pa_reading[A6_RAINFALL_DOT5_1MM].data.i = iData;
              pa_reading[A6_RAINFALL_DOT5_1MM].err = read_err;
              break;
            case A7_PRESSURE:
              adc = read_sensor_barometer(g_sensor_driver[A7_PRESSURE], &read_err);
              pa_reading[A7_PRESSURE].data.f = adc;
              pa_reading[A7_PRESSURE].err = read_err;
              break;
            case A8_RAIN_PRESENT:
              bData = read_sensor_rainPresent(g_sensor_driver[A8_RAIN_PRESENT], &read_err);
              pa_reading[A8_RAIN_PRESENT].data.b = bData;
              pa_reading[A8_RAIN_PRESENT].err = read_err;
              break;
            case A9_SNOW_DEPTH:  // mm
              iData = read_sensor_snow(g_sensor_driver[A9_SNOW_DEPTH], &read_err);
              pa_reading[A9_SNOW_DEPTH].data.i = iData;
              pa_reading[A9_SNOW_DEPTH].err = read_err;
              break;
            case A10_RELATIVE_HUMIDITY:
              adc = read_sensor_humidity(g_sensor_driver[A10_RELATIVE_HUMIDITY], &read_err);
              pa_reading[A10_RELATIVE_HUMIDITY].data.f = adc;
              pa_reading[A10_RELATIVE_HUMIDITY].err = read_err;
              break;
            case B1_SOLAR_RADIATION:
              fData = read_sensor_solarRadiation(g_sensor_driver[B1_SOLAR_RADIATION], &read_err);
              pa_reading[B1_SOLAR_RADIATION].data.f = fData;
              pa_reading[B1_SOLAR_RADIATION].err = read_err;
              break;
            case B2_SUNSHINE_DURATION:
              fData = read_sensor_sunshine(g_sensor_driver[B2_SUNSHINE_DURATION], &read_err);
              pa_reading[B2_SUNSHINE_DURATION].data.f = fData;
              pa_reading[B2_SUNSHINE_DURATION].err = read_err;
              break;
            case B5_SOIL_TEMPERATURE_5CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B5_SOIL_TEMPERATURE_5CM], &read_err);
              pa_reading[B5_SOIL_TEMPERATURE_5CM].data.f = fData;
              pa_reading[B5_SOIL_TEMPERATURE_5CM].err = read_err;
              break;
            case B6_SOIL_TEMPERATURE_10CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B6_SOIL_TEMPERATURE_10CM], &read_err);
              pa_reading[B6_SOIL_TEMPERATURE_10CM].data.f = fData;
              pa_reading[B6_SOIL_TEMPERATURE_10CM].err = read_err;
              break;
            case B7_SOIL_TEMPERATURE_20CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B7_SOIL_TEMPERATURE_20CM], &read_err);
              pa_reading[B7_SOIL_TEMPERATURE_20CM].data.f = fData;
              pa_reading[B7_SOIL_TEMPERATURE_20CM].err = read_err;
              break;
            case B8_SOIL_TEMPERATURE_30CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B8_SOIL_TEMPERATURE_30CM], &read_err);
              pa_reading[B8_SOIL_TEMPERATURE_30CM].data.f = fData;
              pa_reading[B8_SOIL_TEMPERATURE_30CM].err = read_err;
              break;
            case B9_SOIL_TEMPERATURE_50CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B9_SOIL_TEMPERATURE_50CM], &read_err);
              pa_reading[B9_SOIL_TEMPERATURE_50CM].data.f = fData;
              pa_reading[B9_SOIL_TEMPERATURE_50CM].err = read_err;

              break;
            case B10_SOIL_TEMPERATURE_100CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B10_SOIL_TEMPERATURE_100CM], &read_err);
              pa_reading[B10_SOIL_TEMPERATURE_100CM].data.f = fData;
              pa_reading[B10_SOIL_TEMPERATURE_100CM].err = read_err;
              break;
            case B11_SOIL_TEMPERATURE_150CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B11_SOIL_TEMPERATURE_150CM], &read_err);
              pa_reading[B11_SOIL_TEMPERATURE_150CM].data.f = fData;
              pa_reading[B11_SOIL_TEMPERATURE_150CM].err = read_err;
              break;

            case B12_SOIL_TEMPERATURE_300CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B12_SOIL_TEMPERATURE_300CM], &read_err);
              pa_reading[B12_SOIL_TEMPERATURE_300CM].data.f = fData;
              pa_reading[B12_SOIL_TEMPERATURE_300CM].err = read_err;
              break;

            case B13_SOIL_TEMPERATURE_500CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B13_SOIL_TEMPERATURE_500CM], &read_err);
              pa_reading[B13_SOIL_TEMPERATURE_500CM].data.f = fData;
              pa_reading[B13_SOIL_TEMPERATURE_500CM].err = read_err;
              break;
          }
        }
      }
    }

#define AWS_OLD //250ms 마다 측정해서 dualTask에서 처리
void measureTask(void *arg)
{
  uint32_t tick_count;

  DATE_TIME_BUF ct;
  DATE_TIME_BUF ot;


  os_logging_printf("measure task");

  /*
  config의 복사본으로 동작시킨다.
  config변경해도 동작에 영향이 없도록 한다.
  설정값 변경후 장비를 리셋해야 한다다.
  */


  sensor_init();

  ct = Date_Time;
  ot = ct;

  tick_count = osKernelGetTickCount();

#ifndef AWS_OLD
  while (1)
  {
    g_debug_start_time = mcu_get_clk();

    ct = Date_Time;

    measure_250ms(&ct);
    g_reading.type = eMEASURE_TYPE_250MS;
    send_measurement(&g_reading);

    if (ct.Sec != ot.Sec)
    {
      measure_1s(&ct);
      ot.Sec = ct.Sec;
      g_reading.type = eMEASURE_TYPE_1000MS;
      send_measurement(&g_reading);
    }

    g_debug_elased_time = cal_elapsed_us(g_debug_start_time);
    if (g_debug_elased_time > g_debug_elased_max)
    {
      g_debug_elased_max = g_debug_elased_time;
    }
    tick_count += MEASURE_PERIOD_MS;

    osDelayUntil(tick_count);  // 남은 지연 시간만큼 지연
  }

#else
  while (1)
  {
    g_debug_start_time = mcu_get_clk();

    measure_250ms();
    measure_1s(&ct);//함수 이름만 1s이지 호출이 250ms 임

    g_debug_elased_time = cal_elapsed_us(g_debug_start_time);
    if (g_debug_elased_time > g_debug_elased_max)
    {
      g_debug_elased_max = g_debug_elased_time;
    }
    tick_count += MEASURE_PERIOD_MS;

  //  send_measurement(&g_reading);
    osDelayUntil(tick_count);  // 남은 지연 시간만큼 지연
  }
#endif

}





void measureTask_init(void)
{
  osThreadId_t thread_id;

  g_measure_queue = osMessageQueueNew(1, sizeof(measure_data_t), NULL);

  assert_param(g_measure_queue);

  thread_id = osThreadNew(measureTask, NULL, &kMeasureTask_attributes);

  assert_param(thread_id);
}