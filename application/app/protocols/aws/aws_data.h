
#ifndef AWS_DATA_H
#define AWS_DATA_H

#include <stdint.h>
#include <stdbool.h>
#include "util_time.h"


#define READ_TEMP(x) ((float)(x) / 10.0f - 100.0f)  // 기온, 지면온도, 지중온도, 초상온도
#define READ_RADI(x) ((float)(x) / 10.0f - 100.0f)  // 순복사, 전천복사, 반사복사 등
#define READ_X10(x) ((float)(x) / 10.0f)            // 풍속, 풍향, 습도, 토양수분,기압,강수량량 등
#define READ_X100(x) ((float)(x) / 100.0f)          // 일사량, 조도량 등
#define READ_DIRECT(x) ((uint16_t)(x))  // 운고, 시정, 현재일기, 타코미터 등 (정수값 그대로)

//[AWS = (관측값+100)/10, 관측값 = (x-1000)/10]
#define KMA_TO_TEMPERATURE(x) ((float)((x - 1000) / 10.0f))
#define KMA_TO_GENERAL(x) ((float)(x / 10.0f))
#define KMA_TO_1000(x) ((float)((x - 1000) / 10.0f))
#define KMA_TO_ILLUMINANCE(x) ((x) / 100.0f)
#define KMA_TO_RADI(x) ((x) / 10.0f - 100.0f)

// 새롭게 추가
typedef enum aws_data_min_s
{
  eAWS_DATA_AVG=0,//AWS(구) real과 동일
  eAWS_DATA_1MIN,
  eAWS_DATA_10MIN,
  eAWS_DATA_HOUR,
  eAWS_DATA_RAW
} eAWS_DATA_MIN_t;

typedef enum
{
  eKMA_DATA_Q_AVG = 0,
  eKMA_DATA_Q_1MIN
} eKMA_DATA_Q_t;

typedef struct aws_data_s
{
  bool enable;
  uint8_t err;
  uint16_t max;
  uint16_t min;
  uint16_t data;
  union 
  {
    int32_t i;
    float f;
    bool b;
  }raw;
}aws_data_t;

typedef struct aws_data_inst_s
{
  uint8_t err;
  union
  {
    int32_t i;
    float f;
    bool b;
  }data;
}aws_data_inst_t;

typedef struct aws_data_s2
{
  bool enable;
  uint8_t err;
  uint16_t hour;
  uint16_t month;
  uint16_t data;
  uint16_t year;
  uint32_t last_time;
  union 
  {
    int32_t i;
    float f;
  }raw;
} aws_rain_t;


typedef struct
{
  bool init;
  uint32_t crc;
  DATE_TIME_BUF time;
  aws_data_t temperature;             // 1. 기온 (1분 평균)
  aws_data_t wind_direction_avg;      // 2. 풍향 (1분 평균)
  aws_data_t wind_speed_avg;          // 3. 풍속 (1분 평균)
  aws_data_t wind_direction_instant;  // 4. 풍향 (1분 순간)
  aws_data_t wind_speed_instant;      // 5. 풍속 (1분 순간)
  aws_rain_t precipitation;          // 6. 강수량 (0.5/1.0 mm)
  aws_data_t pressure;                // 7. 기압 (1분 평균 현지 기압)
  aws_data_t precipitation_presence;  // 8. 강수 유무
  aws_data_t snowfall;                // 9. 적설
  aws_data_t relative_humidity;       // 10. 상대습도 (1분 평균)
  aws_data_t precipitation_fine;      // 11. 강수량 (0.1 mm)
  aws_data_t solar_radiation;        // 1. 일사 (누적값)  [누적 값(MJ/m2) × 100]
  aws_data_t sunshine_duration;      // 2. 일조 (누적 시간)
  aws_data_t surface_temperature;    // 3. 지면온도 (1분 평균)
  aws_data_t grass_temperature;      // 4. 초상온도 (1분 평균)
  aws_data_t soil_temperature_5cm;   // 5. 지중온도 (5cm, 1분 평균)
  aws_data_t soil_temperature_10cm;  // 6. 지중온도 (10cm, 1분 평균)
  aws_data_t soil_temperature_20cm;  // 7. 지중온도 (20cm, 1분 평균)
  aws_data_t soil_temperature_30cm;  // 8. 지중온도 (30cm, 1분 평균)
  aws_data_t soil_temperature_50cm;  // 9. 지중온도 (50cm, 1분 평균)
  aws_data_t soil_temperature_1m;    // 10. 지중온도 (1.0m, 1분 평균)
  aws_data_t soil_temperature_1_5m;  // 11. 지중온도 (1.5m, 1분 평균)
  aws_data_t soil_temperature_3m;    // 12. 지중온도 (3.0m, 1분 평균)
  aws_data_t soil_temperature_5m;    // 13. 지중온도 (5.0m, 1분 평균)

  aws_data_t cloud_height_1st;     // 1. 1층 운고 (1분 평균)
  aws_data_t cloud_height_2nd;     // 2. 2층 운고 (1분 평균)
  aws_data_t cloud_height_3rd;     // 3. 3층 운고 (1분 평균)
  aws_data_t cloud_amount;         // 4. 운량
  aws_data_t visibility;           // 5. 시정 (1분 평균)
  aws_data_t pm10_concentration;   // 6. PM10 (분진농도)
  aws_data_t pm25_concentration;   // 7. PM2.5 (분진농도)
  aws_data_t net_radiation;        // 8. 순복사 (1분 평균)
  aws_data_t total_radiation;      // 9. 전천복사 (1분 평균)
  aws_data_t reflected_radiation;  // 10. 반사복사 (1분 평균)
  aws_data_t direct_radiation;     // 11. 직달복사 (1분 평균)
  aws_data_t current_weather;      // 12. 현재 일기

  aws_data_t temp0_0;
  aws_data_t temp0_1;
  aws_data_t temp0_2;
  aws_data_t temp0_3;

  aws_data_t soil_moisture_10cm;  // 1. 토양수분 (10 cm)
  aws_data_t soil_moisture_20cm;  // 2. 토양수분 (20 cm)
  aws_data_t soil_moisture_30cm;  // 3. 토양수분 (30 cm)
  aws_data_t soil_moisture_50cm;  // 4. 토양수분 (50 cm)
  aws_data_t illuminance;         // 5. 조도량 (1분 평균)
  aws_data_t wind_speed_1_5m;          // 6. 풍속 (1.5 m, 1분 평균)
  aws_data_t wind_speed_4m;            // 7. 풍속 (4.0 m, 1분 평균)
  aws_data_t instant_wind_speed_1_5m;  // 8. 순간 풍속 (1.5 m)
  aws_data_t instant_wind_speed_4m;    // 9. 순간 풍속 (4.0 m)
  aws_data_t temperature_0_5m;  // 10. 기온 (0.5 m)
  aws_data_t temperature_4m;    // 11. 기온 (4.0 m)
  aws_data_t humidity_0_5m;     // 12. 습도 (0.5 m, 1분 평균)
  aws_data_t humidity_4m;       // 13. 습도 (4.0 m, 1분 평균)
  aws_data_t temp1_0;
  aws_data_t temp1_1;
  aws_data_t temp1_2;
  aws_data_t temp1_3;
  aws_data_t temp1_4;
  aws_data_t temp1_5;
  aws_data_t temp1_6;
  aws_data_t temp1_7;
  aws_data_t temp1_8;
  aws_data_t tacometer;
  uint8_t X_sensorStatus[8];
  uint8_t Y_volateStatus;
  bool updated;
} kma_data_ex_t;

typedef struct
{
  uint32_t crc;
  DATE_TIME_BUF time;
  int16_t temperature;            // 1. 기온 (1분 평균)
  int16_t wind_direction_avg;     // 2. 풍향 (1분 평균)
  int16_t wind_speed_avg;         // 3. 풍속 (1분 평균)
  int16_t wind_direction_instant; // 4. 풍향 (1분 순간)
  int16_t wind_speed_instant;     // 5. 풍속 (1분 순간)
  int16_t precipitation;       // 6. 강수량 (0.5/1.0 mm)
  int16_t pressure;               // 7. 기압 (1분 평균 현지 기압)
  int16_t precipitation_presence; // 8. 강수 유무
  int16_t snowfall;               // 9. 적설
  int16_t relative_humidity;      // 10. 상대습도 (1분 평균)
  int16_t precipitation_fine;     // 11. 강수량 (0.1 mm)
  int16_t solar_radiation;        // 1. 일사 (누적값)  [누적 값(MJ/m2) × 100]
  int16_t sunshine_duration;      // 2. 일조 (누적 시간)
  int16_t surface_temperature;    // 3. 지면온도 (1분 평균)
  int16_t grass_temperature;      // 4. 초상온도 (1분 평균)
  int16_t soil_temperature_5cm;   // 5. 지중온도 (5cm, 1분 평균)
  int16_t soil_temperature_10cm;  // 6. 지중온도 (10cm, 1분 평균)
  int16_t soil_temperature_20cm;  // 7. 지중온도 (20cm, 1분 평균)
  int16_t soil_temperature_30cm;  // 8. 지중온도 (30cm, 1분 평균)
  int16_t soil_temperature_50cm;  // 9. 지중온도 (50cm, 1분 평균)
  int16_t soil_temperature_1m;    // 10. 지중온도 (1.0m, 1분 평균)
  int16_t soil_temperature_1_5m;  // 11. 지중온도 (1.5m, 1분 평균)
  int16_t soil_temperature_3m;    // 12. 지중온도 (3.0m, 1분 평균)
  int16_t soil_temperature_5m;    // 13. 지중온도 (5.0m, 1분 평균)

  int16_t cloud_height_1st;    // 1. 1층 운고 (1분 평균)
  int16_t cloud_height_2nd;    // 2. 2층 운고 (1분 평균)
  int16_t cloud_height_3rd;    // 3. 3층 운고 (1분 평균)
  int16_t cloud_amount;        // 4. 운량
  int16_t visibility;          // 5. 시정 (1분 평균)
  int16_t pm10_concentration;  // 6. PM10 (분진농도)
  int16_t pm25_concentration;  // 7. PM2.5 (분진농도)
  int16_t net_radiation;       // 8. 순복사 (1분 평균)
  int16_t total_radiation;     // 9. 전천복사 (1분 평균)
  int16_t reflected_radiation; // 10. 반사복사 (1분 평균)
  int16_t direct_radiation;    // 11. 직달복사 (1분 평균)
  int16_t current_weather;     // 12. 현재 일기

  int16_t temp0_0;
  int16_t temp0_1;
  int16_t temp0_2;
  int16_t temp0_3;

  int16_t soil_moisture_10cm;      // 1. 토양수분 (10 cm)
  int16_t soil_moisture_20cm;      // 2. 토양수분 (20 cm)
  int16_t soil_moisture_30cm;      // 3. 토양수분 (30 cm)
  int16_t soil_moisture_50cm;      // 4. 토양수분 (50 cm)
  int16_t illuminance;             // 5. 조도량 (1분 평균)
  int16_t wind_speed_1_5m;         // 6. 풍속 (1.5 m, 1분 평균)
  int16_t wind_speed_4m;           // 7. 풍속 (4.0 m, 1분 평균)
  int16_t instant_wind_speed_1_5m; // 8. 순간 풍속 (1.5 m)
  int16_t instant_wind_speed_4m;   // 9. 순간 풍속 (4.0 m)
  int16_t temperature_0_5m;        // 10. 기온 (0.5 m)
  int16_t temperature_4m;          // 11. 기온 (4.0 m)
  int16_t humidity_0_5m;           // 12. 습도 (0.5 m, 1분 평균)
  int16_t humidity_4m;             // 13. 습도 (4.0 m, 1분 평균)
  int16_t temp1_0;
  int16_t temp1_1;
  int16_t temp1_2;
  int16_t temp1_3;
  int16_t temp1_4;
  int16_t temp1_5;
  int16_t temp1_6;
  int16_t temp1_7;
  int16_t temp1_8;
  int16_t tacometer;
  uint8_t X_sensorStatus[8];
  uint8_t Y_volateStatus;
} aws_logging_data_t;

typedef struct
{
  uint16_t temperature;          
  uint16_t wind_direction;      
  uint16_t wind_speed;            
  uint16_t pressure;            
  uint16_t precipitation_presence; 
  uint16_t snowfall;             
  uint16_t relative_humidity;   
  uint16_t precipitation_fine;    
  uint16_t solar_radiation;      
  uint16_t sunshine_duration;    
  uint16_t surface_temperature; 
  uint16_t grass_temperature;   
  uint16_t soil_temperature_5cm; 
  uint16_t soil_temperature_10cm; 
  uint16_t soil_temperature_20cm; 
  uint16_t soil_temperature_30cm; 
  uint16_t soil_temperature_50cm; 
  uint16_t soil_temperature_1m;   
  uint16_t soil_temperature_1_5m; 
  uint16_t soil_temperature_3m;   
  uint16_t soil_temperature_5m;   
} aws_inst_t;

typedef struct
{
  uint16_t temperature;
  uint16_t wind_direction_avg;
  uint16_t wind_speed_avg;
  uint16_t wind_direction_gust;
  uint16_t wind_speed_gust;
  uint16_t pressure;
  uint16_t precipitation_presence;
  uint16_t snowfall;
  uint16_t relative_humidity;
  uint16_t precipitation_fine;
  uint16_t solar_radiation;
  uint16_t sunshine_duration;
  uint16_t surface_temperature;
  uint16_t grass_temperature;
  uint16_t soil_temperature_5cm;
  uint16_t soil_temperature_10cm;
  uint16_t soil_temperature_20cm;
  uint16_t soil_temperature_30cm;
  uint16_t soil_temperature_50cm;
  uint16_t soil_temperature_1m;
  uint16_t soil_temperature_1_5m;
  uint16_t soil_temperature_3m;
  uint16_t soil_temperature_5m;
} aws_1min_t;

typedef struct
{
  uint16_t wind_direction_gust;
  uint16_t wind_speed_gust;
} aws_10min_t;

typedef struct
{
  uint16_t wind_direction_gust;
  uint16_t wind_speed_gust;
} aws_day_t;

typedef struct rainfall_s
{
  float min;
  float ten_min;
  float hourly;
  float today;
  float yesterday; 
  float monthly;  
  float yearly;   
}rainfall_t;

typedef struct sunshine_s
{
  uint32_t yesterday;
  uint32_t min;
  uint32_t today;
  uint32_t hourly;
  uint32_t monthly;
  uint32_t yearly;
} sunshine_t;//일조 

typedef struct sunshine_r_s
{
  uint32_t sunshine_r_1min;//w/m2  1분 누적값
  uint32_t sunshine_r_1min_acc;//1분동안 실시간 누적되는 값
} sunshine_r_t;















kma_data_ex_t *get_kma_data(eAWS_DATA_MIN_t min);

void kma_data_q_init(void);
int32_t read_kma_data(eKMA_DATA_Q_t kma_data_num, kma_data_ex_t *p_kma_data);
void send_kma_data(eKMA_DATA_Q_t kma_data_num, kma_data_ex_t *p_kma_data);



extern sunshine_t g_sunshine;;
extern rainfall_t g_rainfall;
extern sunshine_r_t g_sunshine_r;
extern aws_inst_t g_aws_inst;;

#endif