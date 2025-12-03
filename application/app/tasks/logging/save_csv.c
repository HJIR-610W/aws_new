

#include "save_csv.h"

#include <stdio.h>
#include <stdint.h>
#include "app_file.h"

#include "app_file.h"
#include "old_aws_define.h"
#include "user_heap.h"

#include "aws_data.h"

extern   void update_old_to_kma3(AWS_DATA_STRUCT * p_aws_old, kma_data_ex_t * p_kma_ex);

void save_aws_csv(uint8_t *p_data,uint16_t data_len)
{
    enum {CSV_SIZE=102400};
    char buff[30];
    int folder;
    int16_t len=0;
    FSIZE_t file_size;
    kma_data_ex_t *p_kma;

    char *p_buffer = user_malloc(CSV_SIZE);

    if (!p_buffer)
        return;

    p_kma = user_malloc(sizeof(kma_data_ex_t));

    if(!p_kma)
    {
      user_free(p_buffer);
      return;
    }

    update_old_to_kma3((AWS_DATA_STRUCT *)p_data,p_kma);

    // 파일 경로 생성
    folder = p_kma->time.Year%10;
    snprintf(buff,sizeof(buff),"%02d/csv/%04d%02d%02d_%02d%02d.csv",folder,p_kma->time.Year,
          p_kma->time.Month,p_kma->time.Day,p_kma->time.Hour,p_kma->time.Min);

    // 파일이 없거나 크기가 0이면 헤더 작성
    if (get_file_size(buff, &file_size) != FR_OK || file_size == 0)
    {
        len = snprintf(&p_buffer[len],CSV_SIZE-len,"Time,");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Temp(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Humi(%%),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Wind Dir(deg),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Wind Spd(m/s),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Wind GD(deg),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Wind GS(m/s),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Rain(mm),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Press(hpa),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Rain P,");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Snow(cm),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Solar R(kJ/m2),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"Solar D(sec),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 5cm(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 10cm(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 20cm(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 30cm(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 50cm(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 1m(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 1.5m(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 3m(C),");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"ST 5m(C)");
        len += snprintf(&p_buffer[len],CSV_SIZE-len,"\r\n");
    }

    // 시간 정보
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%04d-%02d-%02d %02d:%02d,",p_kma->time.Year,
    p_kma->time.Month,p_kma->time.Day,p_kma->time.Hour,p_kma->time.Min);

    // 기온
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->temperature.data));

    // 상대습도
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->relative_humidity.data));

    // 풍향
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->wind_direction_avg.data));

    // 풍속
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->wind_speed_avg.data));

    // 순간 풍향
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->wind_direction_instant.data));

    // 순간 풍속
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->wind_speed_instant.data));

    // 강수량
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->precipitation.data));

    // 기압
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_GENERAL(p_kma->pressure.data));

    // 강수유무
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%d,",(p_kma->precipitation_presence.data == 10) ? 1 : 0);

    // 적설
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",(float)p_kma->snowfall.data / 10.0);

    // 일사
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",(float)p_kma->solar_radiation.data * 10);

    // 일조
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%d,",p_kma->sunshine_duration.data);

    // 지중온도 5cm
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_5cm.data));

    // 지중온도 10cm
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_10cm.data));

    // 지중온도 20cm
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_20cm.data));

    // 지중온도 30cm
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_30cm.data));

    // 지중온도 50cm
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_50cm.data));

    // 지중온도 1m
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_1m.data));

    // 지중온도 1.5m
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_1_5m.data));

    // 지중온도 3m
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f,",KMA_TO_TEMPERATURE(p_kma->soil_temperature_3m.data));

    // 지중온도 5m
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"%.1f",KMA_TO_TEMPERATURE(p_kma->soil_temperature_5m.data));

    // 줄바꿈
    len += snprintf(&p_buffer[len],CSV_SIZE-len,"\r\n");

    // 파일에 저장
    append_file(buff,(uint8_t *)p_buffer,len);
    
    
    user_free(p_kma);
    
    
    user_free(p_buffer);

}