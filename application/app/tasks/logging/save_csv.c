

#include "save_csv.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "app_file.h"
#include "old_aws_define.h"
#include "user_heap.h"
#include "aws_data.h"

extern   void update_old_to_kma3(AWS_DATA_STRUCT * p_aws_old, kma_data_ex_t * p_kma_ex);
extern uint32_t timeToOffsetDay(time_t current_tick, uint8_t min, uint16_t byte);


void save_aws_csv(uint8_t *p_data,uint16_t data_len)
{
    enum {CSV_LINE_SIZE=187};
    char buff[30];
    char line[CSV_LINE_SIZE];
    int folder;
    int len=0;
    uint32_t file_offset;
    kma_data_ex_t *p_kma;
    uint32_t c_tick;
    DATE_TIME_BUF ct;

    p_kma = user_malloc(sizeof(kma_data_ex_t));

    if(!p_kma)
    {
      return;
    }

    
    p_kma->time.Year = 0;
    p_kma->time.Month = 0;
    p_kma->time.Day = 0;
    p_kma->time.Hour = 0;
    p_kma->time.Min = 0;
    p_kma->time.Sec = 0;

    update_old_to_kma3((AWS_DATA_STRUCT *)p_data,p_kma);

    // 파일 경로 생성: MMDD.csv
    folder = p_kma->time.Year%10;
    snprintf(buff,sizeof(buff),"0:Y%02d/csv/%02d%02d.csv",folder,  p_kma->time.Month,p_kma->time.Day);

    c_tick = time_cvt_timestamp(&p_kma->time);


    file_offset = timeToOffsetDay(c_tick,1,CSV_LINE_SIZE)+CSV_LINE_SIZE ;

    // 한 줄 데이터 생성 (188바이트 고정)
    memset(line, ' ', sizeof(line));

    // 시간 정보 (17자)
    len = snprintf(line, 18, "%04d-%02d-%02d %02d:%02d",
                   p_kma->time.Year, p_kma->time.Month, p_kma->time.Day,
                   p_kma->time.Hour, p_kma->time.Min);
    line[len] = ',';  // snprintf의 null을 콤마로 변경
    len = 18;


    // 기온
    if((int16_t)p_kma->temperature.data == -9999||(int16_t)p_kma->temperature.data == -999)
    {
    len += snprintf(&line[len], 8, "%7d",(int16_t)p_kma->temperature.data);
    }
    else
    {
    len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->temperature.data));
    }
    line[len] = ',';
    len++;

    // 상대습도
    if((int16_t)p_kma->relative_humidity.data == -9999 || (int16_t)p_kma->relative_humidity.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->relative_humidity.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->relative_humidity.data));
    }
    line[len] = ',';
    len++;

    // 풍향
    if((int16_t)p_kma->wind_direction_avg.data == -9999 || (int16_t)p_kma->wind_direction_avg.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->wind_direction_avg.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->wind_direction_avg.data));
    }
    line[len] = ',';
    len++;

    // 풍속
    if((int16_t)p_kma->wind_speed_avg.data == -9999 || (int16_t)p_kma->wind_speed_avg.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->wind_speed_avg.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->wind_speed_avg.data));
    }
    line[len] = ',';
    len++;

    // 순간 풍향
    if((int16_t)p_kma->wind_direction_instant.data == -9999 || (int16_t)p_kma->wind_direction_instant.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->wind_direction_instant.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->wind_direction_instant.data));
    }
    line[len] = ',';
    len++;

    // 순간 풍속
    if((int16_t)p_kma->wind_speed_instant.data == -9999 || (int16_t)p_kma->wind_speed_instant.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->wind_speed_instant.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->wind_speed_instant.data));
    }
    line[len] = ',';
    len++;

    // 강수량
    if((int16_t)p_kma->precipitation.data == -9999 || (int16_t)p_kma->precipitation.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->precipitation.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->precipitation.data));
    }
    line[len] = ',';
    len++;

    // 기압
    if((int16_t)p_kma->pressure.data == -9999 || (int16_t)p_kma->pressure.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->pressure.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_GENERAL(p_kma->pressure.data));
    }
    line[len] = ',';
    len++;

    // 강수유무
    if((int16_t)p_kma->precipitation_presence.data == -9999 || (int16_t)p_kma->precipitation_presence.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->precipitation_presence.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7d", (p_kma->precipitation_presence.data == 10) ? 1 : 0);
    }
    line[len] = ',';
    len++;

    // 적설
    if((int16_t)p_kma->snowfall.data == -9999 || (int16_t)p_kma->snowfall.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->snowfall.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", (float)p_kma->snowfall.data / 10.0);
    }
    line[len] = ',';
    len++;

    // 일사
    if((int16_t)p_kma->solar_radiation.data == -9999 || (int16_t)p_kma->solar_radiation.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->solar_radiation.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", (float)p_kma->solar_radiation.data * 10);
    }
    line[len] = ',';
    len++;

    // 일조
    if((int16_t)p_kma->sunshine_duration.data == -9999 || (int16_t)p_kma->sunshine_duration.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->sunshine_duration.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7d", p_kma->sunshine_duration.data);
    }
    line[len] = ',';
    len++;

    // 지중온도 5cm
    if((int16_t)p_kma->soil_temperature_5cm.data == -9999 || (int16_t)p_kma->soil_temperature_5cm.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_5cm.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 10cm
    if((int16_t)p_kma->soil_temperature_10cm.data == -9999 || (int16_t)p_kma->soil_temperature_10cm.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_10cm.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 20cm
    if((int16_t)p_kma->soil_temperature_20cm.data == -9999 || (int16_t)p_kma->soil_temperature_20cm.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_20cm.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 30cm
    if((int16_t)p_kma->soil_temperature_30cm.data == -9999 || (int16_t)p_kma->soil_temperature_30cm.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_30cm.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 50cm
    if((int16_t)p_kma->soil_temperature_50cm.data == -9999 || (int16_t)p_kma->soil_temperature_50cm.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_50cm.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 1m
    if((int16_t)p_kma->soil_temperature_1m.data == -9999 || (int16_t)p_kma->soil_temperature_1m.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_1m.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 1.5m
    if((int16_t)p_kma->soil_temperature_1_5m.data == -9999 || (int16_t)p_kma->soil_temperature_1_5m.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_1_5m.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 3m
    if((int16_t)p_kma->soil_temperature_3m.data == -9999 || (int16_t)p_kma->soil_temperature_3m.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_3m.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data));
    }
    line[len] = ',';
    len++;

    // 지중온도 5m (마지막)
    if((int16_t)p_kma->soil_temperature_5m.data == -9999 || (int16_t)p_kma->soil_temperature_5m.data == -999)
    {
        len += snprintf(&line[len], 8, "%7d", (int16_t)p_kma->soil_temperature_5m.data);
    }
    else
    {
        len += snprintf(&line[len], 8, "%7.1f", KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data));
    }
    line[len] = ',';
    len++;

    // 줄바꿈
    line[CSV_LINE_SIZE-2] = '\r';
    line[CSV_LINE_SIZE-1] = '\n';

    // offset 방식으로 파일에 쓰기
    write_file(buff, (uint8_t *)line, CSV_LINE_SIZE, file_offset);

    user_free(p_kma);
}