

#include <stdint.h>

#include "app_file.h"
#include "user_heap.h"
#include "utile_time.h"


typedef struct monthdata_s
{
  uint16_t data[31];
}monthdata_t;

monthdata_t month[12];

void calculate_aws_data(void)
{
  uint16_t month_rain;
  uint8_t *p_data;
  FRESULT rets;
  char buff[100];


}