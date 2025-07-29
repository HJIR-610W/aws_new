
#ifndef AWS_DATA_H
#define AWS_DATA_H

#include <stdint.h>
#include <stdbool.h>

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
  eAWS_DATA_AVG,//AWS(구) real과 동일
  eAWS_DATA_1MIN,
  eAWS_DATA_10MIN,
  eAWS_DATA_HOUR,
  eAWS_DATA_RAW
} eAWS_DATA_MIN_t;
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
  uint16_t crc;
  // 1. 기온 (1분 평균)
  int16_t temperature; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 (관측값 * 10)

  // 2. 풍향 (1분 평균)
  int16_t wind_direction_avg; // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 3599 (관측값 * 10)

  // 3. 풍속 (1분 평균)
  int16_t wind_speed_avg; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 4. 풍향 (1분 순간)
  int16_t wind_direction_instant; // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 3599 (관측값 * 10)

  // 5. 풍속 (1분 순간)
  int16_t wind_speed_instant; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 6. 강수량 (0.5/1.0 mm)
  int16_t precipitation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

  // 7. 기압 (1분 평균 현지 기압)
  int16_t pressure; // 사용비트: 13, 유효범위: 0 ~ 16383 (인치 코드), 표현범위: 5000 ~ 11000

  // 8. 강수 유무
  int16_t precipitation_presence; // 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 = 강수 없음, 1 = 강수 있음

  // 9. 적설
  int16_t snowfall; // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 4095 (관측값 * 10)

  // 10. 상대습도 (1분 평균)
  int16_t relative_humidity; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 11. 강수량 (0.1 mm)
  int16_t precipitation_fine; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

  // 1. 일사 (누적값)
  int16_t solar_radiation; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 [관측값(MJ/m²) * 100]

  // 2. 일조 (누적 시간)
  int16_t sunshine_duration; // 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 65535 [누적시간(초 단위)]

  // 3. 지면온도 (1분 평균)
  int16_t surface_temperature; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 4. 초상온도 (1분 평균)
  int16_t grass_temperature; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 5. 지중온도 (5cm, 1분 평균)
  int16_t soil_temperature_5cm; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 6. 지중온도 (10cm, 1분 평균)
  int16_t soil_temperature_10cm; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 7. 지중온도 (20cm, 1분 평균)
  int16_t soil_temperature_20cm; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 8. 지중온도 (30cm, 1분 평균)
  int16_t soil_temperature_30cm; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 9. 지중온도 (50cm, 1분 평균)
  int16_t soil_temperature_50cm; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 10. 지중온도 (1.0m, 1분 평균)
  int16_t soil_temperature_1m; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 11. 지중온도 (1.5m, 1분 평균)
  int16_t soil_temperature_1_5m; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 12. 지중온도 (3.0m, 1분 평균)
  int16_t soil_temperature_3m; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]

  // 13. 지중온도 (5.0m, 1분 평균)
  int16_t soil_temperature_5m; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000 [(관측값 + 100) * 10]


  // 1. 1층 운고 (1분 평균)
  int16_t cloud_height_1st; // 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

  // 2. 2층 운고 (1분 평균)
  int16_t cloud_height_2nd; // 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

  // 3. 3층 운고 (1분 평균)
  int16_t cloud_height_3rd; // 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000 (관측값[m])

  // 4. 운량
  int16_t cloud_amount; // 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 ~ 10 (관측값)

  // 5. 시정 (1분 평균)
  int16_t visibility; // 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 50000 (관측값[m])

  // 6. PM10 (분진농도)
  int16_t pm10_concentration; // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)

  // 7. PM2.5 (분진농도)
  int16_t pm25_concentration; // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599 (관측값 [μg/m³] × 10)

  // 8. 순복사 (1분 평균)
  int16_t net_radiation; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 9. 전천복사 (1분 평균)
  int16_t total_radiation; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 10. 반사복사 (1분 평균)
  int16_t reflected_radiation; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 11. 직달복사 (1분 평균)
  int16_t direct_radiation; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값[W/m²] + 1000) × 10

  // 12. 현재 일기
  int16_t current_weather; // 사용비트: 6, 유효범위: 0 ~ 127 (인치 코드), 표현범위: 0 ~ 99 (관측값)

  int16_t temp0[4];

  // 1. 토양수분 (10 cm)
  int16_t soil_moisture_10cm; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 2. 토양수분 (20 cm)
  int16_t soil_moisture_20cm; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 3. 토양수분 (30 cm)
  int16_t soil_moisture_30cm; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 4. 토양수분 (50 cm)
  int16_t soil_moisture_50cm; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 5. 조도량 (1분 평균)
  int16_t illuminance; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767 (관측값 * 100)

  // 6. 풍속 (1.5 m, 1분 평균)
  int16_t wind_speed_1_5m; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 7. 풍속 (4.0 m, 1분 평균)
  int16_t wind_speed_4m; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 8. 순간 풍속 (1.5 m)
  int16_t instant_wind_speed_1_5m; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 9. 순간 풍속 (4.0 m)
  int16_t instant_wind_speed_4m; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 10. 기온 (0.5 m)
  int16_t temperature_0_5m; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

  // 11. 기온 (4.0 m)
  int16_t temperature_4m; // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500 [(관측값 + 100) * 10]

  // 12. 습도 (0.5 m, 1분 평균)
  int16_t humidity_0_5m; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  // 13. 습도 (4.0 m, 1분 평균)
  int16_t humidity_4m; // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)
  int16_t temp1[9];
  int16_t tacometer;
  int8_t sensorStatus[8];
  int8_t volateStatus;
  int16_t temp[60];

}kma_data_t;

typedef struct
{
  bool init;
  uint32_t crc;

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

typedef struct rainfall_s
{
  float rainfall_1min;
  float rainfall_10min;
  float rainfall_hourly;
  float rainfall_today;
  float rainfall_yesterday; 
  float rainfall_monthly;  
  float rainfall_yearly;   
}rainfall_t;

typedef struct sunshine_s
{
  uint32_t sunshine_yesterday;
  uint32_t sunshine_today;
  uint32_t sunshine_hourly;
  uint32_t sunshine_monthly;
  uint32_t sunshine_yearly;
} sunshine_t;

rainfall_t *get_rainfall(void);
void set_rainfall_1min(float rainfall);
void set_rainfall_10min(float rainfall);
void set_rainfall_hourly(float rainfall);
void set_rainfall_today(float rainfall);
void set_rainfall_monthly(float rainfall);
void set_rainfall_yesterday(float rainfall);
void set_rainfall_yearly(float rainfall);

sunshine_t *get_sunshine(void);
void set_sunshine_hourly(uint32_t sunshine);
void set_sunshine_today(uint32_t sunshine);
void set_sunshine_monthly(uint32_t sunshine);
void set_sunshine_yesterday(uint32_t sunshine);
void set_sunshine_yearly(uint32_t sunshine);


kma_data_ex_t *get_kma_data(eAWS_DATA_MIN_t min) ;



extern kma_data_t g_kma_inst;
extern kma_data_t g_kma_1min;
extern kma_data_t g_kma_10min;
extern kma_data_t g_kma_hour;


extern kma_data_ex_t g_kma_inst_ex;
extern kma_data_ex_t g_kma_1min_ex;;
extern kma_data_ex_t g_kma_raw_ex;
extern kma_data_ex_t g_kma_10min_ex;
extern kma_data_ex_t g_kma_1Hour_ex;

extern rainfall_t g_rainfall;
#endif