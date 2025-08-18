
#include <string.h>

#include "aws_data.h"
#include "app_sensor.h"


#include "task_measure.h"
#include "cmsis_os2.h"
#include "aws_default_data.h"

#pragma location = "SRAM_section"
kma_data_ex_t g_kma_raw_ex;
#pragma location = "SRAM_section"
kma_data_ex_t g_kma_inst_ex;
#pragma location = "SRAM_section"
kma_data_ex_t g_kma_1min_ex;
#pragma location = "SRAM_section"
kma_data_ex_t g_kma_10min_ex;
#pragma location = "SRAM_section"
kma_data_ex_t g_kma_1Hour_ex;
#pragma location = "SRAM_section"
kma_data_ex_t g_kma_day_ex;
rainfall_t g_rainfall;
sunshine_t g_sunshine;
sunshine_r_t g_solar_radiation;
snowfall_t g_snowfall;

aws_inst_t g_aws_inst;
aws_1min_t g_aws_1min_temp;
aws_day_t g_aws_day;
aws_10min_t g_aws_10min;


osMessageQueueId_t g_kma_data_queue[2];

kma_data_ex_t *get_kma_data(eAWS_DATA_MIN_t min)
{
  kma_data_ex_t *p_kma_data = NULL;

  switch (min)
  {
    case eAWS_DATA_REAL:
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
    case eAWS_DATA_DAY:
      p_kma_data = &g_kma_day_ex;
      break;
    case eAWS_DATA_RAW:
      p_kma_data = &g_kma_raw_ex;
      break;
      default:
      break;
  }

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

  memset(&g_kma_raw_ex, 0,sizeof(kma_data_ex_t));
  memset(&g_kma_inst_ex, 0, sizeof(kma_data_ex_t));
  memset(&g_kma_1min_ex, 0, sizeof(kma_data_ex_t));
  memset(&g_kma_10min_ex, 0, sizeof(kma_data_ex_t));
  memset(&g_kma_1Hour_ex, 0, sizeof(kma_data_ex_t));
  memset(&g_kma_day_ex, 0, sizeof(kma_data_ex_t));

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



