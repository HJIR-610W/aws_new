
#include "wind_data.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void calculate_uv(double theta_deg, double s, double *u, double *v)
{
  double theta = theta_deg * M_PI / 180.0; // 도 -> 라디안
  *u = -s * sin(theta);
  *v = -s * cos(theta);
}

void calculate_wind(double u, double v, double *speed, double *direction_deg)
{
  // 1. 풍속 계산 (이 부분은 올바릅니다)
  *speed = sqrt(u * u + v * v);

  // 2. 수학적 각도 계산 (atan2의 인자 순서는 y, x 입니다)
  // u가 x축(동서), v가 y축(남북)에 해당합니다.
  double angle_rad = atan2(v, u);
  double angle_deg = angle_rad * 180.0 / M_PI;

  // 3. 기상학적 풍향으로 변환
  //    수학 각도(동쪽=0°) -> 풍향(북쪽=0°) 변환 및 바람의 방향(불어가는 쪽) -> 풍향(불어오는 쪽) 변환
  //    공식: 270 - (벡터 방향 각도) + 180 => 450 - (벡터 방향 각도)
  double wind_dir = 270.0 - angle_deg;

  // 4. 결과를 0~360도 범위로 정규화
  //    fmod는 부동소수점의 나머지 연산을 수행합니다.
  *direction_deg = fmod(wind_dir, 360.0);
  if (*direction_deg < 0)
  {
    *direction_deg += 360.0;
  }
}



#define WIND_SPEED_AVG_CNT 12 //0.25초 간격 3초
#define WIND_AVG_10S_CNT 40
#define WIND_AVG_1MIN_CNT 6

typedef struct 
{
  float speed;
  float direction;
}wind_t;

typedef struct wind_vector_s
{
  float u;
  float v;
} wind_vector_t;

#pragma location = "SRAM_section"
static wind_t s_wind_mavg_sample[WIND_SPEED_AVG_CNT] ;
#pragma location = "SRAM_section"
static wind_vector_t s_wind_10s[WIND_AVG_10S_CNT];
#pragma location = "SRAM_section"
static wind_vector_t s_wind_1min[WIND_AVG_1MIN_CNT];




wind_t wind_max[eWIND_MAX];

static uint8_t s_wind_sample_count = 0;
static uint8_t s_wind_1min_count = 0;
static uint8_t s_wind_10s_count = 0;
static uint8_t s_wind_10s_index = 0;
static uint8_t s_wind_1min_index = 0;


// 1분 평균 풍향,풍속 0.25초 간격의 바람벡터를 10초동안 평균구한 후
// 1분동안 총 6개의 자료를 다시 평균하여 분자료 산출
void calculate_wind_1min(double *wind_speed, double *wind_direction)
{
  double u=0;
  double v=0;
  double avg_u;
  double avg_v;
  uint8_t actual_count = (s_wind_1min_count < WIND_AVG_1MIN_CNT) ? s_wind_1min_count : WIND_AVG_1MIN_CNT;

  uint8_t idx = (s_wind_1min_index + WIND_AVG_1MIN_CNT - 1) % WIND_AVG_1MIN_CNT;

  for (int i = 0; i < actual_count; i++)
  {
    u += s_wind_1min[idx].u;
    v += s_wind_1min[idx].v;
    idx = (idx + WIND_AVG_1MIN_CNT - 1) % WIND_AVG_1MIN_CNT;
  }

  avg_u = u / actual_count;
  avg_v = v / actual_count;

  calculate_wind(avg_u, avg_v, wind_speed, wind_direction);
}

/*
10초마다 평균낸 바람벡터를 샘플저장한다.
10초마다 이함수 호출
*/
void add_wind_vector_1min_samle(double u,double v)
{
  s_wind_1min[s_wind_1min_index].u = u;
  s_wind_1min[s_wind_1min_index].v = v;
  s_wind_1min_index = (s_wind_1min_index + 1) % WIND_AVG_1MIN_CNT;

  if (s_wind_1min_count < WIND_AVG_1MIN_CNT)
  {
    s_wind_1min_count++;
  }
}
/*
10초 마다 호출하여 업데이트
10초간 샘플링한 바람벡터 40개의 벡터평균을 구한다.
그런데 장비가 껐다 켜지면 샘플링 갯수가 40개 아닐수 있기에
실제 저장된 샘플링 갯수만큼 평균을 구해야한다.
10초마다 이샘플 호출
*/
void calculate_wind_vector_10s(double *p_u,double *p_v)
{
  double u=0;
  double v=0;


  uint8_t actual_count = (s_wind_10s_count < WIND_AVG_10S_CNT) ? s_wind_10s_count : WIND_AVG_10S_CNT;

  uint8_t idx = (s_wind_10s_index + WIND_AVG_10S_CNT - 1) % WIND_AVG_10S_CNT;
  for (int i = 0; i < actual_count; i++)
  {
    u += s_wind_10s[idx].u;
    v += s_wind_10s[idx].v;

    idx = (idx + WIND_AVG_10S_CNT - 1) % WIND_AVG_10S_CNT;
  }

  *p_u = u / actual_count;
  *p_v = v / actual_count;
}


//0.25마다 바람 벡터를 저장한다. 
void add_wind_vector_250ms(float wind_speed,float wind_direction)
{
  double u;
  double v;
  
  calculate_uv(wind_direction,wind_speed,&u,&v);

  s_wind_10s[s_wind_10s_index].u = u;
  s_wind_10s[s_wind_10s_index].v = v;

  s_wind_10s_index = (s_wind_10s_index + 1) % WIND_AVG_10S_CNT;

  if (s_wind_10s_count < WIND_AVG_10S_CNT)
  {
    s_wind_10s_count++;
  }
}

static uint8_t s_sample_index = 0;

void add_wind_sample(float speed,float direction)
{
  s_wind_mavg_sample[s_sample_index].speed = (double)speed;
  s_wind_mavg_sample[s_sample_index].direction = direction;
  s_sample_index = (s_sample_index + 1) % WIND_SPEED_AVG_CNT;

  if (s_wind_sample_count < WIND_SPEED_AVG_CNT)
  {
    s_wind_sample_count++;
  }
}

/*
순서 샘플링 시간(s)
0    0.25
1    0.5     
2    0.75
3    1.0  
4    1.25
5    1.5
6    1.75 
7    2.0
8    2.25
9    2.5
10   2.7
11   3.0

0.25s마다 샘플링 하고 1초 간격으로 이동평균 해야 한다.
즉 총 12개를 합산하여 평균하는데 1초 간격으로 해야한다.  
샘플링 갯수가 4의 배수마다 샘플링하면 된다. 
이동평균을 하는데 아직 완전히 버퍼가 채워지지 않은 상태라면 채워진 만큼만 평균한다.

*/

//결과값이 순간 풍향 ,푼간 풍속
void calculate_wind_moving_avg(float *wind_speed,float *wind_direction)
{
  double speed_sum=0;
  double direction_sum=0;
  uint8_t actual_count = (s_wind_sample_count < WIND_SPEED_AVG_CNT) ? s_wind_sample_count : WIND_SPEED_AVG_CNT;


  uint8_t idx = (s_sample_index + WIND_SPEED_AVG_CNT - 1) % WIND_SPEED_AVG_CNT;
  for (uint8_t i = 0; i < actual_count; i++)
  {
    speed_sum += s_wind_mavg_sample[idx].speed;
    direction_sum += s_wind_mavg_sample[idx].speed;
    idx = (idx + WIND_SPEED_AVG_CNT - 1) % WIND_SPEED_AVG_CNT;
  }

  *wind_speed = speed_sum / actual_count;
  *wind_direction = direction_sum / actual_count;
}



void calculate_wind_max(eWIND_MAX_t wind,float speed,float direction)
{
  if (speed > wind_max[wind].speed)
  {
    wind_max[wind].speed = speed;
    wind_max[wind].direction = direction;
  }
}



void read_wind_max(eWIND_MAX_t wind,float *speed,float *direction)
{
  *speed     = wind_max[wind].speed ;
  *direction = wind_max[wind].direction;
}