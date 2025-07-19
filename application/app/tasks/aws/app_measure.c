
#include <stdint.h>

#include "Sensors\temperature\temperature.h"
#include "util_memory.h"


#define SAMPLE_CNT_TEMP 6

float g_temp_avg_buff[SAMPLE_CNT_TEMP];
uint8_t g_temp_avg_idx=0;

float g_humi_avg_buff[SAMPLE_CNT_TEMP];
uint8_t g_humi_avg_idx=0;

float g_barometer_avg_buff[SAMPLE_CNT_TEMP];
uint8_t g_barometer_avg_idx=0;


void sample_temperature(float temp)
{
  uint8_t index;

  index= g_temp_avg_idx%SAMPLE_CNT_TEMP;

  g_temp_avg_buff[index] = temp;

  g_temp_avg_idx++;

  if(g_temp_avg_idx >= (SAMPLE_CNT_TEMP*2))
  {
    g_temp_avg_idx =SAMPLE_CNT_TEMP;
  }
}

/**
 * @brief 현재시간 기준 과거데이터 6개의 평균
 * 만약 아직 6개가 수집이 안되었다면 수집된 샘플 수만 평균균
 */
/**
 * @brief 현재시간 기준 과거데이터 6개의 평균
 * 만약 아직 6개가 수집이 안되었다면 수집된 샘플 수만 평균균
 */
float get_avg_temperature(void)
{
  float avg = 0;
  uint8_t cnt = 0;
  uint8_t err_cnt = 0;

  if (g_temp_avg_idx < SAMPLE_CNT_TEMP)
  {
    for (int i = 0; i < g_temp_avg_idx; i++)
    {
      if (g_temp_avg_buff[i] != TEMP_ERR_VAL)
      {
        cnt++;
        avg = recursiveAvg(avg, g_temp_avg_buff[i], cnt);
      }
      else
      {
        err_cnt++;
      }
    }


    if (err_cnt == g_temp_avg_idx)
    {
      avg = TEMP_ERR_VAL;
    }
  }
  else
  {
    for (int i = 0; i < SAMPLE_CNT_TEMP; i++)
    {
      if (g_temp_avg_buff[i] != TEMP_ERR_VAL)
      {
        cnt++;
        avg = recursiveAvg(avg, g_temp_avg_buff[i], cnt);
      }
      else
      {
        err_cnt++;
      }
    }

    if (err_cnt == SAMPLE_CNT_TEMP)
    {
      avg = TEMP_ERR_VAL;
    }
  }


  return avg;
}


void sample_humi(float temp)
{
  uint8_t index;

  index= g_humi_avg_idx%SAMPLE_CNT_TEMP;

  g_humi_avg_buff[index] = temp;

  g_humi_avg_idx++;

  if(g_humi_avg_idx >= (SAMPLE_CNT_TEMP*2))
  {
    g_humi_avg_idx = SAMPLE_CNT_TEMP;
  }
}

/**
 * @brief 현재시간 기준 과거데이터 6개의 평균
 * 만약 아직 6개가 수집이 안되었다면 수집된 샘플 수만 평균균
 */
/**
 * @brief 현재시간 기준 과거데이터 6개의 평균
 * 만약 아직 6개가 수집이 안되었다면 수집된 샘플 수만 평균균
 */
float get_avg_humi(void)
{
  float avg = 0;

  if (g_humi_avg_idx < SAMPLE_CNT_TEMP)
  {
    for (int i = 0; i < g_humi_avg_idx; i++)
    {
      avg = recursiveAvg(avg, g_humi_avg_buff[i], i + 1);
    }
  }
  else
  {
    for (int i = 0; i < SAMPLE_CNT_TEMP; i++)
    {
      avg = recursiveAvg(avg, g_humi_avg_buff[i], i + 1);
    }
  }

  return avg;
}



void sample_barometer(float temp)
{
  uint8_t index;

  index= g_barometer_avg_idx%SAMPLE_CNT_TEMP;

  g_barometer_avg_buff[index] = temp;

  g_barometer_avg_idx++;

  if(g_barometer_avg_idx >= (SAMPLE_CNT_TEMP*2))
  {
    g_barometer_avg_idx = SAMPLE_CNT_TEMP;
  }
}

/**
 * @brief 현재시간 기준 과거데이터 6개의 평균
 * 만약 아직 6개가 수집이 안되었다면 수집된 샘플 수만 평균균
 */
/**
 * @brief 현재시간 기준 과거데이터 6개의 평균
 * 만약 아직 6개가 수집이 안되었다면 수집된 샘플 수만 평균균
 */
float get_avg_barometer(void)
{
  float avg = 0;

  if (g_barometer_avg_idx < SAMPLE_CNT_TEMP)
  {
    for (int i = 0; i < g_barometer_avg_idx; i++)
    {
      avg = recursiveAvg(avg, g_barometer_avg_buff[i], i + 1);
    }
  }
  else
  {
    for (int i = 0; i < SAMPLE_CNT_TEMP; i++)
    {
      avg = recursiveAvg(avg, g_barometer_avg_buff[i], i + 1);
    }
  }

  return avg;
}