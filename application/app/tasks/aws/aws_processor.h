

#ifndef AWS_PROCESSOR_H
#define AWS_PROCESSOR_H

#include <stdint.h>


#define WIND_SPEED_ERR 0x01
#define WIND_DIRECTION_ERR 0x02
typedef struct
{
  float u;
  float v;
} windVector_t;

typedef struct
{
  float speed;
  float direction;
} wind_t;


typedef struct
{
  int16_t dir_tenths;  // Wind direction in 0.1 degrees (0 - 3599)
  int16_t spd_tenths;  // Wind speed in 0.1 m/s
} WindReading_t;


windVector_t calculate_average_vector(const windVector_t* buffer, uint32_t count);
void calculate_windToVector(wind_t* wind,
                            windVector_t* windVector) ;
void calculate_vectorToWin(windVector_t* windVector, wind_t* wind);
void windInst_add_sample(wind_t* wind, uint8_t *err) ;
void wind1min_add_sample(wind_t* wind, uint8_t *err);

wind_t calculate_windInst(void);

void wind_process_250ms(wind_t* wind, uint8_t spd_err,uint8_t dir_err);

#endif