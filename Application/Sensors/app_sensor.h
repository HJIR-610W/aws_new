#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdint.h>

#define SENSOR_NOT_INIT 1

// 제공 가능한 센서 목록
typedef enum
{
  A1_TEMPERATURE = 0,               // 기온
  A2_WIND_DIRECTION = 1,            // 풍향
  A3_WIND_SPEED = 2,                // 풍속
  A4_INSTANT_WIND_DIRECTION = 3,    // 순간풍향
  A5_INSTANT_WIND_SPEED = 4,        // 순간풍속
  A6_RAINFALL_DOT5_1MM = 5,         // 강수량
  A7_PRESSURE = 6,                  // 기압
  A8_RAIN_PRESENT = 7,              // 강수유무
  A9_SNOW_DEPTH = 8,                // 적설
  A10_RELATIVE_HUMIDITY = 9,        // 상대습도
  A11_RAINFALL_DOT1MM = 10,         // 강수량
  B1_SOLAR_RADIATION = 11,          // 일사
  B2_SUNSHINE_DURATION = 12,        // 일조
  B3_GROUND_TEMPERATURE = 13,       // 지면온도
  B4_SURFACE_TEMPERATURE = 14,      // 초상온도
  B5_SOIL_TEMPERATURE_5CM = 15,     // 지중온도
  B6_SOIL_TEMPERATURE_10CM = 16,    // 지중온도
  B7_SOIL_TEMPERATURE_20CM = 17,    // 지중온도
  B8_SOIL_TEMPERATURE_30CM = 18,    // 지중온도
  B9_SOIL_TEMPERATURE_50CM = 19,    // 지중온도
  B10_SOIL_TEMPERATURE_100CM = 20,  // 지중온도
  B11_SOIL_TEMPERATURE_150CM = 21,  // 지중온도
  B12_SOIL_TEMPERATURE_300CM = 22,  // 지중온도
  B13_SOIL_TEMPERATURE_500CM = 23,  // 지중온도
  C1_CLOUD_BASE1 = 24,              // 운고
  C2_CLOUD_BASE2 = 25,              // 운고
  C3_CLOUD_BASE3 = 26,              // 운고
  C4_CLOUD_COVER = 27,              // 운량
  C5_VISIBILITY = 28,               // 시정
  C6_PM10 = 29,                     // 미세먼지
  C7_PM2DOT5 = 30,                  // 미세먼지
  C8_NET_RADIATION = 31,            // 순복사
  C9_TOTAL_RADIATION = 32,          // 전천복사
  C10_REFLECTED_RADIATION = 33,     // 반사복사
  C11_DIRECT_SOLAR = 34,            // 직달일사
  C12_CURRENT_WEATHER = 35,         // 현재일기
  N1_SOIL_MOISTURE_10CM = 36,       // 토양수분
  N2_SOIL_MOISTURE_20CM = 37,       // 토양수분
  N3_SOIL_MOISTURE_30CM = 38,       // 토양수분
  N4_SOIL_MOISTURE_50CM = 39,       // 토양수분
  N5_ILLUMINANCE = 40,              // 조도량
  N6_WIND_VELOCITY_150CM = 41,      // 풍속
  N7_WIND_VELOCITY_400CM = 42,      // 풍속
  N8_INSTANT_VELOCITY_150CM = 43,   // 순간풍속
  N9_INSTANT_VELOCITY_400CM = 44,   // 순간풍속
  N10_AIR_TEMPERATURE_50CM = 45,    // 기온
  N11_AIR_TEMPERATURE_400CM = 46,   // 기온
  N12_HUMIDITY_50CM = 47,           // 습도
  N13_HUMIDITY_400CM = 48,          // 습도
  I1_TACHOMETER = 49,               // 타코미터
  USER_WATER = 50,                  // 수위 cm
  USER_SWV = 51,                    // 표면 유속  float m/s
  USER_FLOW_RATE = 52,              // 유량  m³/s
  USER_SLOPE_1 = 53,                // 경사 지점 1
  USER_SLOPE_2 = 54,                // 경사 지점 2
  USER_SLOPE_3 = 55,                // 경사 지점 3
  USER_SLOPE_4 = 56,                // 경사 지점 4
  USER_SLOPE_5 = 57,                // 경사 지점 5
  USER_SLOPE_6 = 58,                // 경사 지점 6
  USER_SLOPE_7 = 59,                // 경사 지점 7
  USER_SLOPE_8 = 60,                // 경사 지점 8
  USER_SLOPE_9 = 61,                // 경사 지점 9
  USER_SLOPE_10 = 62,               // 경사 지점 10
  USER_DEFAULT = 63,                //
  SENSOR_LIST_MAX
} eSENSOR_LIST_t;

/*
센서별 지원되는 센서 모델델
센서 종류_모델
예)적설_화진_RS485   적설센서이며 화진의 RS485 방식
   적설_두성_RS232   적설센서이며 두성의 RS232 방식

   sensorTypeList 와 1:1 맞게 정의되어있어야함함
*/
typedef enum S_T_e
{
  S_T_UNSUED = 0,
  S_T_ADC = 1,
  S_T_TEMP_232 = 2,
  S_T_TEMP_485 = 3,
  S_T_MODBUS = 4,
  S_T_HART = 5,
  S_T_FREQ_0 = 6,
  S_T_RAIN_REED_05MM = 7,
  S_T_RAIN_REED_1MM = 8,
  S_T_RAIN_HALL_05MM = 9,
  S_T_RAIN_HALL_1MM = 10,
  S_T_DI_0 = 11,
  S_T_SNOW_HJ_485 = 12, /* 화진 */
  S_T_GENERAL_232 = 13,
  S_T_WIND_SPEED_HJ_485 = 14,     /* 화진 */
  S_T_WIND_DIRECTION_HJ_485 = 15, /* 화진 */
  S_T_HUMI_HJ_485 = 16,
  S_T_WIND_SPEED_MAX_VAL = 17,
  S_T_WIND_DIRECTION_MAX_VAL = 18,
  S_T_PRESSURE_485 = 19,
  S_T_HUMI_RS485 = 20,
  S_T_RAIN_PRESENT_DI = 21,
  S_T_SNOW_HJ_232 = 22, /* 화진 */
  S_T_GENERAL_485 = 23,
  S_T_PT100_A = 24,
  S_T_PT100_B = 25,
  S_T_FREQ_A = 26,
  S_T_FREQ_B = 27,
  S_T_SUNSHINE = 28,
  S_T_SOLAR_RADIATION = 29,
  S_T_SOIL_TEMP_5CM = 30,
  S_T_SOIL_TEMP_10CM = 31,
  S_T_SOIL_TEMP_20CM = 32,
  S_T_SOIL_TEMP_30CM = 33,
  S_T_SOIL_TEMP_50CM = 34,
  S_T_SOIL_TEMP_100CM = 35,
  S_T_SOIL_TEMP_150CM = 36,
  S_T_SOIL_TEMP_300CM = 37,
  S_T_SOIL_TEMP_500CM = 38,
  S_T_GENERAL = 39,
  S_T_TEMPERATURE_HJ_485 = 40,
  S_T_MAX
} eSENSOR_MODEL_t;

typedef struct supported_sensors_s
{
  const uint8_t *list;
  uint8_t cnt;
} supported_sensors_t;

typedef struct di_s
{
  uint8_t num;  // DI 핀 번호
} di_config_t;

typedef struct rs232_s
{
  uint32_t baud;
  uint8_t port;
  uint8_t parityIdx;
} rs232_config_t;

typedef struct rs485_s
{
  uint32_t baud;
  uint8_t port;
  uint8_t parityIdx;
} rs485_config_t;

typedef struct modbus_s
{
  uint8_t mode;  // 0 rtu 1 tcp 2 ascii
  rs232_config_t rs232;
} modbus_config_t;

typedef struct hart_s
{
  uint8_t id;
  uint8_t pv;
} hart_config_t;

typedef struct sdi_s
{
  uint8_t id;
} sdi_config_t;

typedef struct hjwindSpeed_s
{
  uint8_t rs485_port;
  int32_t offset;
  int32_t full;
} hjwindspeed_config_t;

typedef struct hjwindDirection_s
{
  uint8_t rs485_port;
} hjwindDirection_config_t;

// 화진 적설 232,485
typedef struct hjsnow_config_s
{
  uint8_t port;
} hjsnow_config_t;

// 화진 온도 센서 485만 사용
typedef struct hjtemp_s
{
  uint8_t rs485_port;
} hjtemp_config_t;

typedef enum adcChType_e
{
  eSINGLE_ADC = 0,
  eDIFF_ADC
} eADC_CH_TYPE_t;

typedef struct
{
  uint8_t mode;  // 0 single, 1 diff
  uint8_t channel;
  int32_t highScale;
  int32_t lowScale;
  int32_t scale;    // 원본값에 몇배 곱해졌다의 의미 highScale 100, lowScale 0이면
                    // 0~100으로 값이 나옴
                    //  scale 10이면 최종 값은 나누기 10해야함
  int32_t outMaxV;  // 센서의 출력 전압 최고
  int32_t outMinV;  // 센서의 출력 전압 최저
} adc_config_t;

typedef struct sensor_s
{
  eSENSOR_MODEL_t type;
  uint8_t configCnt;     // 센서가 가지고 있는 설정값 수 예)
  uint8_t config[4][2];  //[0][0] 센서타입 정보 저장, [0][1] 타입이 할당받은
                         // 설정 위치값 저장
} sensor_t;

/*
config[0][0] = S_T_HUMI_RS485
config[0][1] = 3
RS485는 config_manager  rs485 coing 배열 3을 사용한다는 의미

*/
#include <stdbool.h>
#define SENSOR_ERR_CFG 2

#define DATA_TYPE_I 0
#define DATA_TYPE_F 1
typedef struct sensor_data_s
{
  union aws_data
  {
    int32_t i;
    float f;
    bool b;
  } data;
  union
  {
    int32_t i;
    float f;
  } min;
  union
  {
    int32_t i;
    float f;
  } max;
  float unitScale;
  float avg;
  void *opt;
  uint8_t sample_cnt;
  uint8_t enable : 1;
  uint8_t dataType : 3;
  uint8_t status : 7;
} sensor_data_t;

typedef struct sensor_emul_s
{
  union
  {
    int32_t i;
    float f;
  } data;
  uint8_t type : 3;
  uint8_t enable : 1;
  uint8_t user : 4;
} sensor_emul_t;

typedef struct config_manage_s
{
  uint8_t adc_cnt;
  adc_config_t adc[50];
  uint8_t rs232_cnt;
  rs232_config_t rs232[10];
  uint8_t rs485_cnt;
  rs485_config_t rs485[10];
  uint8_t modbus_cnt;
  modbus_config_t modbus[10];
  uint8_t di_cnt;
  di_config_t di[10];
  uint8_t hart_cnt;
  hart_config_t hart[2];
  uint8_t sdi_cnt;
  sdi_config_t sdi[2];
  uint8_t hjwind_cnt;
  hjwindspeed_config_t hjwind[1];
  uint8_t hjtemp_cnt;
  hjtemp_config_t hjtemp[2];
  uint8_t hjwindDir_cnt;
  hjwindDirection_config_t hjwindDir[1];

  uint8_t hjsnow_cnt;
  hjsnow_config_t hjsnow[2];
} config_manager_t;

void *get_sensor_config(sensor_t *sensor);
void *sensor_add(sensor_t *sensor);

void sensorData_init(void);
void update_sensorData1s(void);
void update_sensorData1min(void);

extern config_manager_t s_config;

extern const char *sensorTypeList[42];

// TODO:하드 코딩됨, 소스파일과 일치시켜야함 주의
extern const uint8_t temperatureList[4];
extern const uint8_t windDirectionList[5];
extern const uint8_t windSpeedList[5];
extern const uint8_t windDirectionInstantList[2];
extern const uint8_t windSpeedInstantList[2];
extern const uint8_t pressureList[3];
extern const uint8_t rainList[6];
extern const uint8_t snowList[3];
extern const uint8_t rainPresentList[2];
extern const uint8_t humiList[3];
extern const uint8_t sunShineList[3];
extern const uint8_t solarRadiationList[3];
extern const uint8_t soilTemp5cmList[3];
extern const uint8_t soilTemp10cmList[3];
extern const uint8_t soilTemp20cmList[3];
extern const uint8_t soilTemp30cmList[3];
extern const uint8_t soilTemp50cmList[3];
extern const uint8_t soilTemp100cmList[3];
extern const uint8_t soilTemp150cmList[3];
extern const uint8_t soilTemp300cmList[3];
extern const uint8_t soilTemp500cmList[3];

extern const uint8_t temperature50cmList[2];

extern const char *sensorNameList[SENSOR_LIST_MAX];
extern const char *dataFmtList[SENSOR_LIST_MAX];
extern sensor_emul_t g_sensor_emul[SENSOR_LIST_MAX];
extern sensor_data_t sensor_data[SENSOR_LIST_MAX];
extern sensor_data_t sensor_data_1s[SENSOR_LIST_MAX];    // 1초마다 갱신되는 자료
extern sensor_data_t sensor_data_real[SENSOR_LIST_MAX];  // 실시간;

extern const supported_sensors_t supported_sensors[SENSOR_LIST_MAX];

#endif