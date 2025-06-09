
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_version.h"
#include "aws_data.h"
#include "config_app.h"
#include "util_crc16_ccitt.h"
#include "kma3.h"
#include "util_memory.h"
#include "util_time.h"
#include "kma_define.h"
#include "kma_protocol_handler.h"
#include "bsp.h"
#include "kma2.h"
#include "app_dataLogging.h"
#include "old_aws_define.h"
#include "Protocols\divas\divas_protocol_handler.h"
#include "user_heap.h"
#include "parse_aws.h"
#include "dev_io.h"

#define KMA_HEADER_START 0xFAFB
#define KMA_HEADER_END 0xFFFE
#define KMA_REQUEST_LEN 29

typedef enum
{
  eKMA_COMMAND_TYPE_AI,  // 순간자료
  eKMA_COMMAND_TYPE_AB,  // 1분 자료
  eKMA_COMMAND_TYPE_AQ,  // 1분 과거 자료
  eKMA_COMMAND_TYPE_AV,  // 데이터로거 버전
  eKMA_COMMAND_TYPE_AR,  // 데이터로거 리셋
  eKMA_COMMAND_TYPE_AO,  // 전원리셋 또는 모뎀리셋 또는 적설센서 리셋
  eKMA_COMMAND_TYPE_AD,  // 지점번호 설정
  eKMA_COMMAND_TYPE_AT,  // 날짜, 시간 설정
  eKMA_COMMAND_TYPE_AW,  // 암호 설정
  eKMA_COMMAND_TYPE_AC,  // 저장데이터 삭제
  eKMA_COMMAND_TYPE_AP,  // 비공식 화진 추가
  eKMA_COMMAND_TYPE_UNKNOWN
} eKMA_COMMAND_TYPE_t;

typedef struct
{
  eKMA_COMMAND_TYPE_t type;
  const char *str;
} kma_commands_t;

const kma_commands_t kma_commands[] = {
    {eKMA_COMMAND_TYPE_AI, "AI?"}, {eKMA_COMMAND_TYPE_AB, "AB?"}, {eKMA_COMMAND_TYPE_AQ, "AQ?"},
    {eKMA_COMMAND_TYPE_AV, "AV?"}, {eKMA_COMMAND_TYPE_AR, "AR?"}, {eKMA_COMMAND_TYPE_AO, "AO?"},
    {eKMA_COMMAND_TYPE_AD, "AD?"}, {eKMA_COMMAND_TYPE_AT, "AT?"}, {eKMA_COMMAND_TYPE_AW, "AW?"},
    {eKMA_COMMAND_TYPE_AC, "AC?"}, {eKMA_COMMAND_TYPE_AP, "AP?"}};

uint16_t count_kma_commands(void) { return _countof(kma_commands); }



uint16_t calculate_old_Y_status(uint8_t kma3_status[8]);
uint8_t calculate_old_Z_status(uint8_t kma3_status[8]);


eKMA_COMMAND_TYPE_t kma_get_command_type(const char *cmd_str_from_packet)
{
  for (size_t i = 0; i < _countof(kma_commands); ++i)
  {
    if (strncmp(cmd_str_from_packet, kma_commands[i].str, strlen(kma_commands[i].str)) == 0)
    {
      return kma_commands[i].type;
    }
  }
  return eKMA_COMMAND_TYPE_UNKNOWN;
}

#define KMA3_REQ_LEN 29

bool is_kma3_protocol(uint8_t *input, uint32_t len)
{
  uint16_t crc;
  uint16_t recv_crc;

  if (input[0] == 0xFA && input[1] == 0xFB)
  {
    crc = crc16_ccitt_table(&input[2], KMA3_REQ_LEN - 6);
    crc = swap_uint16(crc);
    memcpy(&recv_crc, &input[KMA3_REQ_LEN - 4], 2);

    if (crc == recv_crc)
    {
      return true;
    }
  }
  return false;
}

bool is_kma2_protocol(uint8_t *input, uint32_t len)
{
  if (len != KMA_REQUEST_LEN)
  {
    return false;
  }

  uint16_t start_marker = GetWord(&input[0]);
  if (start_marker != KMA_HEADER_START)
  {
    return false;
  }

  uint16_t end_marker = GetWord(&input[len - 2]);
  if (end_marker != KMA_HEADER_END)
  {
    return false;  // 끝 마커 불일치
  }

  const uint8_t *checksum_data_ptr = &input[2];
  uint16_t checksum_data_len = KMA_REQUEST_LEN - 6;

  uint8_t calculated_xor = calculate_xor_checksum(checksum_data_ptr, checksum_data_len);
  uint8_t calculated_sum = make_sum((uint8_t *)checksum_data_ptr, checksum_data_len);
  uint8_t received_xor = input[len - 4];
  uint8_t received_sum = input[len - 3];

  if (calculated_xor == received_xor && calculated_sum == received_sum)
  {
    return true;
  }

  return false;
}

void kma_unpack(uint8_t *packet,kma3_command_request_t *req)
{
  uint16_t usData;

  req->protocol_yy  = packet[2];
  req->protocol_mm = packet[3];
  req->protocol_dd  = packet[4];

  req->date_yy  = packet[5];
  req->date_mm = packet[6];
  req->date_dd   = packet[7];
  req->time_hh  = packet[8];
  req->time_mm   = packet[9];
  req->time_ss   = packet[10];

  usData = GetWord((uint8_t *)&packet[11]);
  req->password = usData;

  usData = GetWord((uint8_t *)&packet[13]);
  req->station_id = usData;

  memcpy(req->command_str,&packet[15],10);

}

#define KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE 0  // 필수 및 선택관측
#define KMA2_DATA_FORMAT_ESSENTIAL 1            // 필수관측
#define KMA2_DATA_FORMAT_PRECIPITATION 2        // 강수량관측

uint8_t calculate_kma2_data_format_no(const kma_data_ex_t *p_kma)
{
  bool selective_elements = false;

  if (p_kma->solar_radiation.enable ||        // a. 일사
      p_kma->sunshine_duration.enable ||      // b. 일조
      p_kma->surface_temperature.enable ||    // c. 지면온도
      p_kma->grass_temperature.enable ||      // d. 초상온도
      p_kma->soil_temperature_5cm.enable ||   // e. 지중온도 5cm
      p_kma->soil_temperature_10cm.enable ||  // f. 지중온도 10cm
      p_kma->soil_temperature_20cm.enable ||  // g. 지중온도 20cm
      p_kma->soil_temperature_30cm.enable ||  // h. 지중온도 30cm
      p_kma->soil_temperature_50cm.enable ||  // i. 지중온도 50cm
      p_kma->soil_temperature_1m.enable ||    // j. 지중온도 1.0m
      p_kma->soil_temperature_1_5m.enable ||  // k. 지중온도 1.5m
      p_kma->soil_temperature_3m.enable ||    // l. 지중온도 3.0m
      p_kma->soil_temperature_5m.enable       // m. 지중온도 5.0m
  )
  {
    selective_elements = true;
  }

  if (selective_elements)
  {
    return KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE;  // 자료형식 0: 필수 및 선택관측
  }

  bool precipitation_active = p_kma->precipitation.enable || p_kma->precipitation_fine.enable;
  bool other_major_essentials_inactive =
      !p_kma->temperature.enable && 
      !p_kma->wind_direction_avg.enable && 
      !p_kma->pressure.enable &&
      !p_kma->relative_humidity.enable && 
      !p_kma->snowfall.enable &&
      !p_kma->precipitation_presence.enable;
  //강우량만 선택된 경우 
  if (precipitation_active && other_major_essentials_inactive)
  {
    return KMA2_DATA_FORMAT_PRECIPITATION;  
  }

  return KMA2_DATA_FORMAT_ESSENTIAL;  
}

void cvt_kma3_to_kma2(kma_data_ex_t *p_kma3, kma2_response_t *p_kma2)
{
  p_kma2->temperature = p_kma3->temperature.data;
  p_kma2->wind_direction_avg = p_kma3->wind_direction_avg.data;
  p_kma2->wind_speed_avg = p_kma3->wind_speed_avg.data;
  p_kma2->wind_direction_instant = p_kma3->wind_direction_instant.data;
  p_kma2->wind_speed_instant = p_kma3->wind_speed_instant.data;
  p_kma2->precipitation = p_kma3->precipitation.data;
  p_kma2->pressure = p_kma3->pressure.data;
  p_kma2->precipitation_presence = p_kma3->precipitation_presence.data;
  p_kma2->snowfall = p_kma3->snowfall.data;
  p_kma2->relative_humidity = p_kma3->relative_humidity.data;
  p_kma2->precipitation_fine = p_kma3->precipitation_fine.data;
  p_kma2->L1 = 0;
  p_kma2->L2 = 0;
  p_kma2->L3 = 0;
  p_kma2->L4 = 0;
  p_kma2->L5 = 0;
  p_kma2->L6 = 0;
  p_kma2->L7 = 0;
  p_kma2->L8 = 0;
  p_kma2->L9 = 0;
  p_kma2->L10 = 0; 
  p_kma2->solar_radiation = p_kma3->solar_radiation.data;
  p_kma2->sunshine_duration = p_kma3->sunshine_duration.data;
  p_kma2->surface_temperature = p_kma3->surface_temperature.data;
  p_kma2->grass_temperature = p_kma3->grass_temperature.data;
  p_kma2->soil_temperature_5cm = p_kma3->soil_temperature_5cm.data;
  p_kma2->soil_temperature_10cm = p_kma3->soil_temperature_10cm.data;
  p_kma2->soil_temperature_20cm = p_kma3->soil_temperature_20cm.data;
  p_kma2->soil_temperature_30cm = p_kma3->soil_temperature_30cm.data;
  p_kma2->soil_temperature_50cm = p_kma3->soil_temperature_50cm.data;
  p_kma2->soil_temperature_1m = p_kma3->soil_temperature_1m.data;
  p_kma2->soil_temperature_1_5m = p_kma3->soil_temperature_1_5m.data;
  p_kma2->soil_temperature_3m = p_kma3->soil_temperature_3m.data;
  p_kma2->soil_temperature_5m = p_kma3->soil_temperature_5m.data;
  p_kma2->S1 = 0;
  p_kma2->S2 = 0;
  p_kma2->S3 = 0;
  p_kma2->S4 =0;
  p_kma2->S5 = 0;
  p_kma2->S6 = 0;
  p_kma2->S7 = 0;
  p_kma2->S8 = 0;
  p_kma2->S9 = 0;
  p_kma2->S10 = 0;
  p_kma2->X_voltage_status =p_kma3->Y_volateStatus;
  p_kma2->Y_logger_status = calculate_old_Y_status(p_kma3->X_sensorStatus); 
  p_kma2->Z_logger_status = calculate_old_Z_status(p_kma3->X_sensorStatus); 
}

    // 순간 자료
uint16_t kma_cmd_handler_AI(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t data_format_no;
  uint8_t data[200];
  uint16_t len=0;
  uint16_t station_id;
  kma_data_ex_t *p_kma3_data;
  kma2_response_t kma2_response;

    
  station_id = GetWord((uint8_t *)&rx_frame[13]);
  p_kma3_data = get_kma_data(eAWS_DATA_AVG);

  switch (get_config_app()->aws_protocol_type)
  {
    case eAWS_PROTOCOL_KMA2:
      //센서 사용 여부를 조사해서 자로형식을 결정한다.
      data_format_no = calculate_kma2_data_format_no(p_kma3_data);
      
      cvt_kma3_to_kma2(p_kma3_data, &kma2_response);

      switch (data_format_no)
      {
        case KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE:
          len = make_kma2_essential_selective(data, &kma2_response);
           break;
        case KMA2_DATA_FORMAT_ESSENTIAL:
          len = make_kma2_essential(data, &kma2_response);
          break;
        case KMA2_DATA_FORMAT_PRECIPITATION:
          len = make_kma2_precipitation(data, &kma2_response);
          break;
      }
      if(len)
      {
        len = make_kma2_response(rx_frame, data, len, data_format_no, tx_frame);
      }
      break;
    case eAWS_PROTOCOL_KMA3:
      len = make_kma3_data_unusedSesor(data, sizeof(data), p_kma3_data);
      len = make_kma3_resp(tx_frame, 'I', DATA_TYPE_GENERAL, station_id, data, len);
      break;
  }


  return len;
}

// 1분 자료
uint16_t kma_cmd_handler_AB(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t data[200];
  uint16_t len = 0;
  uint16_t station_id;
  kma_data_ex_t *p_kma_data;
  kma2_response_t kma2_response;
  uint8_t data_format_no;

  station_id = GetWord((uint8_t *)&rx_frame[13]);
  p_kma_data = get_kma_data(eAWS_DATA_1MIN);

  switch (get_config_app()->aws_protocol_type)
  {
    case eAWS_PROTOCOL_KMA2:
      data_format_no = calculate_kma2_data_format_no(p_kma_data);
      cvt_kma3_to_kma2(p_kma_data, &kma2_response);

      switch (data_format_no)
      {
        case KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE:
          len = make_kma2_essential_selective(data, &kma2_response);
          break;
        case KMA2_DATA_FORMAT_ESSENTIAL:
          len = make_kma2_essential(data, &kma2_response);
          break;
        case KMA2_DATA_FORMAT_PRECIPITATION:
          len = make_kma2_precipitation(data, &kma2_response);
          break;
      }
      len = make_kma2_response(rx_frame, data, len, data_format_no, tx_frame);
      break;
    case eAWS_PROTOCOL_KMA3:
      len = make_kma3_data_unusedSesor(data, sizeof(data), p_kma_data);
      len = make_kma3_resp(tx_frame, 'B', DATA_TYPE_GENERAL, station_id, data, len);
      break;
  }

  return len;
}



uint16_t calculate_old_Y_status(uint8_t kma3_status[8])
{
  uint16_t status=0;

  if (kma3_is_sensor_error(A3_WIND_SPEED, kma3_status))
  {
    status |= WINDSPEEDFAIL_BIT;
  }

  if (kma3_is_sensor_error(A2_WIND_DIRECTION, kma3_status))
  {
    status |= WINDDIRECFAIL_BIT;
  }

  if (kma3_is_sensor_error(A1_TEMPERATURE, kma3_status))
  {
    status |= TEMPERATUREFAIL_BIT;
  }

  if (kma3_is_sensor_error(A8_RAIN_PRESENT, kma3_status))
  {
    status |= RAINDETECTFAIL_BIT;
  }

  if (kma3_is_sensor_error(A6_RAINFALL_DOT5_1MM, kma3_status))
  {
    status |= RAINFALLFAIL_BIT;
  }

  if (kma3_is_sensor_error(A10_RELATIVE_HUMIDITY, kma3_status))
  {
    status |= HUMIDITYFAIL_BIT;
  }

  if (kma3_is_sensor_error(A7_PRESSURE, kma3_status))
  {
    status |= BAROMETRICFAIL_BIT;
  }

  if (kma3_is_sensor_error(I1_TACHOMETER, kma3_status))
  {
    status |= FANFAIL_BIT;
  }

  return status;
}

uint8_t calculate_old_Z_status(uint8_t kma3_status[8])
{
  uint8_t status = 0;

  if (kma3_is_sensor_error(A6_RAINFALL_DOT5_1MM, kma3_status))
  {
    status |= RAINFAIL_BIT;
  }

  return status;
}
  /*
  AWS(구) KMA2에서는 센서 에러는 16bit로 처리됨(AWS 규격참고)
  KMA3에서는 64bit로 처리됨(AWS 규격서 참고)
  따라서 16bit로 처리되던것을 64bit로 변환해줌
  */
  void update_old_status(uint16_t status, uint8_t kma3_status[8])
  {
    if (status & WINDSPEEDFAIL_BIT)
    {
      kma3_set_sensor_status(A3_WIND_SPEED, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A3_WIND_SPEED, kma3_status);
    }

    if (status & WINDDIRECFAIL_BIT)
    {
      kma3_set_sensor_status(A2_WIND_DIRECTION, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A2_WIND_DIRECTION, kma3_status);
    }

    if (status & TEMPERATUREFAIL_BIT)
    {
      kma3_set_sensor_status(A1_TEMPERATURE, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A1_TEMPERATURE, kma3_status);
    }

    if (status & RAINDETECTFAIL_BIT)
    {
      kma3_set_sensor_status(A8_RAIN_PRESENT, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A8_RAIN_PRESENT, kma3_status);
    }

    if (status & RAINFALLFAIL_BIT)
    {
      kma3_set_sensor_status(A6_RAINFALL_DOT5_1MM, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A6_RAINFALL_DOT5_1MM, kma3_status);
    }

    if (status & HUMIDITYFAIL_BIT)
    {
      kma3_set_sensor_status(A10_RELATIVE_HUMIDITY, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A10_RELATIVE_HUMIDITY, kma3_status);
    }

    if (status & BAROMETRICFAIL_BIT)
    {
      kma3_set_sensor_status(A7_PRESSURE, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(A7_PRESSURE, kma3_status);
    }

    if (status & FANFAIL_BIT)
    {
      kma3_set_sensor_status(I1_TACHOMETER, kma3_status);
    }
    else
    {
      kma3_clear_sensor_status(I1_TACHOMETER, kma3_status);
    }
  }

  void update_old_to_kma3(AWS_DATA_STRUCT * p_aws_old, kma_data_ex_t * p_kma_ex)
  {


    p_kma_ex->temperature.data = p_aws_old->mTemperature.sReal;
    p_kma_ex->wind_direction_avg.data = p_aws_old->mWind.mDirection.sReal;
    p_kma_ex->wind_speed_avg.data = p_aws_old->mWind.mSpeed.sReal;
    p_kma_ex->wind_speed_instant.data = p_aws_old->mWind.mSpeed.sMax;
    p_kma_ex->wind_direction_instant.data = p_aws_old->mWind.mDirection.sMax;
    p_kma_ex->precipitation.data = p_aws_old->mRainFall.sReal;
    p_kma_ex->pressure.data = p_aws_old->mBarometric.sReal;
    p_kma_ex->precipitation_presence.data = p_aws_old->mRainDetect.sReal;
    p_kma_ex->snowfall.data = p_aws_old->mSnowFall.sReal;
    p_kma_ex->relative_humidity.data = p_aws_old->mHumidity.sReal;
    p_kma_ex->precipitation_fine.data = 0;  // 미사용

    p_kma_ex->solar_radiation.data = p_aws_old->mSolarRad.sReal;
    p_kma_ex->sunshine_duration.data = p_aws_old->mSunshine.sReal;
    p_kma_ex->grass_temperature.data = 0;
    p_kma_ex->soil_temperature_5cm.data = p_aws_old->mSoilTemp5cm.sReal;
    p_kma_ex->soil_temperature_10cm.data = p_aws_old->mSoilTemp10cm.sReal;
    p_kma_ex->soil_temperature_20cm.data = p_aws_old->mSoilTemp20cm.sReal;
    p_kma_ex->soil_temperature_30cm.data = p_aws_old->mSoilTemp30cm.sReal;
    p_kma_ex->soil_temperature_50cm.data = p_aws_old->mSoilTemp50cm.sReal;
    p_kma_ex->soil_temperature_1m.data = p_aws_old->mSoilTemp1_0m.sReal;
    p_kma_ex->soil_temperature_1_5m.data = p_aws_old->mSoilTemp1_5m.sReal;
    p_kma_ex->soil_temperature_3m.data = p_aws_old->mSoilTemp3_0m.sReal;
    p_kma_ex->soil_temperature_5m.data = p_aws_old->mSoilTemp5_0m.sReal;

    p_kma_ex->Y_volateStatus = (uint8_t)(p_aws_old->mStatus.sReal & 0xFF);

    for (int i = 0; i < _countof(p_kma_ex->X_sensorStatus); i++)
    {
      p_kma_ex->X_sensorStatus[i] = 0;
    }

    switch (get_config_app()->aws_protocol_type)
    {
      case eAWS_PROTOCOL_KMA2:
        update_old_status(p_aws_old->mStatus.sMin, p_kma_ex->X_sensorStatus);
        break;
      case eAWS_PROTOCOL_KMA3:
        for (int i = 0; i < 8; i++)
        {
          p_kma_ex->X_sensorStatus[i] = p_aws_old->kma3_sensor_status[i];
        }
        break;
    }
  }

  //현재값 기준으로 enable적용
  void update_sensor_enable(kma_data_ex_t *p_kma)
  {
    kma_data_ex_t *p_kma_avg;

    p_kma_avg = get_kma_data(eAWS_DATA_AVG);

    p_kma->temperature.enable =p_kma_avg->temperature.enable;
    p_kma->wind_direction_avg.enable = p_kma_avg->wind_direction_avg.enable;
    p_kma->wind_speed_avg.enable = p_kma_avg->wind_speed_avg.enable;
    p_kma->wind_direction_instant.enable = p_kma_avg->wind_direction_instant.enable;
    p_kma->wind_speed_instant.enable = p_kma_avg->wind_speed_instant.enable;
    p_kma->precipitation.enable = p_kma_avg->precipitation.enable;
    p_kma->pressure.enable = p_kma_avg->pressure.enable;
    p_kma->precipitation_presence.enable = p_kma_avg->precipitation_presence.enable;
    p_kma->snowfall.enable = p_kma_avg->snowfall.enable;
    p_kma->relative_humidity.enable = p_kma_avg->relative_humidity.enable;
    p_kma->precipitation_fine.enable = p_kma_avg->precipitation_fine.enable;
    p_kma->solar_radiation.enable = p_kma_avg->solar_radiation.enable;
    p_kma->sunshine_duration.enable = p_kma_avg->sunshine_duration.enable;
    p_kma->surface_temperature.enable = p_kma_avg->surface_temperature.enable;
    p_kma->grass_temperature.enable = p_kma_avg->grass_temperature.enable;
    p_kma->soil_temperature_5cm.enable = p_kma_avg->soil_temperature_5cm.enable;
    p_kma->soil_temperature_10cm.enable = p_kma_avg->soil_temperature_10cm.enable;
    p_kma->soil_temperature_20cm.enable = p_kma_avg->soil_temperature_20cm.enable;
    p_kma->soil_temperature_30cm.enable = p_kma_avg->soil_temperature_30cm.enable;
    p_kma->soil_temperature_50cm.enable = p_kma_avg->soil_temperature_50cm.enable;
    p_kma->soil_temperature_1m.enable = p_kma_avg->soil_temperature_1m.enable;
    p_kma->soil_temperature_1_5m.enable = p_kma_avg->soil_temperature_1_5m.enable;
    p_kma->soil_temperature_3m.enable = p_kma_avg->soil_temperature_3m.enable;
    p_kma->soil_temperature_5m.enable = p_kma_avg->soil_temperature_5m.enable;
    p_kma->cloud_height_1st.enable = p_kma_avg->cloud_height_1st.enable;
    p_kma->cloud_height_2nd.enable = p_kma_avg->cloud_height_2nd.enable;
    p_kma->cloud_height_3rd.enable = p_kma_avg->cloud_height_3rd.enable;
    p_kma->cloud_amount.enable = p_kma_avg->cloud_amount.enable;
    p_kma->visibility.enable = p_kma_avg->visibility.enable;
    p_kma->pm10_concentration.enable = p_kma_avg->pm10_concentration.enable;
    p_kma->pm25_concentration.enable = p_kma_avg->pm25_concentration.enable;
    p_kma->net_radiation.enable = p_kma_avg->net_radiation.enable;
    p_kma->total_radiation.enable = p_kma_avg->total_radiation.enable;
    p_kma->reflected_radiation.enable = p_kma_avg->reflected_radiation.enable;
    p_kma->direct_radiation.enable = p_kma_avg->direct_radiation.enable;
    p_kma->current_weather.enable = p_kma_avg->current_weather.enable;
    p_kma->temp0_0.enable = p_kma_avg->temp0_0.enable;
    p_kma->temp0_1.enable = p_kma_avg->temp0_1.enable;
    p_kma->temp0_2.enable = p_kma_avg->temp0_2.enable;
    p_kma->temp0_3.enable = p_kma_avg->temp0_3.enable;
    p_kma->soil_moisture_10cm.enable = p_kma_avg->soil_moisture_10cm.enable;
    p_kma->soil_moisture_20cm.enable = p_kma_avg->soil_moisture_20cm.enable;
    p_kma->soil_moisture_30cm.enable = p_kma_avg->soil_moisture_30cm.enable;
    p_kma->soil_moisture_50cm.enable = p_kma_avg->soil_moisture_50cm.enable;
    p_kma->illuminance.enable = p_kma_avg->illuminance.enable;
    p_kma->wind_speed_1_5m.enable = p_kma_avg->wind_speed_1_5m.enable;
    p_kma->wind_speed_4m.enable = p_kma_avg->wind_speed_4m.enable;
    p_kma->instant_wind_speed_1_5m.enable = p_kma_avg->instant_wind_speed_1_5m.enable;
    p_kma->instant_wind_speed_4m.enable = p_kma_avg->instant_wind_speed_4m.enable;
    p_kma->temperature_0_5m.enable = p_kma_avg->temperature_0_5m.enable;
    p_kma->temperature_4m.enable = p_kma_avg->temperature_4m.enable;
    p_kma->humidity_0_5m.enable = p_kma_avg->humidity_0_5m.enable;
    p_kma->humidity_4m.enable = p_kma_avg->humidity_4m.enable;
    p_kma->temp1_0.enable = p_kma_avg->temp1_0.enable;
    p_kma->temp1_1.enable = p_kma_avg->temp1_1.enable;
    p_kma->temp1_2.enable = p_kma_avg->temp1_2.enable;
    p_kma->temp1_3.enable = p_kma_avg->temp1_3.enable;
    p_kma->temp1_4.enable = p_kma_avg->temp1_4.enable;
    p_kma->temp1_5.enable = p_kma_avg->temp1_5.enable;
    p_kma->temp1_6.enable = p_kma_avg->temp1_6.enable;
    p_kma->temp1_7.enable = p_kma_avg->temp1_7.enable;
    p_kma->temp1_8.enable = p_kma_avg->temp1_8.enable;
    p_kma->tacometer.enable = p_kma_avg->tacometer.enable;
    }
      // 1분 과거 자료
  uint16_t kma_cmd_handler_AQ(uint8_t *rx_frame, uint8_t *tx_frame)
  {

    uint8_t data[200];
    uint8_t data_format_no;
    uint16_t len = 0;
    uint16_t station_id;
    kma2_response_t kma2_response;
    kma_data_ex_t *p_kma3=NULL;
    DATE_TIME_BUF mOldDate;
    DATE_TIME_BUF *pDate;
    time_t cur_t,  poll_t;
    AWS_DATA_STRUCT *p_aws = NULL;
  int32_t ret;

    p_kma3 = pvPortMalloc(sizeof(kma_data_ex_t));

    if(p_kma3 == NULL)
    {
      return 0;
    }
    p_aws = pvPortMalloc(sizeof(AWS_DATA_STRUCT));

    if(p_aws==NULL)
    {
      vPortFree(p_kma3);
      return 0;
    }

    station_id = GetWord((uint8_t *)&rx_frame[13]);

    mOldDate.Year = rx_frame[5] + 2000;
    mOldDate.Month = rx_frame[6];
    mOldDate.Day = rx_frame[7];
    mOldDate.Hour = rx_frame[8];
    mOldDate.Min = rx_frame[9];
    mOldDate.Sec = rx_frame[10];

    pDate = &mOldDate;

    tx_frame[5] = rx_frame[5];
    tx_frame[6] = rx_frame[6];
    tx_frame[7] = rx_frame[7];
    tx_frame[8] = rx_frame[8];
    tx_frame[9] = rx_frame[9];

    poll_t = SetTime(pDate->Year, pDate->Month, pDate->Day, pDate->Hour, pDate->Min, 0);

    cur_t =
        SetTime(Date_Time.Year, Date_Time.Month, Date_Time.Day, Date_Time.Hour, Date_Time.Min, 0);
    if ((poll_t == cur_t) && (Date_Time.Sec < 2))
    {
      vPortFree(p_kma3);
      vPortFree(p_aws);
      return 0;  //
    }

    ret = read_data_month(pDate, p_aws, sizeof(AWS_DATA_STRUCT), LOGGING_AWS, 1);
    
    if(ret)
    {
      vPortFree(p_kma3);
      vPortFree(p_aws);
      return 0;
    }
    update_old_to_kma3(p_aws, p_kma3);
    update_sensor_enable(p_kma3);

    switch (get_config_app()->aws_protocol_type)
    {
      case eAWS_PROTOCOL_KMA2:

        data_format_no = calculate_kma2_data_format_no(p_kma3);
        cvt_kma3_to_kma2(p_kma3, &kma2_response);

        switch (data_format_no)
        {
          case KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE:
            len = make_kma2_essential_selective(data, &kma2_response);
            break;
          case KMA2_DATA_FORMAT_ESSENTIAL:
            len = make_kma2_essential(data, &kma2_response);
            break;
          case KMA2_DATA_FORMAT_PRECIPITATION:
            len = make_kma2_precipitation(data, &kma2_response);
            break;
        }
        len = make_kma2_response(rx_frame, data, len, data_format_no, tx_frame);
        break;
      case eAWS_PROTOCOL_KMA3:
        len = make_kma3_data_unusedSesor(data, sizeof(data), p_kma3);
        len = make_kma3_resp(tx_frame, 'Q', DATA_TYPE_GENERAL, station_id, data, len);
        break;

    }
    vPortFree(p_aws);
    vPortFree(p_kma3);
    return len;
  }

// 시간 설정
uint16_t kma_cmd_handler_AT(uint8_t *frame, uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  kma3_command_request_t *req = (kma3_command_request_t *)frame;
  DATE_TIME_BUF nt;
  uint16_t station_id ;

  station_id = GetWord((uint8_t *)&frame[13]);

  nt.Year = req->date_yy + 2000;
  nt.Month = req->date_mm;
  nt.Day = req->date_dd;
  nt.Hour = req->time_hh;
  nt.Min = req->time_mm;
  nt.Sec = req->time_ss;

  bsp_rtc_set(&nt);

  len = make_kma3_resp_RODTWC(packet, sizeof(packet), station_id, req->command_str[1], "OKAY");

  memcpy(send, packet, len);

  return len;
}

// 로거 버전 응답
uint32_t kma_cmd_handler_AV(uint8_t *packet, uint8_t *txBuff)
{
  uint8_t data[30];
  uint16_t cnt = 0;
  uint8_t version[3];

  switch (config.aws_protocol_type)
  {
    case eAWS_PROTOCOL_KMA3:  // KMA3 153바이트형
      version[0] = KMA3_PROTOCOL_YEAR % 100;
      version[1] = KMA3_PROTOCOL_MONTH;
      version[2] = KMA3_PROTOCOL_DAY;
      break;
    default:
      version[0] = KMA3_PROTOCOL_YEAR % 100;
      version[1] = KMA3_PROTOCOL_MONTH;
      version[2] = KMA3_PROTOCOL_DAY;
      break;
  }

  memset(data, 0, sizeof(data));

  SetWord(&data[cnt], 0xFAFB);
  cnt += 2;

  data[cnt++] = version[0];  // Ⅱ 프로토콜 버전 년
  data[cnt++] = version[0];  // Ⅱ 프로토콜 버전 월
  data[cnt++] = version[0];  // Ⅱ 프로토콜 버전 월

  SetWord(&data[cnt], config.id);
  cnt += 2;

  memcpy(&data[cnt], "1.0.0             ", 18);
  cnt += 18;

  SetWord(&data[cnt], crc16_ccitt_table(&data[2], cnt - 2));
  cnt += 2;

  SetWord(&data[cnt], 0xFFFE);
  cnt += 2;

  memcpy(txBuff, data, cnt);

  return cnt;
}

// 리셋
uint16_t kma_cmd_handler_AR(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t packet[50];
  uint16_t len;
  uint16_t station_id;


  station_id = GetWord((uint8_t *)&rx_frame[13]);

  len = make_kma3_resp_RODTWC(packet, sizeof(packet), station_id, 'R', "OKAY");

  memcpy(tx_frame, packet, len);

  return len;
}

uint16_t kma_cmd_handler_AO(uint8_t *rx_frame, uint8_t *send)
{
  uint8_t packet[50];
  uint16_t len;
  uint16_t station_id;

  station_id = GetWord((uint8_t *)&rx_frame[13]);

  len = make_kma3_resp_RODTWC(packet, sizeof(packet), station_id ,'O', "OKAY");

  memcpy(send, packet, len);

  return len;
}

// 암호 설정
uint16_t kma_cmd_handler_AW(uint8_t *rx_frame, uint8_t *tx_frame)
{

  uint8_t packet[50];
  uint16_t len;
  uint16_t password;
  uint16_t station_id;

  station_id = GetWord((uint8_t *)&rx_frame[13]);
  password = GetWord((uint8_t *)&packet[11]);

  set_config_app_password(password);

  len = make_kma3_resp_RODTWC(packet, sizeof(packet), station_id, 'W', "OKAY");

  memcpy(tx_frame, packet, len);

  return len;
}

// 데이터 삭제
uint16_t kma_cmd_handler_AC(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t packet[50];
  uint16_t len;
  uint16_t station_id;

  station_id = GetWord((uint8_t *)&rx_frame[13]);

  len = make_kma3_resp_RODTWC(packet, sizeof(packet), station_id, 'C', "OKAY");

  memcpy(tx_frame, packet, len);

  return len;
}

//기존에 없는데 화진에서 응용해서 추가한듯함
uint16_t kma_cmd_handler_AP(uint8_t *rx_frame, uint8_t *tx_frame)
{
  uint8_t ip[4];
  uint8_t packet[50];
  uint16_t len;
  uint32_t i;
  uint16_t port;
  uint16_t station_id;

  station_id = GetWord((uint8_t *)&rx_frame[13]);

  for (i = 0; i < 4; i++)
  {
    ip[i] = GetWord((uint8_t *)&rx_frame[15+3+i]);
  }

  port = GetWord((uint8_t *)&rx_frame[15+7+i]);

  set_config_app_cdma_ip(ip);
  set_config_app_cdma_port(port);

  len = make_kma3_resp_RODTWC(packet, sizeof(packet), station_id, 'P', "OKAY");

  memcpy(tx_frame, packet, len);

  return len;
}



/**
 * @retval 전송 길이
 */
int32_t kma_cmd_handler(uint8_t *rx_frame, uint32_t frame_len, uint8_t *tx_buffer, eREQ_SOURCE_t source)
{
  bool protocol_ok = false;
  int32_t len = 0;
  eKMA_COMMAND_TYPE_t cmd_type;
  kma3_command_request_t request;

  switch (get_config_app()->aws_protocol_type)
  {
    case eAWS_PROTOCOL_KMA2:
      protocol_ok = is_kma2_protocol(rx_frame, frame_len);
      break;
    case eAWS_PROTOCOL_KMA3:
      protocol_ok = is_kma3_protocol(rx_frame, frame_len);
       break;
  }

  if (protocol_ok == false)
  {
    task_hex_dump("AWS frame",rx_frame,frame_len);
    return divas_cmd_handler(rx_frame, frame_len,tx_buffer);
  }

  switch (get_config_app()->aws_protocol_type)
  {
    case eAWS_PROTOCOL_KMA2:
      print_kma2_command_request((kma2_command_request_t*)rx_frame);
      break;
    case eAWS_PROTOCOL_KMA3:
      print_kma3_command_request((kma3_command_request_t*)rx_frame);
      break;
  }



    kma_unpack(rx_frame, &request);

  if (request.command_str[1] == 'D')  // 지점번호 설정,AWS(구)에서 이렇게 처리함
  {
    if (request.password == config.id)
    {
      config.id = request.station_id;
    }
  }

  if (request.station_id != config.id)
  {
    return 0;
  }

  // 반드시 0으로 초기화
  memset(tx_buffer, 0, KMA_TX_BUFFER_SIZE);

  cmd_type = kma_get_command_type(request.command_str);

  switch (cmd_type)
  {
    case eKMA_COMMAND_TYPE_AI:  // 순간자료
      len = kma_cmd_handler_AI((uint8_t *)rx_frame, tx_buffer);
      break;
    case eKMA_COMMAND_TYPE_AB:                          // 1분자료(최근 1분 자료 요구)
      len = kma_cmd_handler_AB((uint8_t *)rx_frame, tx_buffer);  // 동일한 함수로 처리
      break;
    case eKMA_COMMAND_TYPE_AQ:  // 1분 과거 자료(C한에 과거 시간을 입력하여 과거자료 요구)
      len = kma_cmd_handler_AQ((uint8_t *)rx_frame, tx_buffer);  // 동일한 함수로 처리
      break;
    case eKMA_COMMAND_TYPE_AV:  // 데이터로거 버전
      len = kma_cmd_handler_AV((uint8_t *)rx_frame, tx_buffer);
      break;
    case eKMA_COMMAND_TYPE_AR:  // 데이터로거 리셋
      len = kma_cmd_handler_AR((uint8_t *)rx_frame, tx_buffer);
      osDelay(10);
      break;
    case eKMA_COMMAND_TYPE_AO:  // 전원리셋 또는 모뎀 리셋 또는 적설센서 리셋
      len = kma_cmd_handler_AO((uint8_t *)rx_frame, tx_buffer);
      break;
    case eKMA_COMMAND_TYPE_AD:  // 지점번호 설정
      break;
    case eKMA_COMMAND_TYPE_AT:  // 날짜,시간 설정
      len = kma_cmd_handler_AT((uint8_t *)rx_frame, tx_buffer);
      break;
    case eKMA_COMMAND_TYPE_AW:  // 암호 설정
      len = kma_cmd_handler_AW((uint8_t *)rx_frame, tx_buffer);
      break;
    case eKMA_COMMAND_TYPE_AC:  // 저장데이터 삭제
      len = kma_cmd_handler_AC((uint8_t *)rx_frame, tx_buffer);
      break;
    case eKMA_COMMAND_TYPE_AP:  // 국립공원 Protocol전용 : 원격 CDMA 원격 TCP  IP & PORT 변경
      len = kma_cmd_handler_AP((uint8_t *)rx_frame, tx_buffer);
      break;
    default:
      len = 0;
      break;
  }

  switch (get_config_app()->aws_protocol_type)
  {
  case eAWS_PROTOCOL_KMA2:
    parse_kma2_response(tx_buffer,len);
     break;
  case eAWS_PROTOCOL_KMA3:
    parse_kma3_response(tx_buffer, len);
    break;
  default:
    break;
  }


  return len;
}
