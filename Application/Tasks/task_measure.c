/**
 * @file           task_measure.c
 * @brief          하드웨어가 제공가능한 센서를 250ms,1초마다 측정
 * @author         t
 * @date           2025-01-01
 * @version        v1.0.0
 *
 * @note
 *  - 센서 값은 250ms,1초 주기로 읽어 메시지 큐로 전달됨
 *
 * @details
 * v1.0.0 2025-04-15
 * 
 * 
 * 센서가 추가되면 해야 할것
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

#include "app_dataLogging.h"
#include "app_file.h"
#include "app_measure.h"
#include "bsp.h"
#include "aws_data.h"
#include "cmsis_os2.h"
#include "config_app.h"
#include "driver_do.h"
#include "task_logging.h"
#include "bsp_delay.h"
#include "util_memory.h"
#include "util_time.h"
#include "aws_processor.h"
#include "os_user_def.h"

#define MEASURE_PERIOD_250MS 250
#define MEASURE_PERIOD_1000MS 1000


const osThreadAttr_t kMeasure250msTask_attributes = {
    .name = "measure250msTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityRealtime,
};

const osThreadAttr_t kMeasure1sTask_attributes = {
    .name = "measure1sTask",
    .stack_size = 2048,
    .priority = (osPriority_t)osPriorityRealtime,
};


const uint32_t kMesaureTimeOutMs = 50;
static sensor_t g_sensor_config_bk[SENSOR_LIST_MAX];  // config 센서의 복사본
static driver_t *g_sensor_driver[SENSOR_LIST_MAX];


osMessageQueueId_t g_reading_250ms_queue;
osMessageQueueId_t g_reading_1s_queue;

measure_data_250ms_t g_reading_250;//Task 실행 시간 측정용
measure_data_1s_t g_reading_1;//Task 실행 시간 측정용
exec_time_t g_exec_250ms_time; //Task 실행 시간 측정용
exec_time_t g_exec_1s_time;//Task 실행 시간 측정용

driver_t *get_sensor_driver(eSENSOR_TYPE_t sensor)
{
  return g_sensor_driver[sensor];
}

    // Task 실행 시간 측정용
    void elapse_start(exec_time_t *p_time)
{ 
  p_time->start_time = HAL_GetTick(); 
}

//Task 실행 시간 측정용
void elapse_stop(exec_time_t *p_time)
{
  p_time->elapsed_time = HAL_GetTick() - p_time->start_time;
  if (p_time->elapsed_time > p_time->elapsed_max)
  {
    p_time->elapsed_max = p_time->elapsed_time;
  }
}


/**
 * @brief 측정 데이터 전송
 */
void send_measurement(void *queue,void *data)
{
  osStatus_t status;

  status = osMessageQueuePut(queue, data, 0, kMesaureTimeOutMs);

  if(status != osOK)
  {
    task_printf("send_measureData fail %s\r\n", osStatusToStr(status));
  }
}

/**
 * @brief 측정 데이터 확인
 */
bool is_measurement_250(void *data,uint32_t timeout)
{
  osStatus_t status;

  if (g_reading_250ms_queue == NULL)
  {
    osDelay(100);
    return false;
  }

  status = osMessageQueueGet(g_reading_250ms_queue, data, NULL, timeout);

  if (status != osOK)
  {
    io_printf("is_measurement fail %d\r\n", status);
    return false;
  }

  return true;
}

bool is_measurement_1s( void *data,uint32_t timeout)
{
  osStatus_t status;
  if (g_reading_1s_queue == NULL)
  {
    osDelay(100);
    return false;
  }

  status = osMessageQueueGet(g_reading_1s_queue, data, NULL, timeout);

  (void)status;
  
  return true;
}

/**
 *  센서 모델과 타입에 해당하는 드라이버 번호를 넘겨준다.
 *  예)
 *  measure테스트가 제공가능한 센서목록이 있고
 *  센서에 해당하는 실제 모델이 있다. 그것을 연결시켜준다
 *  type:온도센서 -> pt100 을 사용하겠다.
 * 
 *  타입:물리적인 값의 구분(온도,습도 등등)
 *  모델:타임의 모델(ADC형, 시리얼 타입형 등등)
 *  
 * */
// ADDMODEL:센서 추가시 수정해야함
int32_t get_driver_number(eSENSOR_TYPE_MODEL_t type)
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
    case S_T_SNOW_HJ:
      num = SNOW_HJ;
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
    case S_T_TEMPERATURE_HJ:
      num = TEMP_HJ_TEMPERATURE;
      break;
    case S_T_HUMINITY_HJ:
      num = TEMP_HJ_HUMINITY;
     break;
    case S_T_SOLAR_RADIATION_OTT_SMP3:
      num = OTT_SMP3_MODBUS;
     break;
    case S_T_FREQ:
      num = GENERAL_FREQ;
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

  //프로그램 실행 중 설정값 변경되어도 영향 없도록 측정 Task는 설정값 복사본으로 동작
  memcpy(g_sensor_config_bk, config.sensor, sizeof(g_sensor_config_bk));

  p_sensor = g_sensor_config_bk;

  adc_init();  // ADC 항상 초기화

  //사용하는 센서의 드라이버를 초기화 한다.
  for (int i = 0; i < SENSOR_LIST_MAX; i++)
  {
    if (p_sensor[i].type)  // 0이 아니면 사용으로 설정된것
    {
      switch (i)
      {
        case A1_TEMPERATURE:
          num = get_driver_number(p_sensor[A1_TEMPERATURE].type);
          para = get_sensor_config(&p_sensor[A1_TEMPERATURE]);
          g_sensor_driver[A1_TEMPERATURE] = temperature_open(num, para);
          break;
        case A10_RELATIVE_HUMIDITY:
          num = get_driver_number(p_sensor[A10_RELATIVE_HUMIDITY].type);
          para = get_sensor_config(&p_sensor[A10_RELATIVE_HUMIDITY]);
          g_sensor_driver[A10_RELATIVE_HUMIDITY] = humidity_open(num, para);
          break;
        case A3_WIND_SPEED:
          num = get_driver_number(p_sensor[A3_WIND_SPEED].type);
          para = get_sensor_config(&p_sensor[A3_WIND_SPEED]);
          g_sensor_driver[A3_WIND_SPEED] = windSpeed_open(num, para);
          break;
        case A2_WIND_DIRECTION:
          num = get_driver_number(p_sensor[A2_WIND_DIRECTION].type);
          para = get_sensor_config(&p_sensor[A2_WIND_DIRECTION]);
          g_sensor_driver[A2_WIND_DIRECTION] = windSpeed_open(num, para);
          break;
        case A9_SNOW_DEPTH:
          num = get_driver_number(p_sensor[A9_SNOW_DEPTH].type);
          para = get_sensor_config(&p_sensor[A9_SNOW_DEPTH]);
          g_sensor_driver[A9_SNOW_DEPTH] = snow_open(num, para);
          break;
        case A6_RAINFALL_DOT5_1MM:
          num = get_driver_number(p_sensor[A6_RAINFALL_DOT5_1MM].type);
          g_sensor_driver[A6_RAINFALL_DOT5_1MM] = rain_open(num, 0);
          break;
        case A8_RAIN_PRESENT:
          g_sensor_driver[A8_RAIN_PRESENT] = rainPresent_open(RAIN_PRESENT_DI, 0);
          break;
        case A7_PRESSURE:
          num = get_driver_number(p_sensor[A7_PRESSURE].type);
          para = get_sensor_config(&p_sensor[A7_PRESSURE]);
          g_sensor_driver[A7_PRESSURE] = barometer_open(num, para);
          break;
        
        case B5_SOIL_TEMPERATURE_5CM:
          num = get_driver_number(p_sensor[B5_SOIL_TEMPERATURE_5CM].type);
          para = get_sensor_config(&p_sensor[B5_SOIL_TEMPERATURE_5CM]);
          g_sensor_driver[B5_SOIL_TEMPERATURE_5CM] = barometer_open(num, para);
          break;
        case B6_SOIL_TEMPERATURE_10CM:
          num = get_driver_number(p_sensor[B6_SOIL_TEMPERATURE_10CM].type);
          para = get_sensor_config(&p_sensor[B6_SOIL_TEMPERATURE_10CM]);
          g_sensor_driver[B6_SOIL_TEMPERATURE_10CM] = barometer_open(num, para);
          break;
        case B7_SOIL_TEMPERATURE_20CM:
          num = get_driver_number(p_sensor[B7_SOIL_TEMPERATURE_20CM].type);
          para = get_sensor_config(&p_sensor[B7_SOIL_TEMPERATURE_20CM]);
          g_sensor_driver[B7_SOIL_TEMPERATURE_20CM] = barometer_open(num, para);
          break;
        case B8_SOIL_TEMPERATURE_30CM:
          num = get_driver_number(p_sensor[B8_SOIL_TEMPERATURE_30CM].type);
          para = get_sensor_config(&p_sensor[B8_SOIL_TEMPERATURE_30CM]);
          g_sensor_driver[B8_SOIL_TEMPERATURE_30CM] = barometer_open(num, para);
          break;
        case B9_SOIL_TEMPERATURE_50CM:
          num = get_driver_number(p_sensor[B9_SOIL_TEMPERATURE_50CM].type);
          para = get_sensor_config(&p_sensor[B9_SOIL_TEMPERATURE_50CM]);
          g_sensor_driver[B9_SOIL_TEMPERATURE_50CM] = barometer_open(num, para);
          break;
        case B10_SOIL_TEMPERATURE_100CM:
          num = get_driver_number(p_sensor[B10_SOIL_TEMPERATURE_100CM].type);
          para = get_sensor_config(&p_sensor[B10_SOIL_TEMPERATURE_100CM]);
          g_sensor_driver[B10_SOIL_TEMPERATURE_100CM] = barometer_open(num, para);
          break;
        case B11_SOIL_TEMPERATURE_150CM:
          num = get_driver_number(p_sensor[B11_SOIL_TEMPERATURE_150CM].type);
          para = get_sensor_config(&p_sensor[B11_SOIL_TEMPERATURE_150CM]);
          g_sensor_driver[B11_SOIL_TEMPERATURE_150CM] = barometer_open(num, para);
          break;
        case B12_SOIL_TEMPERATURE_300CM:
          num = get_driver_number(p_sensor[B12_SOIL_TEMPERATURE_300CM].type);
          para = get_sensor_config(&p_sensor[B12_SOIL_TEMPERATURE_300CM]);
          g_sensor_driver[B12_SOIL_TEMPERATURE_300CM] = barometer_open(num, para);
          break;
        case B13_SOIL_TEMPERATURE_500CM:
          num = get_driver_number(p_sensor[B13_SOIL_TEMPERATURE_500CM].type);
          para = get_sensor_config(&p_sensor[B13_SOIL_TEMPERATURE_500CM]);
          g_sensor_driver[B13_SOIL_TEMPERATURE_500CM] = barometer_open(num, para);
          break;
        case B2_SUNSHINE_DURATION:
          num = get_driver_number(p_sensor[B2_SUNSHINE_DURATION].type);
          para = get_sensor_config(&p_sensor[B2_SUNSHINE_DURATION]);
          g_sensor_driver[B2_SUNSHINE_DURATION] = sunshine_open(num, para);
          break;
        case B1_SOLAR_RADIATION:
          num = get_driver_number(p_sensor[B1_SOLAR_RADIATION].type);
          para = get_sensor_config(&p_sensor[B1_SOLAR_RADIATION]);
          g_sensor_driver[B1_SOLAR_RADIATION] = solar_radiation_open(num, para);
          break;
        case N10_AIR_TEMPERATURE_50CM:
          num = get_driver_number(p_sensor[N10_AIR_TEMPERATURE_50CM].type);
          g_sensor_driver[N10_AIR_TEMPERATURE_50CM] = temperature_open(num, 0);
          break;
        default://현재 구현되어 있지 않은 센서 드라이버는 ADC만 사용하도록함
        num = get_driver_number(p_sensor[i].type);
        para = get_sensor_config(&p_sensor[i]);
        g_sensor_driver[i] = general_adc_open(num, para);
        break;
      }
    }
  }

  //센서사용 여부를 업데이트한다.
  for (int i = 0; i < SENSOR_LIST_MAX; i++)
  { 
    if (get_config_app()->sensor[i].type)  // 사용으로 설정되었는지 확인
    {
      g_reading_1.data[i].enable = 1;  // 풍향,풍속은 사용하지 않는다.

      switch (i)
      {
        case A2_WIND_DIRECTION:
          g_reading_250.data[eA2_WIND_DIRECTION].enable = true;
          g_reading_250.data[eA2_WIND_DIRECTION].data_type = eDATA_TYPE_F;
          break;
        case A3_WIND_SPEED:
          g_reading_250.data[eA3_WIND_SPEED].enable = true;
          g_reading_250.data[eA3_WIND_SPEED].data_type = eDATA_TYPE_F;
          break;
        }
    }


  }

  //센서 데이터의 타입을 정의한다.
  g_reading_1.data[A1_TEMPERATURE].data_type = eDATA_TYPE_F;
  g_reading_1.data[A2_WIND_DIRECTION].data_type = eDATA_TYPE_F;
  g_reading_1.data[A3_WIND_SPEED].data_type = eDATA_TYPE_F;
  g_reading_1.data[A6_RAINFALL_DOT5_1MM].data_type = eDATA_TYPE_F;
  g_reading_1.data[A7_PRESSURE].data_type = eDATA_TYPE_F;
  g_reading_1.data[A8_RAIN_PRESENT].data_type = eDATA_TYPE_B;
  g_reading_1.data[A9_SNOW_DEPTH].data_type = eDATA_TYPE_I;
  g_reading_1.data[A10_RELATIVE_HUMIDITY].data_type = eDATA_TYPE_F;
  g_reading_1.data[A11_RAINFALL_DOT1MM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B1_SOLAR_RADIATION].data_type = eDATA_TYPE_F;
  g_reading_1.data[B2_SUNSHINE_DURATION].data_type = eDATA_TYPE_F;
  g_reading_1.data[B3_GROUND_TEMPERATURE].data_type = eDATA_TYPE_F;
  g_reading_1.data[B4_SURFACE_TEMPERATURE].data_type = eDATA_TYPE_F;
  g_reading_1.data[B5_SOIL_TEMPERATURE_5CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B6_SOIL_TEMPERATURE_10CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B7_SOIL_TEMPERATURE_20CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B8_SOIL_TEMPERATURE_30CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B9_SOIL_TEMPERATURE_50CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B10_SOIL_TEMPERATURE_100CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B11_SOIL_TEMPERATURE_150CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B12_SOIL_TEMPERATURE_300CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[B13_SOIL_TEMPERATURE_500CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[C1_CLOUD_BASE1].data_type = eDATA_TYPE_F;
  g_reading_1.data[C2_CLOUD_BASE2].data_type = eDATA_TYPE_F;
  g_reading_1.data[C3_CLOUD_BASE3].data_type = eDATA_TYPE_F;
  g_reading_1.data[C4_CLOUD_COVER].data_type = eDATA_TYPE_F;
  g_reading_1.data[C5_VISIBILITY].data_type = eDATA_TYPE_F;
  g_reading_1.data[C6_PM10].data_type = eDATA_TYPE_F;
  g_reading_1.data[C7_PM2DOT5].data_type = eDATA_TYPE_F;
  g_reading_1.data[C8_NET_RADIATION].data_type = eDATA_TYPE_F;
  g_reading_1.data[C9_TOTAL_RADIATION].data_type = eDATA_TYPE_F;
  g_reading_1.data[C10_REFLECTED_RADIATION].data_type = eDATA_TYPE_F;
  g_reading_1.data[C11_DIRECT_SOLAR].data_type = eDATA_TYPE_F;
  g_reading_1.data[C12_CURRENT_WEATHER].data_type = eDATA_TYPE_F;
  g_reading_1.data[N1_SOIL_MOISTURE_10CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N2_SOIL_MOISTURE_20CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N3_SOIL_MOISTURE_30CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N4_SOIL_MOISTURE_50CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N5_ILLUMINANCE].data_type = eDATA_TYPE_F;
  g_reading_1.data[N6_WIND_VELOCITY_150CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N7_WIND_VELOCITY_400CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N8_INSTANT_VELOCITY_150CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N9_INSTANT_VELOCITY_400CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N10_AIR_TEMPERATURE_50CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N11_AIR_TEMPERATURE_400CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N12_HUMIDITY_50CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[N13_HUMIDITY_400CM].data_type = eDATA_TYPE_F;
  g_reading_1.data[I1_TACHOMETER].data_type = eDATA_TYPE_F;

  }

/**
 * @brief 250ms마다 측정
 */
void measure_250ms(void)
{
  uint8_t err_wind_spd=0;
  uint8_t err_wind_dir=0;
  float speed = 0.0f;
  float direction = 0.0f;
  sensor_t *p_sensor_cfg = g_sensor_config_bk;
 
  sensor_data_t *p_reading_250ms = g_reading_250.data;


  if (p_sensor_cfg[A3_WIND_SPEED].type)
  {
    speed = wind_read(g_sensor_driver[A3_WIND_SPEED], WIND_CHANNEL_SPEED, &err_wind_spd);
    p_reading_250ms[eA3_WIND_SPEED].data.f = speed;
    p_reading_250ms[eA3_WIND_SPEED].err = err_wind_spd;
  }

  if (p_sensor_cfg[A2_WIND_DIRECTION].type)
  {
    direction =
    wind_read(g_sensor_driver[A2_WIND_DIRECTION], WIND_CHANNEL_DIRECTION, &err_wind_dir);
    p_reading_250ms[eA2_WIND_DIRECTION].data.f = direction;
    p_reading_250ms[eA2_WIND_DIRECTION].err = err_wind_dir;

  }
}

void measure_1s(void)
{
  bool bData;
  uint8_t read_err;
  int32_t iData;
  float adc;
  float fData;
  eSENSOR_TYPE_MODEL_t model;
  eSENSOR_TYPE_t sensor_type;
  sensor_data_t *pa_reading_1s = g_reading_1.data;
  sensor_t *sensor = g_sensor_config_bk;


      // AWS센서만 처리
      for (sensor_type = A1_TEMPERATURE; sensor_type <= I1_TACHOMETER;
           (eSENSOR_TYPE_t)sensor_type++)
      {
        model = sensor[sensor_type].type;

        if (model)  // 모델이 존재하면 사용함을 의미
        {
          adc = 0;
          switch (sensor_type)
          {
            case A1_TEMPERATURE:
              adc = temperature_read(g_sensor_driver[A1_TEMPERATURE], &read_err);
              pa_reading_1s[A1_TEMPERATURE].data.f = adc + sensor[sensor_type].offset;
              pa_reading_1s[A1_TEMPERATURE].err = read_err;
              break;
            case A6_RAINFALL_DOT5_1MM:
              fData = read_sensor_rain(g_sensor_driver[A6_RAINFALL_DOT5_1MM], &read_err);
              pa_reading_1s[A6_RAINFALL_DOT5_1MM].data.f = fData;
              pa_reading_1s[A6_RAINFALL_DOT5_1MM].err = read_err;
              break;
            case A7_PRESSURE:
              adc = read_sensor_barometer(g_sensor_driver[A7_PRESSURE], &read_err);
              pa_reading_1s[A7_PRESSURE].data.f = adc + sensor[sensor_type].offset;
              pa_reading_1s[A7_PRESSURE].err = read_err;
              break;
            case A8_RAIN_PRESENT:
              bData = read_sensor_rainPresent(g_sensor_driver[A8_RAIN_PRESENT], &read_err);
              pa_reading_1s[A8_RAIN_PRESENT].data.b = bData;
              pa_reading_1s[A8_RAIN_PRESENT].err = read_err;
              break;
            case A9_SNOW_DEPTH:  // mm
              iData = read_sensor_snow(g_sensor_driver[A9_SNOW_DEPTH], &read_err);
              pa_reading_1s[A9_SNOW_DEPTH].data.i = iData;
              pa_reading_1s[A9_SNOW_DEPTH].err = read_err;
              break;
            case A10_RELATIVE_HUMIDITY:
              adc = read_sensor_humidity(g_sensor_driver[A10_RELATIVE_HUMIDITY], &read_err);
              pa_reading_1s[A10_RELATIVE_HUMIDITY].data.f = adc + sensor[sensor_type].offset;
              pa_reading_1s[A10_RELATIVE_HUMIDITY].err = read_err;
              break;
            case B1_SOLAR_RADIATION:
              fData = read_sensor_solarRadiation(g_sensor_driver[B1_SOLAR_RADIATION], &read_err);
              pa_reading_1s[B1_SOLAR_RADIATION].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B1_SOLAR_RADIATION].err = read_err;
              break;
            case B2_SUNSHINE_DURATION:
              fData = read_sensor_sunshine(g_sensor_driver[B2_SUNSHINE_DURATION], &read_err);
              pa_reading_1s[B2_SUNSHINE_DURATION].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B2_SUNSHINE_DURATION].err = read_err;
              break;
            case B5_SOIL_TEMPERATURE_5CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B5_SOIL_TEMPERATURE_5CM], &read_err);
              pa_reading_1s[B5_SOIL_TEMPERATURE_5CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B5_SOIL_TEMPERATURE_5CM].err = read_err;
              break;
            case B6_SOIL_TEMPERATURE_10CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B6_SOIL_TEMPERATURE_10CM], &read_err);
              pa_reading_1s[B6_SOIL_TEMPERATURE_10CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B6_SOIL_TEMPERATURE_10CM].err = read_err;
              break;
            case B7_SOIL_TEMPERATURE_20CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B7_SOIL_TEMPERATURE_20CM], &read_err);
              pa_reading_1s[B7_SOIL_TEMPERATURE_20CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B7_SOIL_TEMPERATURE_20CM].err = read_err;
              break;
            case B8_SOIL_TEMPERATURE_30CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B8_SOIL_TEMPERATURE_30CM], &read_err);
              pa_reading_1s[B8_SOIL_TEMPERATURE_30CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B8_SOIL_TEMPERATURE_30CM].err = read_err;
              break;
            case B9_SOIL_TEMPERATURE_50CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B9_SOIL_TEMPERATURE_50CM], &read_err);
              pa_reading_1s[B9_SOIL_TEMPERATURE_50CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B9_SOIL_TEMPERATURE_50CM].err = read_err;

              break;
            case B10_SOIL_TEMPERATURE_100CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B10_SOIL_TEMPERATURE_100CM], &read_err);
              pa_reading_1s[B10_SOIL_TEMPERATURE_100CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B10_SOIL_TEMPERATURE_100CM].err = read_err;
              break;
            case B11_SOIL_TEMPERATURE_150CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B11_SOIL_TEMPERATURE_150CM], &read_err);
              pa_reading_1s[B11_SOIL_TEMPERATURE_150CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B11_SOIL_TEMPERATURE_150CM].err = read_err;
              break;

            case B12_SOIL_TEMPERATURE_300CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B12_SOIL_TEMPERATURE_300CM], &read_err);
              pa_reading_1s[B12_SOIL_TEMPERATURE_300CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B12_SOIL_TEMPERATURE_300CM].err = read_err;
              break;

            case B13_SOIL_TEMPERATURE_500CM:
              fData = read_sensor_soilTemp(g_sensor_driver[B13_SOIL_TEMPERATURE_500CM], &read_err);
              pa_reading_1s[B13_SOIL_TEMPERATURE_500CM].data.f = fData + sensor[sensor_type].offset;
              pa_reading_1s[B13_SOIL_TEMPERATURE_500CM].err = read_err;
              break;
            default:

            fData = general_adc_read(g_sensor_driver[sensor_type],&read_err);
            pa_reading_1s[sensor_type].data.f = fData+ sensor[sensor_type].offset;
            break;
          }
        }
      }
    }





void measure250ms_task(void *arg)
{
  uint32_t tick_count;

  tick_count = osKernelGetTickCount();
  while(1)
  {
    elapse_start(&g_exec_250ms_time);
    measure_250ms();
    elapse_stop(&g_exec_250ms_time);
    send_measurement(g_reading_250ms_queue, &g_reading_250);
    
    tick_count += MEASURE_PERIOD_250MS;
    osDelayUntil(tick_count);  
  }
}


void measure1s_task(void *arg)
{
  uint32_t tick_count;

  tick_count = osKernelGetTickCount();
  while(1)
  {
    elapse_start(&g_exec_1s_time);
    measure_1s();
    elapse_stop(&g_exec_1s_time);
    send_measurement(g_reading_1s_queue, &g_reading_1);
    tick_count += MEASURE_PERIOD_1000MS;
    osDelayUntil(tick_count);
  }
}

/**
 * @brief 250ms,1s 마다 센서 데이터 수집
 * 250ms 풍향 풍속 전용으로 처리
 * 1s는 일반 센서처리
 */
void measureTask_init(void)
{
  osThreadId_t thread_id;

  sensor_init();

  g_reading_250ms_queue = osMessageQueueNew(1, sizeof(measure_data_250ms_t), NULL);
  g_reading_1s_queue = osMessageQueueNew(1, sizeof(measure_data_1s_t), NULL);

  thread_id = osThreadNew(measure250ms_task, NULL, &kMeasure250msTask_attributes);
  assert_param(thread_id);

  thread_id = osThreadNew(measure1s_task, NULL, &kMeasure1sTask_attributes);
  assert_param(thread_id);
  }
