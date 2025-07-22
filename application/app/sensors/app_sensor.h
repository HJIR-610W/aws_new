#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdbool.h>
#include <stdint.h>

#include "config_sensor.h"
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
  X(I1_TACHOMETER,              "타코미터", "%-5.2f")

#define SENSOR_ENG_LIST                                         \
  X(A1_TEMPERATURE, "Temperature", "%-5.2fC,")                 \
  X(A2_WIND_DIRECTION, "Wind Direction", "%-6.2f")                \
  X(A3_WIND_SPEED, "Wind Speed", "%-5.2fm/s,")              \
  X(A6_RAINFALL_DOT5_1MM, "Rainfall", "%-dmm")              \
  X(A7_PRESSURE, "Pressure", "%-5.2fbar")                   \
  X(A8_RAIN_PRESENT, "Rain Detect", "%-d")                  \
  X(A9_SNOW_DEPTH, "Snow Depth", "%-dmm")                   \
  X(A10_RELATIVE_HUMIDITY, "Rel Humidity", "%-5.2f")        \
  X(A11_RAINFALL_DOT1MM, "Rainfall 0.1", "%-dmm")           \
  X(B1_SOLAR_RADIATION, "Solar Rad", "%-5.2f")              \
  X(B2_SUNSHINE_DURATION, "Sunshine", "%-5.2f")             \
  X(B3_GROUND_TEMPERATURE, "Ground Temp", "%-5.2f")         \
  X(B4_SURFACE_TEMPERATURE, "Surface Temp", "%-5.2f")       \
  X(B5_SOIL_TEMPERATURE_5CM, "Soil Temp 5cm", "%-5.2f")     \
  X(B6_SOIL_TEMPERATURE_10CM, "Soil Temp 10cm", "%-5.2f")   \
  X(B7_SOIL_TEMPERATURE_20CM, "Soil Temp 20cm", "%-5.2f")   \
  X(B8_SOIL_TEMPERATURE_30CM, "Soil Temp 30cm", "%-5.2f")   \
  X(B9_SOIL_TEMPERATURE_50CM, "Soil Temp 50cm", "%-5.2f")   \
  X(B10_SOIL_TEMPERATURE_100CM, "Soil Temp 1m", "%-5.2f")   \
  X(B11_SOIL_TEMPERATURE_150CM, "Soil Temp 1.5m", "%-5.2f") \
  X(B12_SOIL_TEMPERATURE_300CM, "Soil Temp 3m", "%-5.2f")   \
  X(B13_SOIL_TEMPERATURE_500CM, "Soil Temp 5m", "%-5.2f")   \
  X(C1_CLOUD_BASE1, "Cloud Base1", "%-5.2f")                \
  X(C2_CLOUD_BASE2, "Cloud Base2", "%-5.2f")                \
  X(C3_CLOUD_BASE3, "Cloud Base3", "%-5.2f")                \
  X(C4_CLOUD_COVER, "Cloud Cover", "%-5.2f")                \
  X(C5_VISIBILITY, "Visibility", "%-5.2f")                  \
  X(C6_PM10, "PM10", "%-5.2f")                              \
  X(C7_PM2DOT5, "PM2.5", "%-5.2f")                          \
  X(C8_NET_RADIATION, "Net Rad", "%-5.2f")                  \
  X(C9_TOTAL_RADIATION, "Total Rad", "%-5.2f")              \
  X(C10_REFLECTED_RADIATION, "Refl Rad", "%-5.2f")          \
  X(C11_DIRECT_SOLAR, "Direct Solar", "%-5.2f")             \
  X(C12_CURRENT_WEATHER, "Current Wx", "%-5.2f")            \
  X(N1_SOIL_MOISTURE_10CM, "Soil Moist 10", "%-5.2f")       \
  X(N2_SOIL_MOISTURE_20CM, "Soil Moist 20", "%-5.2f")       \
  X(N3_SOIL_MOISTURE_30CM, "Soil Moist 30", "%-5.2f")       \
  X(N4_SOIL_MOISTURE_50CM, "Soil Moist 50", "%-5.2f")       \
  X(N5_ILLUMINANCE, "Illuminance", "%-5.2f")                \
  X(N6_WIND_VELOCITY_150CM, "Wind Vel 1.5m", "%-5.2f")      \
  X(N7_WIND_VELOCITY_400CM, "Wind Vel 4.0m", "%-5.2f")      \
  X(N8_INSTANT_VELOCITY_150CM, "Inst Vel 1.5m", "%-5.2f")   \
  X(N9_INSTANT_VELOCITY_400CM, "Inst Vel 4.0m", "%-5.2f")   \
  X(N10_AIR_TEMPERATURE_50CM, "Air Temp 50cm", "%-5.2fC")   \
  X(N11_AIR_TEMPERATURE_400CM, "Air Temp 4m", "%-5.2f")     \
  X(N12_HUMIDITY_50CM, "Humidity 50cm", "%-5.2f")           \
  X(N13_HUMIDITY_400CM, "Humidity 4m", "%-5.2f")            \
  X(I1_TACHOMETER, "Tachometer", "%-5.2f")

// 제공 가능한 센서 데이터,AWS 항목
typedef enum sensor_list_e
{
#define X(name, name2,format) name,
  SENSOR_LIST
#undef X
      SENSOR_LIST_MAX
} eSENSOR_TYPE_t;

//ADDMODEL:센서 타입이 추가되면 여기에도 추가해야함
//센서 모델,이름 정의
#define SENSOR_TYPE_LIST                   \
  X(S_T_UNSUED, "미사용")                   \
  X(S_T_ADC, "ADC")                         \
  X(S_T_FREQ, "GENERAL_FREQ")               \
  X(S_T_RAIN_REED_05MM, "REED 0.5mm")       \
  X(S_T_RAIN_REED_1MM, "REED 1mm")          \
  X(S_T_RAIN_HALL_05MM, "화진 HALL 0.5mm")  \
  X(S_T_RAIN_HALL_1MM, "화진 HALL 1mm")     \
  X(S_T_SNOW_HJ, "화진 적설")               \
  X(S_T_WIND_SPEED_HJ_485, "화진 풍속")     \
  X(S_T_WIND_DIRECTION_HJ_485, "화진 풍향") \
  X(S_T_HUMINITY_HJ, "화진 습도")           \
  X(S_T_TEMPERATURE_HJ, "화진 온도")        \
  X(S_T_RAIN_PRESENT_DI, "화진 강우감지")   \
  X(S_T_PT100_A, "PT100_A")                 \
  X(S_T_PT100_B, "PT100_B")                 \
  X(S_T_SOLAR_RADIATION_OTT_SMP3, "일사 OTT SMP3")

#define SENSOR_TYPE_ENG_LIST                  \
  X(S_T_UNSUED, "Unused")                     \
  X(S_T_ADC, "ADC")                           \
  X(S_T_FREQ, "Freq")                      \
  X(S_T_RAIN_REED_05MM, "Reed 0.5mm")         \
  X(S_T_RAIN_REED_1MM, "Reed 1mm")            \
  X(S_T_RAIN_HALL_05MM, "HJ Hall 0.5")        \
  X(S_T_RAIN_HALL_1MM, "HJ Hall 1mm")         \
  X(S_T_SNOW_HJ, "HJ Snow")                   \
  X(S_T_WIND_SPEED_HJ_485, "HJ Wind Spd")     \
  X(S_T_WIND_DIRECTION_HJ_485, "HJ Wind Dir") \
  X(S_T_HUMINITY_HJ, "HJ Humi")           \
  X(S_T_TEMPERATURE_HJ, "HJ Temp")            \
  X(S_T_RAIN_PRESENT_DI, "HJ Rain Det")       \
  X(S_T_PT100_A, "PT100_A")                   \
  X(S_T_PT100_B, "PT100_B")                   \
  X(S_T_SOLAR_RADIATION_OTT_SMP3, "Solar SMP3")

typedef enum sensor_model_e
{
#define X(name, format) name,
  SENSOR_TYPE_LIST
#undef X
      SENSOR_TYPE_MAX
} eSENSOR_TYPE_MODEL_t;

//센서별 모델 리스트와, 갯수
typedef struct sensor_table_s
{
  const uint8_t *list;
  uint8_t cnt;
} sensor_model_entry_t;


typedef enum adcChType_e
{
  eSINGLE_ADC = 0,
  eDIFF_ADC
} eADC_CH_TYPE_t;

#define SENSOR_CONFIG_TABLE_MAX 2
typedef struct sensor_s
{
  float offset;
  eSENSOR_TYPE_MODEL_t type;
  uint8_t configCnt;     // 센서가 가지고 있는 설정값 수 예)
  uint8_t config[SENSOR_CONFIG_TABLE_MAX][2];  //[0][0] 센서타입 정보 저장, [0][1] 타입이 할당받은
                                               // 설정 위치값 저장
} sensor_t;


void *get_sensor_config(sensor_t *sensor);
void *sensor_add(sensor_t *sensor);
rain_present_config_t *get_rain_present_config(void);



// TODO:하드 코딩됨, 소스파일과 일치시켜야함 주의
extern const uint8_t temperatureList[4];
extern const uint8_t windDirectionList[3];
extern const uint8_t windSpeedList[4];
extern const uint8_t pressureList[2];
extern const uint8_t rainList[5];
extern const uint8_t snowList[2];
extern const uint8_t rainPresentList[2];
extern const uint8_t humiList[3];
extern const uint8_t sunShineList[3];
extern const uint8_t solarRadiationList[3];
extern const uint8_t defaultList[1];
extern const uint8_t soil_temp_list[2];
extern const char *g_sensor_model_table[SENSOR_TYPE_MAX];
extern const char *g_sensor_model_eng_table[SENSOR_TYPE_MAX];
extern const char *sensor_name_list[SENSOR_LIST_MAX];
extern const char *sensor_name_eng_list[SENSOR_LIST_MAX];
extern const char *sensor_format_list[SENSOR_LIST_MAX];
extern const sensor_model_entry_t sensor_table[SENSOR_LIST_MAX];





#endif