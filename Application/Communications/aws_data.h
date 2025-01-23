
#ifndef AWS_DATA_H
#define AWS_DATA_H


#include <stdint.h>
#include <stdbool.h>

typedef enum
{
A1_TEMPERATURE            = 0,   // 기온
A2_WIND_DIRECTION         = 1,   // 풍향
A3_WIND_SPEED             = 2,   // 풍속
A4_INSTANT_WIND_DIRECTION = 3,   // 순간풍향
A5_INSTANT_WIND_SPEED     = 4,   // 순간풍속
A6_RAINFALL_DOT5_1MM      = 5,   // 강수량
A7_PRESSURE               = 6,   // 기압
A8_RAIN_PRESENT           = 7,   // 강수유무
A9_SNOW_DEPTH             = 8,   // 적설
A10_RELATIVE_HUMIDITY     = 9,   // 상대습도
A11_RAINFALL_DOT1MM      = 10,   // 강수량

B1_SOLAR_RADIATION        = 11,  // 일사
B2_SUNSHINE_DURATION      = 12,  // 일조
B3_GROUND_TEMPERATURE     = 13,  // 지면온도
B4_SURFACE_TEMPERATURE    = 14,  // 초상온도
B5_SOIL_TEMPERATURE_5CM   = 15,  // 지중온도
B6_SOIL_TEMPERATURE_10CM  = 16,  // 지중온도
B7_SOIL_TEMPERATURE_20CM  = 17,  // 지중온도
B8_SOIL_TEMPERATURE_30CM  = 18,  // 지중온도
B9_SOIL_TEMPERATURE_50CM  = 19,  // 지중온도
B10_SOIL_TEMPERATURE_100CM = 20, // 지중온도
B11_SOIL_TEMPERATURE_150CM = 21, // 지중온도
B12_SOIL_TEMPERATURE_300CM = 22, // 지중온도
B13_SOIL_TEMPERATURE_500CM = 23, // 지중온도

C1_CLOUD_BASE1 = 24,          // 운고
C2_CLOUD_BASE2 = 25,          // 운고
C3_CLOUD_BASE3 = 26,          // 운고
C4_CLOUD_COVER = 27,          // 운량
C5_VISIBILITY  = 28,          // 시정
C6_PM10        = 29,
C7_PM2DOT5     = 30,
C8_NET_RADIATION        = 31,  // 순복사
C9_TOTAL_RADIATION      = 32,  // 전천복사
C10_REFLECTED_RADIATION = 33,  // 반사복사
C11_DIRECT_SOLAR        = 34,  // 직달일사
C12_CURRENT_WEATHER     = 35,  // 현재일기

N1_SOIL_MOISTURE_10CM   = 36,  // 토양수분
N2_SOIL_MOISTURE_20CM   = 37,  // 토양수분
N3_SOIL_MOISTURE_30CM   = 38,  // 토양수분
N4_SOIL_MOISTURE_50CM   = 39,  // 토양수분
N5_ILLUMINANCE            = 40, // 조도량
N6_WIND_VELOCITY_150CM    = 41, // 풍속
N7_WIND_VELOCITY_400CM    = 42, // 풍속
N8_INSTANT_VELOCITY_150CM = 43, // 순간풍속
N9_INSTANT_VELOCITY_400CM = 44, // 순간풍속
N10_AIR_TEMPERATURE_50CM  = 45, // 기온
N11_AIR_TEMPERATURE_400CM = 46, // 기온
N12_HUMIDITY_50CM         = 47, // 습도
N13_HUMIDITY_400CM        = 48, // 습도
I1_TACHOMETER             = 49, // 타코미터

FAN_STATUS = 63
}eSENSOR_LIST_t;



typedef struct
{
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
  int16_t precipitation; // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

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
}kma_data_t;




typedef struct sensor_emul_s
{
  const char *name;
  int16_t data;
  bool use;
}sensor_emul_t;

typedef struct sensor_data_s
{
  int16_t data;
  int16_t max;
  int16_t min;
  uint8_t status;
  float unitScale;
}sensor_data_t;

extern kma_data_t kma_data_1s;
extern sensor_emul_t g_sensor_emul[50];
extern const char *sensorNameList[50];
extern sensor_data_t sensor_data[50];
#endif