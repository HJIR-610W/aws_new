#include "aws_kma.h"
#include "utile.h"
#include "app_sensor.h"
const kma_cmd_t kma_cmd[]={{eAI,"AI?"},
                           {eAB,"AB?"},
                           {eAQ,"AQ?"},
                           {eAV,"AV?"},
                           {eAR,"AR?"},
                           {eAO,"AO?"},
                           {eAD,"AD?"},
                           {eAT,"AT?"},
                           {eAW,"AW?"},
                           {eAC,"AC?"},
                           {eAP,"AP?"}};


uint16_t coutntof_kma_cmd(void)
{
  return _countof(kma_cmd);
}



