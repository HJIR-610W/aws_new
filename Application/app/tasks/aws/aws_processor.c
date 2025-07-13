




#include <math.h>

#include "cmsis_os2.h"
#include "aws_processor.h"
#include "util_memory.h"
#include "util_time.h"
#include "aws_data.h"

#include "task_measure.h"
#include "user_heap.h"

#include "os_user_def.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define DEG2RAD (M_PI / 180.0)
#define RAD2DEG (180.0 / M_PI)

#define SAMPLING_INTERVAL_MS 250  // 0.25초 (밀리초 단위)
#define SAMPLES_PER_GUST 12  // 3초 / 0.25초
#define SAMPLES_PER_MINUTE 240 
#define MINUTES_PER_10MIN 10
#define MINUTES_PER_DAY 1440   //하루 최대 풍향 풍속




static windVector_t g_inst_vector_buffer[SAMPLES_PER_GUST];
static uint8_t g_inst_vector_err[SAMPLES_PER_GUST];
static uint32_t g_inst_buffer_idx = 0;
static uint32_t g_inst_sample_count = 0;

static windVector_t g_1min_vector_buffer[SAMPLES_PER_MINUTE];
static uint8_t g_1min_vector_err[SAMPLES_PER_GUST];
static uint32_t g_1min_buffer_idx = 0;

static wind_t g_1min_gust_buffer[SAMPLES_PER_MINUTE];
static uint8_t g_1min_gust_err[SAMPLES_PER_MINUTE];
static uint32_t g_1min_gust_buffer_idx = 0;

static wind_t g_10min_max_gust_buffer[MINUTES_PER_10MIN];
static uint32_t g_10min_buffer_idx = 0;
static uint32_t g_minute_counter_for_10min = 0;

static windVector_t g_current_daily_max_gust = {0, 0};
static uint32_t g_minute_counter_for_day = 0;

wind_t avg_wind;
wind_t wind_gust_inst;
wind_t g_wind_gust_1min;




//온도
#define TEMP_SAMPLES_PER_MINUTE 6
static uint16_t g_1min_temp_avg[TEMP_SAMPLES_PER_MINUTE];
static uint8_t g_1min_temp_avg_idx=0;
void calculate_windToVector(wind_t* wind, windVector_t* windVector);

void wind1min_add_sample(wind_t* wind, uint8_t *err)
{
    windVector_t windVector;

  // 풍향 풍속을 벡터로 변환한 뒤 저장
  calculate_windToVector(wind, &windVector);
  
  
  CIRCULAR_PUSH(g_1min_vector_buffer, g_1min_buffer_idx, windVector,
                _countof(g_1min_vector_buffer));
}



void calculate_avgWind(uint32_t N, float* ws, float* wd, float* composite_speed, float* wind_dir)
{
  float sum_u = 0.0f, sum_v = 0.0f;

  for (uint32_t i = 0; i < N; i++)
  {
    float theta_rad = (90 - wd[i]) * DEG2RAD;
    float u = ws[i] * cosf(theta_rad);
    float v = ws[i] * sinf(theta_rad);

    sum_u += u;
    sum_v += v;

   // printf("ws=%6.2f wd=%6.2f => u=%10.4f v=%10.4f\n", ws[i], wd[i], u, v);
  }

  float avg_u = sum_u / N;
  float avg_v = sum_v / N;

  //printf("평균 u=%.4f v=%.4f\n", avg_u, avg_v);

  float speed = sqrtf(avg_u * avg_u + avg_v * avg_v);

  *composite_speed = speed;

  if (speed < 0.01f)  // 0.01m/s 는 그냥 0으로 처리
  {
    *wind_dir = 0.0f;  // 무풍이면 풍향은 의미 없음
    //printf("[풍속 %f]\r\n", speed);
  }
  else
  {
    float angle_rad = atan2f(avg_v, avg_u);

    *wind_dir = fmodf((450.0f - angle_rad * RAD2DEG), 360.0f);
  }
}

void calculate_windToVector(wind_t *wind, windVector_t *windVector)
{
 float theta_rad = (90.0 - wind->direction) * DEG2RAD;
 windVector->u = wind->speed * cosf(theta_rad);
 windVector->v = wind->speed * sinf(theta_rad);
}


void calculate_vectorToWin(windVector_t* windVector, wind_t *wind )
{


  float speed = sqrtf(windVector->u * windVector->u + windVector->v * windVector->v);

  wind->speed = speed;

  if (speed < 0.01f)  // 0.01m/s 는 그냥 0으로 처리
  {
    wind->direction = 0.0f;  // 무풍이면 풍향은 의미 없음
  }
  else
  {
    float angle_rad = atan2f(windVector->v, windVector->u);

    wind->direction = fmodf((450.0f - angle_rad * RAD2DEG), 360.0f);
  }
}

windVector_t calculate_average_vector(const windVector_t* buffer, uint32_t count)
{
  windVector_t avg_vector = {0.0f, 0.0f};
  if (!buffer || count == 0)
    return avg_vector;

  double sum_u = 0.0;  // Use double for sum to avoid precision loss
  double sum_v = 0.0;
  for (uint32_t i = 0; i < count; ++i)
  {
    sum_u += buffer[i].u;
    sum_v += buffer[i].v;
  }
  avg_vector.u = (float)(sum_u / count);
  avg_vector.v = (float)(sum_v / count);
  return avg_vector;
}




/**
 * @brief 풍향,풍속을 벡터변환하여 저장장
 */
void windInst_add_sample(wind_t* wind, uint8_t *err)
{
  windVector_t windVector;


  // 풍향 풍속을 벡터로 변환한 뒤 저장
  calculate_windToVector(wind, &windVector);

  CIRCULAR_PUSH2(g_inst_vector_buffer, g_inst_buffer_idx, windVector,
                 _countof(g_inst_vector_buffer));


}

//순간 풍향 풍속
wind_t calculate_windInst(void)
{

wind_t wind;
  wind.direction = 0;
  wind.speed = 0;

  if (g_inst_sample_count >= SAMPLES_PER_GUST)
  {
    windVector_t avg_inst_vector = calculate_average_vector(g_inst_vector_buffer, SAMPLES_PER_GUST);
    calculate_vectorToWin(&avg_inst_vector, &wind);
  }
  else
  {
    windVector_t avg_inst_vector =
        calculate_average_vector(g_inst_vector_buffer, g_inst_sample_count);
    calculate_vectorToWin(&avg_inst_vector, &wind);
  }

  return wind;
}



// 순간 풍향 풍속
wind_t calculate_wind1min(void)
{
  wind_t wind;

  wind.direction = 0;
  wind.speed = 0;

  if (g_1min_buffer_idx >= SAMPLES_PER_MINUTE)
  {
    windVector_t avg_1min_vector =
        calculate_average_vector(g_1min_vector_buffer, SAMPLES_PER_MINUTE);
    calculate_vectorToWin(&avg_1min_vector, &wind);
  }
  else
  {
    windVector_t avg_1min_vector =
        calculate_average_vector(g_1min_vector_buffer, g_1min_buffer_idx);
    calculate_vectorToWin(&avg_1min_vector, &wind);
  }

  return wind;
}

wind_t find_max_gust(const wind_t* buffer, uint32_t count)
{ 
  wind_t max_gust = {0, 0};


  max_gust = buffer[0];

  for (uint32_t i = 1; i < count; ++i)
  {
    if (buffer[i].speed > max_gust.speed)
    {
      max_gust = buffer[i];
    }

  }
  return max_gust;
}

// 매 1분마다 지난 10개의 1분값 중에서 최댓값을 10분 최대순간풍향ㆍ풍속으로 산출한다
void wind10min_add_gust(wind_t *wind)
{
  CIRCULAR_PUSH2(g_10min_max_gust_buffer, g_10min_buffer_idx, *wind,
                 _countof(g_10min_max_gust_buffer));
}

//1분마다 이함수 호출시 최근 10분값 10개 값중 최댓값 산출
wind_t find_max_10min_gust(void)
{
  return find_max_gust(g_10min_max_gust_buffer,_countof(g_10min_max_gust_buffer));
}

void wind_process_250ms(wind_t* wind_sample, uint8_t spd_err, uint8_t dir_err)
{
  windVector_t current_vector;
  wind_t wind;


  if(spd_err || dir_err)
  {
    wind.direction = 0.0f;
    wind.speed = 0.0f;
  }

  // 풍향 풍속을 벡터로 변환
  calculate_windToVector(&wind, &current_vector);

  //1분 평균
  g_1min_vector_buffer[g_1min_buffer_idx] = current_vector;

  //3초 순간 풍향 풍속 250ms 샘플


  CIRCULAR_PUSH2(g_inst_vector_buffer, g_inst_buffer_idx, current_vector, SAMPLES_PER_GUST);

  //매 0.25초 간격으로 3초 동안 12개의 샘플링 된 자료를 평균하고 0.25초 간격으로 이동평균하여
  //순간풍향ㆍ풍속을 산출한다.
  //250ms 이동해가면서 최근 12샘플자료를 평균한다.
  if (g_inst_buffer_idx >= SAMPLES_PER_GUST)
  {
    windVector_t avg_inst_vector = calculate_average_vector(g_inst_vector_buffer, SAMPLES_PER_GUST);
    calculate_vectorToWin(&avg_inst_vector, &avg_wind);
  }
  else
  {
    windVector_t avg_inst_vector =
        calculate_average_vector(g_inst_vector_buffer, g_inst_sample_count);
    calculate_vectorToWin(&avg_inst_vector, &avg_wind);
  }


  g_1min_gust_buffer[g_1min_gust_buffer_idx] = avg_wind;

   g_inst_buffer_idx = (g_inst_buffer_idx + 1) % SAMPLES_PER_GUST;
  // g_1min_buffer_idx = (g_1min_buffer_idx + 1);  // Increment first
  // g_1min_gust_buffer_idx = g_1min_buffer_idx;   // Keep synced for simplicity

   int32_t i_wind_speed;
   int32_t i_wind_direction;
   int32_t i_wind_speed_inst;
   int32_t i_wind_direction_inst;


   i_wind_speed = (int32_t)(avg_wind.speed*1000);
   i_wind_direction = (int32_t)(avg_wind.direction*1000);

   i_wind_speed_inst = (int32_t)(wind_gust_inst.speed * 1000);
   i_wind_direction_inst = (int32_t)(wind_gust_inst.direction * 1000);

   //250ms 실시간 최대값 계산산,실시간은 분이 바뀌면 초기화
   if (i_wind_speed > i_wind_speed_inst)
   {
     i_wind_speed_inst = i_wind_speed;
   }

   if (i_wind_direction > i_wind_direction_inst)
   {
     i_wind_direction_inst = i_wind_direction;
   }

   wind_gust_inst.speed = (i_wind_speed_inst/1000.0);
   wind_gust_inst.direction = (i_wind_direction_inst / 1000.0);
}
void wind_process_1min(void)
{
  g_wind_gust_1min = find_max_gust(g_1min_gust_buffer, sizeof(g_1min_gust_buffer));
}



void wind_gust_init(void)
{
  wind_gust_inst.direction = 0;
  wind_gust_inst.speed  = 0;
}



 void temperature_process_1sec(float* temperature, uint8_t* err)
{

}
 uint16_t avg_sample(uint16_t* samples, uint16_t cnt)
 {
   uint32_t sum = 0;
   uint16_t avg;
   for (int i = 0; i < cnt; i++)
   {
     sum += samples[i];
   }

   avg = (sum / cnt);

   return avg;
 }

uint16_t g_temperature_avg;

void temperature_process_10s(uint16_t temperature)
{ 
  g_1min_temp_avg[g_1min_temp_avg_idx] = temperature;

  g_1min_temp_avg_idx = (g_1min_temp_avg_idx + 1) % TEMP_SAMPLES_PER_MINUTE;

  if (g_inst_buffer_idx >= SAMPLES_PER_GUST)
  {
    g_temperature_avg = avg_sample(g_1min_temp_avg, TEMP_SAMPLES_PER_MINUTE);
  }
}



void huminity_process_10s(uint16_t temperature)
{
  g_1min_temp_avg[g_1min_temp_avg_idx] = temperature;

  g_1min_temp_avg_idx = (g_1min_temp_avg_idx + 1) % TEMP_SAMPLES_PER_MINUTE;
}

 static measure_data_1s_t* g_p_raw;



 uint16_t get_aws_huminity(void)
 {
   uint16_t huminity=9;

   return huminity;
 }

uint16_t get_aws_barometer(void)
{
  uint16_t barometer;

  return barometer;

}

uint16_t get_aws_rain(void)
{
  sensor_data_t* p_sensor = g_p_raw->data;

  return p_sensor[A6_RAINFALL_DOT5_1MM].data.i;

}

uint16_t get_aws_raining(void)
{

  sensor_data_t* p_sensor = g_p_raw->data;
  bool raining;

  raining = p_sensor[A8_RAIN_PRESENT].data.b;

  if(raining)
  {
    return 10;
  }

  return 0;
}

uint16_t get_aws_snow(void)
{
  sensor_data_t* p_sensor = g_p_raw->data;
  uint16_t snow;

  snow = p_sensor[A9_SNOW_DEPTH].data.i;

  return snow;
}


//일사
uint16_t get_aws_solar_radiation(void)
{
  uint16_t rain;

  return rain;
}

uint16_t get_aws_sunshine(void)
{
  uint16_t rain;

  return rain;
}

uint16_t get_aws_soil_temperature_20cm(void)
{
  uint16_t rain;

  return rain;
}

uint16_t get_aws_soil_temperature_30cm(void)
{
  uint16_t rain;

  return rain;
}

uint16_t get_aws_wind_speed_gust(void)
{
  uint16_t wind_speed_gust;
  int32_t val;

  val = (int32_t)(wind_gust_inst.speed*10);

  wind_speed_gust = val;

  return wind_speed_gust;
}

uint16_t get_aws_wind_direction_gust(void)
{
  uint16_t wind_direction_gust;
  int32_t val;

  val = (int32_t)(wind_gust_inst.speed * 10);

  wind_direction_gust = val;

  return wind_direction_gust;
}




uint16_t get_aws_wind_speed(uint8_t* err)
{
  uint16_t wind_speed;
  int32_t val;

  sensor_data_t* p_sensor = g_p_raw->data;

  val = (uint16_t)(p_sensor[A3_WIND_SPEED].data.f * 10);

  if (p_sensor[A3_WIND_SPEED].err)
  {
    *err = 1;
    wind_speed = 9999;
  }
  else
  {
    *err = 0;
    wind_speed = val;
  }

  return wind_speed;
}


uint16_t get_aws_wind_direction(uint8_t* err)
{
  uint16_t wind_direction;
  int32_t val;

  sensor_data_t* p_sensor = g_p_raw->data;

  val = (uint16_t)(p_sensor[A2_WIND_DIRECTION].data.f*10);

  if(p_sensor[A2_WIND_DIRECTION].err)
  {
    *err = 1;
    wind_direction = 9999;
  }
  else
  {
    *err = 0;
    wind_direction = val;
  }

  return wind_direction;

}

uint16_t get_wind_direction(uint8_t* err)
{
  sensor_data_t* p_sensor = g_p_raw->data;
int32_t val;

  *err = p_sensor[A2_WIND_DIRECTION].err;
val =   (int32_t)(p_sensor[A2_WIND_DIRECTION].data.f*10);

  return val;
}

float get_wind_speed(uint8_t *err)
{
  sensor_data_t* p_sensor = g_p_raw->data;

  return p_sensor[A3_WIND_SPEED].data.f;

  *err = p_sensor[A3_WIND_SPEED].err;
}





#include <stdint.h>

#define ERROR_TIMEOUT_MS 10000

typedef struct
{
  uint8_t error_active;       // 현재 에러 상태인지 여부
  uint32_t error_start_time;  // 에러가 시작된 시간
} error_timer_t;


int is_error_timeout(error_timer_t* timer, uint8_t err_now, uint32_t now)
{
  if (err_now == 0)
  {
    // 정상 상태면 타이머 초기화
    timer->error_active = 0;
    timer->error_start_time = 0;
    return 0;  // 타임아웃 아님
  }

  if (!timer->error_active)
  {
    // 에러 상태 진입 시 타이머 시작
    timer->error_active = 1;
    timer->error_start_time = now;
    return 0;
  }

  // 이미 에러 상태 → 시간 확인
  if ((now - timer->error_start_time) >= ERROR_TIMEOUT_MS)
  {
    return 1;  // 타임아웃 발생
  }

  return 0;
}

#define ERROR_TIMEOUT_MS 10000
#define ERROR_VALUE 9999

#define ERROR_VALUE 9999

uint16_t get_aws_wind_direction_safe(void)
{
  static uint16_t last_valid_wind_direction = 0;
  static error_timer_t dir_timer = {0, 0};
  uint8_t err = 0;
  uint32_t now = OS_GET_TICK();

  uint16_t temp = get_aws_wind_direction(&err);

  if (is_error_timeout(&dir_timer, err, now))
  {
    return ERROR_VALUE;
  }

  if (err == 0)
  {
    last_valid_wind_direction = temp;
  }

  return last_valid_wind_direction;
}

uint16_t get_aws_wind_speed_safe(void)
{
  static uint16_t last_valid_wind_direction = 0;
  static error_timer_t dir_timer = {0, 0};
  uint8_t err = 0;
  uint32_t now = OS_GET_TICK();

  uint16_t temp = get_aws_wind_speed(&err);

  if (is_error_timeout(&dir_timer, err, now))
  {

    return ERROR_VALUE;
  }

  if (err == 0)
  {

    last_valid_wind_direction = temp;
  }

  return last_valid_wind_direction;
}

uint16_t get_aws_wind_speed_avg(void) 
{
  int32_t speed;
  
  speed = (int32_t)(avg_wind.speed*10);

  return speed;
}
uint16_t get_aws_wind_direction_avg(void)
{
  int32_t direction;

  direction = (int32_t)(avg_wind.direction * 10);

  return direction;
}

void update_kma_raw(void) 
{
  uint8_t err=0;

  g_kma_raw_ex.temperature.data = get_aws_temperature(&err);
  g_kma_raw_ex.temperature.err = err;
  
}

void aws_data_task(void* arg)
{
  DATE_TIME_BUF ct,ot;
  wind_t wind;
  uint8_t wind_spd_err;
  uint8_t wind_dir_err;
  uint8_t err=0;
  g_p_raw = aws_malloc(sizeof(measure_data_1s_t));

  ct = Date_Time;
  ot = ct;

  while (1)
  {
    ct = Date_Time;
    
    if(is_measurement_1s(g_p_raw,0) == false)
    {
      continue;
    }

    update_kma_raw();//원본 값을 저장한다.

    //자료 규격 처리 
    wind.direction = get_wind_direction(&wind_dir_err);
    wind.speed = get_wind_speed(&wind_spd_err);

    wind_process_250ms(&wind,wind_spd_err,wind_dir_err);//250ms마다 이동평균

    g_kma_inst.wind_speed_avg = get_aws_wind_speed_avg();
    g_kma_inst.wind_direction_avg = get_aws_wind_direction_avg();

    g_kma_inst.wind_speed_instant = get_aws_wind_speed_gust();
    g_kma_inst.wind_direction_instant = get_aws_wind_direction_gust();

    g_kma_inst.precipitation += get_aws_rain();
    g_kma_inst.precipitation_presence = get_aws_raining();//샘플링 시간 1분,더 빠르게 처리리
    g_kma_inst.snowfall = get_aws_snow();

    if (ct.Sec != ot.Sec)
    {
      g_kma_inst.solar_radiation = get_aws_solar_radiation();
      
      if (ct.Sec % 10 == 0)
      {
        g_kma_inst.temperature = get_aws_temperature(&err);
        g_kma_inst.relative_humidity = get_aws_temperature(&err);
        g_kma_inst.pressure    = get_aws_barometer();

        g_kma_inst.soil_temperature_20cm = get_aws_soil_temperature_20cm();
      }
      ot.Sec = ct.Sec;
    }

    if(ct.Min != ot.Min)
    {
      wind_gust_init();
      wind_process_1min();
      
      g_kma_1min = g_kma_inst;
      g_kma_1min.wind_direction_avg = 0;
      g_kma_1min.wind_speed_avg = 1;

      ot.Min = ct.Min;
    }

    if (ct.Day != ot.Day)
    {
      g_kma_inst.precipitation = 0;//금일 강우량
    }

    }//while
  }