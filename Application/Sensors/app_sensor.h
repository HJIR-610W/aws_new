#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdint.h>
#include <stdbool.h>

#define SENSOR_ERR_COMM 1
#define SENSOR_ERR_VAL  2

//장비가 측정가능한 데이터,이름,printf 에 사용될 format을 정의의
#define SENSOR_LIST                                      \
  X(A1_TEMPERATURE,             "기온", "%-5.2fC,")      \
  X(A2_WIND_DIRECTION,          "풍향", "%-6.2f")        \
  X(A3_WIND_SPEED,              "풍속", "%-5.2fm/s,")    \
  X(A6_RAINFALL_DOT5_1MM,       "강수량", "%-dmm")       \
  X(A7_PRESSURE,                "기압", "%-5.2fbar")     \
  X(A8_RAIN_PRESENT,            "강수유무", "%-d")       \
  X(A9_SNOW_DEPTH,              "적설", "%-dmm")         \
  X(A10_RELATIVE_HUMIDITY,      "상대습도", "%-5.2f")    \
  X(A11_RAINFALL_DOT1MM,        "강수량 0.1", "%-dmm")       \
  X(B1_SOLAR_RADIATION,         "일사", "%-5.2f")        \
  X(B2_SUNSHINE_DURATION,       "일조", "%-5.2f")        \
  X(B3_GROUND_TEMPERATURE,      "지면온도", "%-5.2f")    \
  X(B4_SURFACE_TEMPERATURE,     "초상온도", "%-5.2f")    \
  X(B5_SOIL_TEMPERATURE_5CM,    "지중온도 5cm", "%-5.2f")    \
  X(B6_SOIL_TEMPERATURE_10CM,   "지중온도 10cm", "%-5.2f")    \
  X(B7_SOIL_TEMPERATURE_20CM,   "지중온도 20cm", "%-5.2f")    \
  X(B8_SOIL_TEMPERATURE_30CM,   "지중온도 30cm", "%-5.2f")    \
  X(B9_SOIL_TEMPERATURE_50CM,   "지중온도 50cm", "%-5.2f")    \
  X(B10_SOIL_TEMPERATURE_100CM, "지중온도 1m", "%-5.2f")    \
  X(B11_SOIL_TEMPERATURE_150CM, "지중온도 1.5m", "%-5.2f")    \
  X(B12_SOIL_TEMPERATURE_300CM, "지중온도 3m", "%-5.2f")    \
  X(B13_SOIL_TEMPERATURE_500CM, "지중온도 5m", "%-5.2f")    \
  X(C1_CLOUD_BASE1,             "운고1", "%-5.2f")        \
  X(C2_CLOUD_BASE2,             "운고2", "%-5.2f")        \
  X(C3_CLOUD_BASE3,             "운고3", "%-5.2f")        \
  X(C4_CLOUD_COVER,             "운량", "%-5.2f")        \
  X(C5_VISIBILITY,              "시정", "%-5.2f")        \
  X(C6_PM10,                    "미세먼지 1.0", "%-5.2f")    \
  X(C7_PM2DOT5,                 "미세먼지 2.5", "%-5.2f")    \
  X(C8_NET_RADIATION,           "순복사", "%-5.2f")      \
  X(C9_TOTAL_RADIATION,         "전천복사", "%-5.2f")    \
  X(C10_REFLECTED_RADIATION,    "반사복사", "%-5.2f")    \
  X(C11_DIRECT_SOLAR,           "직달일사", "%-5.2f")    \
  X(C12_CURRENT_WEATHER,        "현재일기", "%-5.2f")    \
  X(N1_SOIL_MOISTURE_10CM,      "토양수분 10cm", "%-5.2f")    \
  X(N2_SOIL_MOISTURE_20CM,      "토양수분 20cm", "%-5.2f")    \
  X(N3_SOIL_MOISTURE_30CM,      "토양수분 30cm", "%-5.2f")    \
  X(N4_SOIL_MOISTURE_50CM,      "토양수분 50cm", "%-5.2f")    \
  X(N5_ILLUMINANCE,             "조도량", "%-5.2f")      \
  X(N6_WIND_VELOCITY_150CM,     "풍속 1.5m", "%-5.2f")        \
  X(N7_WIND_VELOCITY_400CM,     "풍속 4.0m", "%-5.2f")        \
  X(N8_INSTANT_VELOCITY_150CM,  "순간풍속 1.5m", "%-5.2f")    \
  X(N9_INSTANT_VELOCITY_400CM,  "순간풍속 4.0m", "%-5.2f")    \
  X(N10_AIR_TEMPERATURE_50CM,   "기온 50cm", "%-5.2fC")       \
  X(N11_AIR_TEMPERATURE_400CM,  "기온 4m", "%-5.2f")        \
  X(N12_HUMIDITY_50CM,          "습도 50cm", "%-5.2f")        \
  X(N13_HUMIDITY_400CM,         "습도 4m", "%-5.2f")        \
  X(I1_TACHOMETER,              "타코미터", "%-5.2f")    \
  X(USER_WATER,                 "수위", "%-5.2f") 


// 제공 가능한 센서 데이터,AWS 항목
typedef enum sensor_list_e
{
#define X(name, name2,format) name,
  SENSOR_LIST
#undef X
      SENSOR_LIST_MAX
} eSENSOR_LIST_t;


//센서 모델,이름 정의 
#define SENSOR_MODEL_LIST                             \
  X(S_T_UNSUED,  "미사용")                            \
  X(S_T_ADC,     "ADC")                               \
  X(S_T_TEMP_232,"RS232")                             \
  X(S_T_TEMP_485,"RS485")                             \
  X(S_T_MODBUS,  "MODBUS")                            \
  X(S_T_HART,    "HART")                              \
  X(S_T_FREQ_0,  "FREQ_0")                            \
  X(S_T_RAIN_REED_05MM, "REED 0.5mm")                 \
  X(S_T_RAIN_REED_1MM,  "REED 1mm")                   \
  X(S_T_RAIN_HALL_05MM,    "화진 HALL 0.5mm")         \
  X(S_T_RAIN_HALL_1MM,     "화진 HALL 1mm")           \
  X(S_T_DI_0,              "DI_0")                    \
  X(S_T_SNOW_HJ,       "화진 적설")        \
  X(S_T_GENERAL_232,       "GENERAL_RS232")           \
  X(S_T_WIND_SPEED_HJ_485, "화진 풍속")         \
  X(S_T_WIND_DIRECTION_HJ_485, "화진 풍향")     \
  X(S_T_HUMINITY_HJ, "화진 온습도")               \
  X(S_T_WIND_SPEED_MAX_VAL, "WIND_SPEED_MAX")         \
  X(S_T_WIND_DIRECTION_MAX_VAL, "WIND_DIRECTION_MAX") \
  X(S_T_PRESSURE_485, "PRESSURE_RS485")               \
  X(S_T_HUMI_RS485, "HUMI_RS485")                     \
  X(S_T_RAIN_PRESENT_DI, "화진 강우감지")                 \
  X(S_T_GENERAL_485, "GENERAL_485")                   \
  X(S_T_PT100_A, "PT100_A")                           \
  X(S_T_PT100_B, "PT100_B")                           \
  X(S_T_FREQ_A, "FREQ_A")                             \
  X(S_T_FREQ_B, "FREQ_B")                             \
  X(S_T_SUNSHINE, "SUNSHINE")                         \
  X(S_T_SOLAR_RADIATION, "SOLAR_RADIATION")           \
  X(S_T_SOIL_TEMP_5CM, "SOIL_TEMP_5CM")               \
  X(S_T_SOIL_TEMP_10CM, "SOIL_TEMP_10CM")             \
  X(S_T_SOIL_TEMP_20CM, "SOIL_TEMP_20CM")             \
  X(S_T_SOIL_TEMP_30CM, "SOIL_TEMP_30CM")             \
  X(S_T_SOIL_TEMP_50CM, "SOIL_TEMP_50CM")             \
  X(S_T_SOIL_TEMP_100CM, "SOIL_TEMP_100CM")           \
  X(S_T_SOIL_TEMP_150CM, "SOIL_TEMP_150CM")           \
  X(S_T_SOIL_TEMP_300CM, "SOIL_TEMP_300CM")           \
  X(S_T_SOIL_TEMP_500CM, "SOIL_TEMP_500CM")           \
  X(S_T_GENERAL, "GENERAL")                           \
  X(S_T_TEMPERATURE_HJ, "화진 온습도")\

typedef enum sensor_model_e
{
#define X(name, format) name,
  SENSOR_MODEL_LIST
#undef X
      SENSOR_MODEL_MAX
} eSENSOR_MODEL_t;

typedef struct supported_sensors_s
{
  const uint8_t *list;
  uint8_t cnt;
} supported_sensors_t;


typedef enum adcChType_e
{
  eSINGLE_ADC = 0,
  eDIFF_ADC
} eADC_CH_TYPE_t;

typedef struct sensor_s
{
  eSENSOR_MODEL_t type;
  uint8_t configCnt;     // 센서가 가지고 있는 설정값 수 예)
  uint8_t config[4][2];  //[0][0] 센서타입 정보 저장, [0][1] 타입이 할당받은
                         // 설정 위치값 저장
} sensor_t;


void *get_sensor_config(sensor_t *sensor);
void *sensor_add(sensor_t *sensor);

extern const char *g_sensor_model_list[SENSOR_MODEL_MAX];

// TODO:하드 코딩됨, 소스파일과 일치시켜야함 주의
extern const uint8_t temperatureList[4];
extern const uint8_t windDirectionList[3];
extern const uint8_t windSpeedList[3];
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

extern const char *sensor_name_list[SENSOR_LIST_MAX];
extern const char *sensor_format_list[SENSOR_LIST_MAX];
extern const supported_sensors_t supported_sensors[SENSOR_LIST_MAX];





#endif