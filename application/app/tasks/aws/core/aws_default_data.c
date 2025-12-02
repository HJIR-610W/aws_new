

#include "aws_default_data.h"

#include "app_sensor.h"

#include "util_memory.h"
#include "util_filter.h"


#if 0
#define AVG_CNT 6
typedef struct 
{
  uint8_t index;
  int32_t sample[AVG_CNT];
}avg_1min_t;

avg_1min_t avg_1min[eAVG_MAX];


void add_sample_1min(eAVG_DATA_TYPE_t number, int32_t data)
{
  int index = avg_1min[number].index % AVG_CNT;
  avg_1min[number].sample[index] = data;
  index++;
  if (index >= (AVG_CNT <<1))
  {
    index = AVG_CNT;
  }
  avg_1min[number].index = index;

}

void calculate_1min_avg(eAVG_DATA_TYPE_t number,int32_t *p_avg)
{
  int32_t sum = 0;
  int32_t count;

  if (avg_1min[number].index < AVG_CNT)
    count = avg_1min[number].index;
  else
    count = AVG_CNT;
    
    for (uint8_t i = 0; i < count; i++)
    {
      sum += avg_1min[number].sample[i];
    }

  *p_avg = sum / count;



}

//이것을 호출하면 이동평균으로 사용하지 않는다.
void reset_avg(eAVG_DATA_TYPE_t number)
{
  avg_1min[number].index = 0;
}
#endif
/*
10초마다 add_sample_1min호출하여 샘플 저장
이동평균 함수로 사용하자고 할때 예)

장비가 15초에 켜졌다고 가정하면 20초부터 샘플이 저장된다.
총 60초간 즉 6개의 샘플을 사용하여 이동평균하는데 
버퍼는 6개고 초기값은 전부 0이다.
이동평균 할때 첫번째 값이 1이다
그때의 이동평균값은
sample[0] = 1;
sample[1] = 0;
sample[2] = 0;
sample[3] = 0;
sample[4] = 0;
sample[5] = 0;

매 10초마다 이동평균값을 산출해야하는데 
20초에 이동평균값은 1/6 하면 안된다. 아직 버퍼가 완전히 채워진 상태가 아니기 때문에 입력된 샘플 갯수만 평균내어야 한다.
평균 = 1(값)/1(갯수)이다. 

초 index 값  평균을 위한 갯수
20  0     1  1 
30  1     2  2
40  2     3  3
50  3     4  4
0   4     5  5   
10  5     6  6
20  0     7  6
30  1     8  6

버퍼간 완전히 채워진 상태 부터는 평균을 위한 갯수가 버퍼 갯수가 되는것이다.

이렇게 하지 않는다면 
처음 샘플값이 정상 값인데도 불구하고 잘못된 이동평균방식으로 값이 낮게 측정되는 문제가 발생한다.

*/


data_avg_t g_avg_1min[eAVG_MAX];
data_avg_t g_avg_10min[eAVG_MAX];
data_avg_t g_avg_hour[eAVG_MAX];

data_min_max_t g_1min_min_max[eAVG_MAX];
data_min_max_t g_10min_min_max[eAVG_MAX];
data_min_max_t g_hour_min_max[eAVG_MAX];
data_min_max_t g_day_min_max[eAVG_MAX];

int32_t calculate_data_avg(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer, int16_t sample)
{
  uint16_t count = p_avg_buffer[type].count;
  float average = p_avg_buffer[type].average;

  if(sample==-9999)//에러값은 평균에 포함하지 않는다.
  {
    return (int32_t)average;
  }
  count++;
  average = recursive_avg_i(average, sample, count);
  p_avg_buffer[type].average = average;
  p_avg_buffer[type].count = count;

  return (int32_t)average;
}

int32_t read_current_data_average(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer)
{
  return (int32_t)p_avg_buffer[type].average;
}

void data_avg_init(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer)
{
  p_avg_buffer[type].average = 0;
  p_avg_buffer[type].count = 0;
}

int32_t read_data_average(eAVG_DATA_TYPE_t type, data_avg_t *p_avg_buffer)
{
  int32_t average;

  average = read_current_data_average(type,p_avg_buffer);
  data_avg_init(type,p_avg_buffer);

  return average;
}

void calculate_data_min_max(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max, int32_t sample)
{
  if (sample > p_min_max[type].max)
  {
    p_min_max[type].max = sample;
  }

  if (sample < p_min_max[type].min)
  {
    p_min_max[type].min = sample;
  }
}

int32_t read_current_data_min(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max)
{
  return p_min_max[type].min;

}

int32_t read_current_data_max(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max)
{

  return  p_min_max[type].max;
}



void data_min_max_init(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max, int32_t min, int32_t max)
{
  p_min_max[type].min = min;
  p_min_max[type].max = max;
}


int32_t read_data_min(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max_buffer,int32_t set_min)
{
  int32_t min;
  min = p_min_max_buffer[type].min;
  p_min_max_buffer[type].min = set_min;

  return min;
}

int32_t read_data_max(eAVG_DATA_TYPE_t type, data_min_max_t *p_min_max_buffer,int32_t set_max)
{
  int32_t max;

  max = p_min_max_buffer[type].max;
  p_min_max_buffer[type].max = set_max;

  return max;
}

void aws_min_max_init(void)
{

  for (int i = 0; i < eAVG_MAX; i++)
  {
    g_1min_min_max[i].max = 0;
    g_10min_min_max[i].max = 0;
    g_hour_min_max[i].max = 0;
    g_day_min_max[i].max = 0;

    g_1min_min_max[i].min = 10000;
    g_10min_min_max[i].min = 10000;
    g_hour_min_max[i].min = 10000;
    g_day_min_max[i].min = 10000;
  }
}