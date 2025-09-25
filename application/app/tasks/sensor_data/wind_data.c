
#include "wind_data.h"

#include <stdio.h>
#include <math.h>
#include <stdint.h>
#include "util_memory.h"
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

  if (*speed == 0)
  {
    *direction_deg = 0;
    return;
  }
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


static wind_t s_wind_mavg_sample[WIND_SPEED_AVG_CNT] ;

wind_t wind_max[eWIND_MAX];






/*
[별표 4]<개정 2023.3.6.>
 신호 및 자료처리의 표준규격(제 9조 관련)
풍향,풍속

순간 풍향,순간 풍속(gust)
- 250ms 마다 이동평균 3초
- 1분 동안 이동평균하여 산출된 지난 240개의 자료 중 최댓값을 1분 최대 순간풍향ㆍ풍속으로 산출한다
   (원태희 해석)순간 풍향 순간 풍속은 벡터 계산 하지 않는다.측정된 풍속이 최대일때 그때의 풍향을 최대
  풍향으로 사용한다.
- 매 1분마다 지난 10개의 1분값 중에서 최댓값을 10분 최대순간풍향ㆍ풍속으로 산출한다.
- 하루 동안 수집된 1분 최대순간풍향ㆍ풍속 1440개 중에서 최댓값을 일 최대순간풍향ㆍ풍속으로 산출한다.

 1분 평균 풍향ㆍ풍속
- 0.25초 간격의 바람벡터 자료를 10초 동안 평균을 구한 후 1분 동안 6개의 자료를 다시 평균하여 매분자료를 산출한다.
  (원태희 해석)0.25초마다 이동평균한 값을 바람벡터로 환산후 10초동안 평균을 구하여(40개 샘플)
  그렇게 1분동안 총 6개를 다시 평균하여 1분 평균 풍향,풍속을 산출한다.
  
  *쉽게 하는법
   재귀적 평균사용하여 10초가 되면 평균값을 저장, 그 값을 다시 1분 재귀적 평균함수에 전달 1분이 되면 평균값 사용




*/

static uint8_t s_mavg_sample_count = 0;

void add_wind_sample(float speed,float direction)
{
  uint16_t index = s_mavg_sample_count%WIND_SPEED_AVG_CNT;
  s_wind_mavg_sample[index].speed = (double)speed;
  s_wind_mavg_sample[index].direction = direction;

  s_mavg_sample_count++;
  if (s_mavg_sample_count >= (WIND_SPEED_AVG_CNT<<1))
  {
    s_mavg_sample_count = WIND_SPEED_AVG_CNT;
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
  uint8_t actual_count;

  if (s_mavg_sample_count < WIND_SPEED_AVG_CNT)
    actual_count = s_mavg_sample_count;
  else
    actual_count = WIND_SPEED_AVG_CNT;

  for (uint8_t i = 0; i < actual_count; i++)
  {
    speed_sum += s_wind_mavg_sample[i].speed;
    direction_sum += s_wind_mavg_sample[i].direction;
  }

  *wind_speed = speed_sum / actual_count;
  *wind_direction = direction_sum / actual_count;
}


//speed,direction x10한값 
void calculate_wind_max(eWIND_MAX_t wind,int32_t speed,int32_t direction)
{
  if (speed > (int32_t)wind_max[wind].speed)
  {
    wind_max[wind].speed = speed;
    wind_max[wind].direction = direction;
  }
}
// speed,direction x10한값 direction 123 ->12.3도
void read_wind_max(eWIND_MAX_t wind, int32_t *speed, int32_t *direction)
{
  *speed     = (int32_t)wind_max[wind].speed ;
  *direction = (int32_t)wind_max[wind].direction;
}

int32_t read_wind_speed_max(eWIND_MAX_t wind)
{
  int32_t max;

  max  = (int32_t)wind_max[wind].speed;
  wind_max[wind].speed = 0;

  return max;
}

int32_t read_wind_direction_max(eWIND_MAX_t wind)
{
  int32_t max;

  max =  (int32_t)wind_max[wind].direction;
  wind_max[wind].direction = 0;
  return max;
}

    void wind_max_init(eWIND_MAX_t wind)
{
  wind_max[wind].speed = 0;
  wind_max[wind].direction = 0;;
}

// 1분 바람벡터 평균 값 용
double wind_avg_1min_u = 0;
double wind_avg_1min_v=0;
uint8_t wind_avg_1min_count=0;

void update_wind_vector_avg_1min(float speed,float direction)
{
  double u,v;

  calculate_uv(direction,speed,&u,&v);

  wind_avg_1min_count++;
  wind_avg_1min_u = recursive_avg(wind_avg_1min_u, u, wind_avg_1min_count);
  wind_avg_1min_v = recursive_avg(wind_avg_1min_v, v, wind_avg_1min_count);
}

void calculate_wind_avg_1min(float *p_speed,float *p_direction)
{
  double speed;
  double direction;
  calculate_wind(wind_avg_1min_u, wind_avg_1min_v,&speed,&direction);
  *p_speed = speed;
  *p_direction = direction;
}

void wind_avg_1min_init(void)
{
  wind_avg_1min_u = 0;
  wind_avg_1min_v = 0;
  wind_avg_1min_count = 0;
}