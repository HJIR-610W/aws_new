#ifndef KMA2_PROTOCOL_H
#define KMA2_PROTOCOL_H

#include <stdint.h>

#include "kma_define.h"
// AWS(구)에서 szProtocolVersion[3]에 설정된값 
#define KMA2_PROTOCOL_YEAR 9
#define KMA2_PROTOCOL_MONTH 5
#define KMA2_PROTOCOL_DAY 1


typedef struct
{
  int16_t temperature;  
  int16_t wind_direction_avg;  
  int16_t wind_speed_avg;
  int16_t wind_direction_instant;
  int16_t wind_speed_instant;
  int16_t precipitation;
  int16_t pressure; 
  int16_t precipitation_presence;  
  int16_t snowfall;  
  int16_t relative_humidity;  
  int16_t precipitation_fine;  
  int16_t L1;
  int16_t L2;
  int16_t L3;
  int16_t L4;
  int16_t L5;
  int16_t L6;
  int16_t L7;
  int16_t L8;
  int16_t L9;
  int16_t L10;
  int16_t solar_radiation;  //a일사사
  int16_t sunshine_duration;  //b일조조
  int16_t surface_temperature;  //c
  int16_t grass_temperature;   //d
  int16_t soil_temperature_5cm; //e
  int16_t soil_temperature_10cm;//f
  int16_t soil_temperature_20cm; //g
  int16_t soil_temperature_30cm; //h
  int16_t soil_temperature_50cm; //i
  int16_t soil_temperature_1m; //j
  int16_t soil_temperature_1_5m; //k
  int16_t soil_temperature_3m;  //l
  int16_t soil_temperature_5m;//m
  int16_t S1;
  int16_t S2;
  int16_t S3;
  int16_t S4;
  int16_t S5;
  int16_t S6;
  int16_t S7;
  int16_t S8;
  int16_t S9;
  int16_t S10;
  uint8_t X_voltage_status;
  uint16_t Y_logger_status;
  uint8_t Z_logger_status;
} kma2_response_t;

uint16_t make_kma2_essential(uint8_t *buffer, kma2_response_t *p_aws) ;
uint16_t make_kma2_essential_selective(uint8_t *buffer, kma2_response_t *p_aws);
uint16_t make_kma2_precipitation(uint8_t *buffer, kma2_response_t *p_aws);

uint16_t make_kma2_response(uint8_t *rx_frame, uint8_t *data, uint16_t data_len,
                          uint8_t data_format_no,uint8_t *tx_frame);

#endif