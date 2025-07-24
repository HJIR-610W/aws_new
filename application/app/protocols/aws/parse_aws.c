#include "parse_aws.h"

#include <stdio.h>
#include <string.h>

#include "dev_io.h"
#include "util_memory.h"
#include "util_time.h"

#define KMA2_PRINT_LABEL_WIDTH 38  // 콜론 앞까지의 레이블이 차지할 최대 너비 (조정 가능)

// 비트 체크를 위한 매크로
#define IS_BIT_SET(value, bit_pos) (((value) >> (bit_pos)) & 0x01)

// KMA2 프로토콜 상수 정의 (이전과 동일)
#define KMA2_HEADER_START 0xFAFB
#define KMA2_HEADER_END 0xFFFE
// ... (기타 KMA2 상수 정의는 이전 답변 내용과 동일하게 유지) ...
#define KMA2_CMD_REQUEST_LEN 29
#define KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE 0
#define KMA2_DATA_FORMAT_ESSENTIAL 1
#define KMA2_DATA_FORMAT_PRECIPITATION 2
#define KMA2_DATA_CONTENT_ESSENTIAL_SELECTIVE_LEN 91
#define KMA2_DATA_CONTENT_ESSENTIAL_LEN 45
#define KMA2_DATA_CONTENT_PRECIPITATION_LEN 16
#define KMA2_OBS_PACKET_BASE_LEN (2 + 3 + 5 + 1 + 1 + 2)
#define KMA2_OBS_PACKET_FOOTER_LEN (2 + 2)
#define KMA2_OBS_PACKET_TOTAL_LEN_ESSENTIAL_SELECTIVE                     \
  (KMA2_OBS_PACKET_BASE_LEN + KMA2_DATA_CONTENT_ESSENTIAL_SELECTIVE_LEN + \
   KMA2_OBS_PACKET_FOOTER_LEN)
#define KMA2_OBS_PACKET_TOTAL_LEN_ESSENTIAL \
  (KMA2_OBS_PACKET_BASE_LEN + KMA2_DATA_CONTENT_ESSENTIAL_LEN + KMA2_OBS_PACKET_FOOTER_LEN)
#define KMA2_OBS_PACKET_TOTAL_LEN_PRECIPITATION \
  (KMA2_OBS_PACKET_BASE_LEN + KMA2_DATA_CONTENT_PRECIPITATION_LEN + KMA2_OBS_PACKET_FOOTER_LEN)
#define KMA2_PROTOCOL_VERSION_YY 14
#define KMA2_PROTOCOL_VERSION_MM 9
#define KMA2_PROTOCOL_VERSION_DD 25


uint16_t swap_bytes_uint16(uint16_t val)
{ 
  return (val << 8) | (val >> 8); 
}
static inline uint16_t kma2_get_u16_big_endian(const uint8_t* buf)
{
  return ((uint16_t)buf[0] << 8) | buf[1];
}

void print_kma2_command_request(const kma2_command_request_t* req)
{
  task_printf("KMA2 Command Request(%04d-%02d-%02d %02d:%02d:%02d)\r\n", Date_Time.Year,
              Date_Time.Month, Date_Time.Min, Date_Time.Hour, Date_Time.Min, Date_Time.Sec);
  task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "Header",
              swap_bytes_uint16(req->header_start));
  task_printf("%-*s : %02d-%02d-%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "P.Ver", req->protocol_yy,
              req->protocol_mm, req->protocol_dd);
  task_printf("%-*s : %02d-%02d-%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "Date", req->date_yy,
              req->date_mm, req->date_dd);
  task_printf("%-*s : %02d:%02d:%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "Time", req->time_hh,
              req->time_mm, req->time_ss);
  task_printf("%-*s : %u\r\n", KMA2_PRINT_LABEL_WIDTH, "Pass", swap_bytes_uint16(req->password));
  task_printf("%-*s : %u\r\n", KMA2_PRINT_LABEL_WIDTH, "ID", swap_bytes_uint16(req->station_id));
  char cmd_print_buf[11];  // 10 chars + null terminator
  memcpy(cmd_print_buf, req->command_str, 10);
  cmd_print_buf[10] = '\0';
  task_printf("%-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH, "Cmd", cmd_print_buf);
  task_printf("%-*s : 0x%02X\r\n", KMA2_PRINT_LABEL_WIDTH, "XOR", req->checksum_xor);
  task_printf("%-*s : 0x%02X\r\n", KMA2_PRINT_LABEL_WIDTH, "SUM", req->checksum_sum);
  task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "End",
              swap_bytes_uint16(req->header_end));
  task_printf("----------------------------\r\n");
}
void print_kma3_command_request(const kma3_command_request_t* req)
{
  task_printf("KMA3 Command Request(%04d-%02d-%02d %02d:%02d:%02d)\r\n",Date_Time.Year,Date_Time.Month,Date_Time.Min,
  Date_Time.Hour,Date_Time.Min,Date_Time.Sec);

  task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "Header",
              swap_bytes_uint16(req->header_start));
  task_printf("%-*s : %02d-%02d-%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "P.Ver", req->protocol_yy,
              req->protocol_mm, req->protocol_dd);
  task_printf("%-*s : %02d-%02d-%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "Date", req->date_yy,
              req->date_mm, req->date_dd);
  task_printf("%-*s : %02d:%02d:%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "Time", req->time_hh,
              req->time_mm, req->time_ss);
  task_printf("%-*s : %u\r\n", KMA2_PRINT_LABEL_WIDTH, "Pass", swap_bytes_uint16(req->password));
  task_printf("%-*s : %u\r\n", KMA2_PRINT_LABEL_WIDTH, "ID", swap_bytes_uint16(req->station_id));
  char cmd_print_buf[11];  // 10 chars + null terminator
  memcpy(cmd_print_buf, req->command_str, 10);
  cmd_print_buf[10] = '\0';
  task_printf("%-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH, "Cmd", cmd_print_buf);
  task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "CRC", req->crc);
  task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "End",
              swap_bytes_uint16(req->header_end));
  task_printf("----------------------------\r\n");
}

// is_kma2_observation_packet_valid 함수 (이전과 동일)
bool is_kma2_observation_packet_valid(const uint8_t* buffer, size_t len,
                                      uint8_t* out_data_format_no, uint16_t* out_data_content_len)
{
  uint16_t data;
  if (len != KMA2_OBS_PACKET_TOTAL_LEN_ESSENTIAL_SELECTIVE &&
      len != KMA2_OBS_PACKET_TOTAL_LEN_ESSENTIAL && len != KMA2_OBS_PACKET_TOTAL_LEN_PRECIPITATION)
  {
    return false;
  }
  const kma2_observation_packet_header_t* header =
      (const kma2_observation_packet_header_t*)(buffer);
  if (swap_bytes_uint16(header->start_mark) != KMA2_HEADER_START)
  {
    return false;
  }
  uint16_t current_data_content_len = 0;
  if (header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE)
  {
    current_data_content_len = KMA2_DATA_CONTENT_ESSENTIAL_SELECTIVE_LEN;
  }
  else if (header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL)
  {
    current_data_content_len = KMA2_DATA_CONTENT_ESSENTIAL_LEN;
  }
  else if (header->data_format_no == KMA2_DATA_FORMAT_PRECIPITATION)
  {
    current_data_content_len = KMA2_DATA_CONTENT_PRECIPITATION_LEN;
  }
  else
  {
    return false;
  }
  if (KMA2_OBS_PACKET_BASE_LEN + current_data_content_len + KMA2_OBS_PACKET_FOOTER_LEN != len)
  {
    return false;
  }
  if (out_data_format_no)
    *out_data_format_no = header->data_format_no;
  if (out_data_content_len)
    *out_data_content_len = current_data_content_len;
  const kma2_observation_packet_footer_t* footer =
      (const kma2_observation_packet_footer_t*)(buffer + KMA2_OBS_PACKET_BASE_LEN +
                                                current_data_content_len);

      data = footer->end_mark;
      if (kma2_get_u16_big_endian((const uint8_t*)&data) != KMA2_HEADER_END)
      {
        return false;
      }
  const uint8_t* checksum_data_start = buffer + 2;
  size_t checksum_data_len = (KMA2_OBS_PACKET_BASE_LEN - 2) + current_data_content_len;
  uint8_t calc_xor = calculate_xor_checksum(checksum_data_start, checksum_data_len);
  uint8_t calc_sum = make_sum((uint8_t*)checksum_data_start, checksum_data_len);
  if (calc_xor != footer->checksum_xor || calc_sum != footer->checksum_sum)
  {
    return false;
  }
  return true;
}

// parse_kma2_data_content 함수 (이전과 동일)
bool parse_kma2_data_content(const uint8_t* content_buffer, uint8_t data_format_no,
                             uint16_t content_len, kma2_observation_fields_t* fields)
{
  memset(fields, 0, sizeof(kma2_observation_fields_t));
  size_t offset = 0;

  if (data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE ||
      data_format_no == KMA2_DATA_FORMAT_ESSENTIAL)
  {
    if (offset + 22 > content_len)
      return false;
    fields->temperature = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->valid_A = true;
    fields->wind_direction_avg = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_B = true;
    fields->wind_speed_avg = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_C = true;
    fields->gust_wind_direction = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_D = true;
    fields->gust_wind_speed = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_E = true;
    fields->precipitation_0_5mm = (float)kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_F = true;
    fields->pressure = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_G = true;
    fields->precipitation_presence = kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_H = true;
    fields->snowfall_accum = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_I = true;
    fields->relative_humidity = kma2_get_u16_big_endian(content_buffer + offset) / 10.0f;
    offset += 2;
    fields->valid_J = true;
    fields->precipitation_0_1mm = (float)kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_K = true;
  }

  if (data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE ||
      data_format_no == KMA2_DATA_FORMAT_ESSENTIAL)
  {
    if (offset + 20 > content_len && data_format_no != KMA2_DATA_FORMAT_PRECIPITATION)
      return false;
    for (int i = 0; i < 10; ++i)
    {
      if (offset + 2 > content_len && data_format_no != KMA2_DATA_FORMAT_PRECIPITATION && i >= 5)
        break; /* Precipitation L1-L5 까지만 */
      fields->raw_L[i] = kma2_get_u16_big_endian(content_buffer + offset);
      offset += 2;
      fields->valid_L[i] = true;
    }
  }
  else if (data_format_no == KMA2_DATA_FORMAT_PRECIPITATION)
  {
    offset = 0;
    fields->precipitation_0_5mm = (float)kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_F = true;
    fields->precipitation_0_1mm = (float)kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_K = true;
    if (offset + 10 > content_len)
      return false;  // L1-L5 for precipitation
    for (int i = 0; i < 5; ++i)
    {
      fields->raw_L[i] = kma2_get_u16_big_endian(content_buffer + offset);
      offset += 2;
      fields->valid_L[i] = true;
    }
  }

  if (data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE)
  {
    if (offset + 26 + 20 > content_len)
      return false;
    fields->solar_radiation_mj = kma2_get_u16_big_endian(content_buffer + offset) / 100.0f;
    offset += 2;
    fields->valid_a = true;
    fields->sunshine_duration_sec = kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_b = true;
    fields->surface_temperature = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->valid_c = true;
    fields->grass_temperature = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->valid_d = true;
    fields->valid_e_m = true;
    fields->soil_temp_5cm = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_10cm = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_20cm = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_30cm = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_50cm = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_1m = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_1_5m = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_3m = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    fields->soil_temp_5m = (kma2_get_u16_big_endian(content_buffer + offset) - 1000) / 10.0f;
    offset += 2;
    for (int i = 0; i < 10; ++i)
    {
      fields->raw_S[i] = kma2_get_u16_big_endian(content_buffer + offset);
      offset += 2;
      fields->valid_S[i] = true;
    }
  }

  if (data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE ||
      data_format_no == KMA2_DATA_FORMAT_ESSENTIAL)
  {
    if (offset + 1 > content_len)
      return false;
    fields->status_X = content_buffer[offset++];
    fields->valid_X = true;
    if (offset + 2 > content_len)
      return false;
    fields->status_Y = kma2_get_u16_big_endian(content_buffer + offset);
    offset += 2;
    fields->valid_Y = true;
  }
  else if (data_format_no == KMA2_DATA_FORMAT_PRECIPITATION)
  {
    if (offset + 1 > content_len)
      return false;
    fields->status_X = content_buffer[offset++];
    fields->valid_X = true;
    if (offset + 1 > content_len)
      return false;
    fields->status_Z = content_buffer[offset++];
    fields->valid_Z = true;
  }

  if (offset != content_len)
  { /* Mismatch possible if parsing logic is not perfectly aligned with content_len for each format
     */
  }
  return true;
}

// --- KMA2 데이터 출력 함수 (상태 X, Y, Z 상세 출력 추가) ---
void print_kma2_observation_data(const kma2_observation_packet_header_t* header,
                                 const kma2_observation_fields_t* fields)
{
#ifdef _WIN32
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
  WORD saved_attributes;

  // 현재 콘솔 속성 저장
  GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
  saved_attributes = consoleInfo.wAttributes;
#endif

  task_printf("--- KMA2 Observation Data ---\r\n");
  // %-*s : 너비만큼 문자열 출력, 왼쪽 정렬, 부족하면 공백 채움
  task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "Header Mark",
              swap_bytes_uint16(header->start_mark));
  task_printf("%-*s : 20%02d-%02d-%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "Protocol Ver",
              header->protocol_ver_yy, header->protocol_ver_mm, header->protocol_ver_dd);
  task_printf("%-*s : 20%02d-%02d-%02d %02d:%02d\r\n", KMA2_PRINT_LABEL_WIDTH, "Timestamp",
              header->date_yy, header->date_mm, header->date_dd, header->time_hh, header->time_mm);
  task_printf("%-*s : %c\r\n", KMA2_PRINT_LABEL_WIDTH, "Data Type Char", header->data_type_char);

  const char* format_str = "Unknown";
  if (header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE)
    format_str = "Essential+Selective (0)";
  else if (header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL)
    format_str = "Essential (1)";
  else if (header->data_format_no == KMA2_DATA_FORMAT_PRECIPITATION)
    format_str = "Precipitation (2)";
  task_printf("%-*s : %u (%s)\r\n", KMA2_PRINT_LABEL_WIDTH, "Data Format No",
              header->data_format_no, format_str);
  task_printf("%-*s : %u\r\n", KMA2_PRINT_LABEL_WIDTH, "Station ID",
              swap_bytes_uint16(header->station_id));
  task_printf("--- Data Content (VII) ---\r\n");

  if (fields->valid_A)
    task_printf("  %-*s : %.1f C\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "A. Temperature",
                fields->temperature);
  if (fields->valid_B)
    task_printf("  %-*s : %.1f deg\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "B. Wind Dir Avg",
                fields->wind_direction_avg);
  if (fields->valid_C)
    task_printf("  %-*s : %.1f m/s\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "C. Wind Spd Avg",
                fields->wind_speed_avg);
  if (fields->valid_D)
    task_printf("  %-*s : %.1f deg\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "D. Gust Wind Dir",
                fields->gust_wind_direction);
  if (fields->valid_E)
    task_printf("  %-*s : %.1f m/s\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "E. Gust Wind Spd",
                fields->gust_wind_speed);
  if (fields->valid_F)
    task_printf("  %-*s : %.0f (0.5/1mm unit)\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "F. Precip (Raw)",
                fields->precipitation_0_5mm);
  if (fields->valid_G)
    task_printf("  %-*s : %.1f hPa\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "G. Pressure",
                fields->pressure);
  if (fields->valid_H)
    task_printf("  %-*s : 0x%04X (%s)\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "H. Precip Presence",
                fields->precipitation_presence,
                fields->precipitation_presence == 0x0010 ? "Yes" : "No");
  if (fields->valid_I)
    task_printf("  %-*s : %.1f cm\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "I. Snowfall Accum",
                fields->snowfall_accum);
  if (fields->valid_J)
    task_printf("  %-*s : %.1f %%\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "J. Rel. Humidity",
                fields->relative_humidity);
  if (fields->valid_K)
    task_printf("  %-*s : %.0f (0.1mm unit)\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "K. Precip (Raw)",
                fields->precipitation_0_1mm);

  char label_buf[128];
  if (header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE ||
      header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL)
  {
    for (int i = 0; i < 10; ++i)
      if (fields->valid_L[i])
      {
        sprintf(label_buf, "L%d. Reserve L%d", i + 1, i + 1);
        task_printf("  %-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH - 2, label_buf, fields->raw_L[i]);
      }
  }
  else if (header->data_format_no == KMA2_DATA_FORMAT_PRECIPITATION)
  {
    for (int i = 0; i < 5; ++i)
      if (fields->valid_L[i])
      {
        sprintf(label_buf, "L%d. Reserve L%d", i + 1, i + 1);
        task_printf("  %-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH - 2, label_buf, fields->raw_L[i]);
      }
  }

  if (header->data_format_no == KMA2_DATA_FORMAT_ESSENTIAL_SELECTIVE)
  {
    if (fields->valid_a)
      task_printf("  %-*s : %.2f MJ/m^2\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "a. Solar Rad.",
                  fields->solar_radiation_mj);
    if (fields->valid_b)
      task_printf("  %-*s : %u sec\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "b. Sunshine Dur",
                  fields->sunshine_duration_sec);
    if (fields->valid_c)
      task_printf("  %-*s : %.1f C\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "c. Surface Temp",
                  fields->surface_temperature);
    if (fields->valid_d)
      task_printf("  %-*s : %.1f C\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "d. Grass Temp",
                  fields->grass_temperature);
    if (fields->valid_e_m)
    {
      task_printf("  %-*s : %.1f C\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "e. Soil Temp 5cm",
                  fields->soil_temp_5cm);
      task_printf("  %-*s : %.1f C\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "f. Soil Temp 10cm",
                  fields->soil_temp_10cm);
      // ... (기타 지중온도 출력) ...
      task_printf("  %-*s : %.1f C\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "m. Soil Temp 5m",
                  fields->soil_temp_5m);
    }
    for (int i = 0; i < 10; ++i)
      if (fields->valid_S[i])
      {
        sprintf(label_buf, "S%d. Reserve S%d", i + 1, i + 1);
        task_printf("  %-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH - 2, label_buf, fields->raw_S[i]);
      }
  }

#if 0

  if (fields->valid_X) {
    printf("  %-*s : 0x%02X\n", KMA2_PRINT_LABEL_WIDTH - 2, "X. Datalogger Voltage Status", fields->status_X);
    // BIT 0: DC입력전압
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 0 (DC Input Volt)", IS_BIT_SET(fields->status_X, 0) ? "Abnormal" : "Normal");
    // BIT 1: 배터리 전압
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 1 (Battery Volt)", IS_BIT_SET(fields->status_X, 1) ? "Abnormal" : "Normal");
    // BIT 2, 3: AC 전압
    uint8_t ac_status = (fields->status_X >> 2) & 0x03; // 비트 2와 3 추출
    const char* ac_str = "Unknown";
    if (ac_status == 0x00) ac_str = "110V";
    else if (ac_status == 0x01) ac_str = "220V";
    else if (ac_status == 0x03) ac_str = "AC OFF"; // 문서상 11 (이진수 3)
    else ac_str = "Reserved/Unknown"; // 0x02 (이진수 2)는 정의되지 않음
    printf("    %-*s : %s (0x%02X)\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 2-3 (AC Volt)", ac_str, ac_status);
    // BIT 4: 데이터로거함 잠금상태
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 4 (Logger Door)", IS_BIT_SET(fields->status_X, 4) ? "Open" : "Closed");
    // BIT 5: 예비 1
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 5 (Reserve 1)", IS_BIT_SET(fields->status_X, 5) ? "Abnormal" : "Normal");
    // BIT 6: 예비 2
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 6 (Reserve 2)", IS_BIT_SET(fields->status_X, 6) ? "Abnormal" : "Normal");
    // BIT 7: 예비 3
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 7 (Reserve 3)", IS_BIT_SET(fields->status_X, 7) ? "Abnormal" : "Normal");
  }

  if (fields->valid_Y) {
    printf("  %-*s : 0x%04X\n", KMA2_PRINT_LABEL_WIDTH - 2, "Y. Logger Sensor Status", fields->status_Y);
    const char* sensor_names_Y[] = {
        "Wind Dir Sensor", "Wind Spd Sensor", "Temp Sensor", "Precip Pres. Sensor",
        "Rainfall Sensor", "Humidity Sensor", "Pressure Sensor", "Reserve Y1",
        "Reserve Y2",      "Reserve Y3",      "Reserve Y4",      "Reserve Y5",
        "Reserve Y6",      "Reserve Y7",      "Reserve Y8",      "FAN Operation"
    };
    for (int i = 0; i < 16; ++i) {
      sprintf(label_buf, "BIT %d (%s)", i, sensor_names_Y[i]);
      printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, label_buf, IS_BIT_SET(fields->status_Y, i) ? "Abnormal" : "Normal");
    }
  }

  if (fields->valid_Z && header->data_format_no == KMA2_DATA_FORMAT_PRECIPITATION) { // Z는 강수량관측(2) 형식일 때 의미 있음
    printf("  %-*s : 0x%02X\n", KMA2_PRINT_LABEL_WIDTH - 2, "Z. Logger Sensor Status (Precip)", fields->status_Z);
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 0 (Rainfall Sensor)", IS_BIT_SET(fields->status_Z, 0) ? "Abnormal" : "Normal");
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 1 (Reserve Z1)", IS_BIT_SET(fields->status_Z, 1) ? "Abnormal" : "Normal");
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 2 (Reserve Z2)", IS_BIT_SET(fields->status_Z, 2) ? "Abnormal" : "Normal");
    printf("    %-*s : %s\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 3 (Reserve Z3)", IS_BIT_SET(fields->status_Z, 3) ? "Abnormal" : "Normal");
  }

#else
  if (fields->valid_X)
  {
    task_printf("  %-*s : 0x%02X\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "X. Datalogger Voltage Status",
                fields->status_X);

    bool is_abnormal;
    // BIT 0: DC입력전압
    is_abnormal = IS_BIT_SET(fields->status_X, 0);
#ifdef _WIN32
    if (is_abnormal)
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
    task_printf("    %-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 0 (DC Input Volt)",
                is_abnormal ? "Abnormal" : "Normal");
#ifdef _WIN32
    if (is_abnormal)
      SetConsoleTextAttribute(hConsole, saved_attributes);
#endif

    // BIT 1: 배터리 전압
    is_abnormal = IS_BIT_SET(fields->status_X, 1);
#ifdef _WIN32
    if (is_abnormal)
      SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
    task_printf("    %-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 1 (Battery Volt)",
                is_abnormal ? "Abnormal" : "Normal");
#ifdef _WIN32
    if (is_abnormal)
      SetConsoleTextAttribute(hConsole, saved_attributes);
#endif

    // BIT 2, 3: AC 전압
    uint8_t ac_status_val = (fields->status_X >> 2) & 0x03;
    const char* ac_str = "Unknown";

    if (ac_status_val == 0x00)
      ac_str = "110V";
    else if (ac_status_val == 0x01)
      ac_str = "220V";
    else if (ac_status_val == 0x03)
    {
      ac_str = "AC OFF";

    }
    else
      ac_str = "Reserved/Unknown";

    task_printf("    %-*s : %s (0x%02X)\r\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 2-3 (AC Volt)",
                ac_str, ac_status_val);


    // BIT 4: 데이터로거함 잠금상태
    is_abnormal = IS_BIT_SET(fields->status_X, 4);  // 'Open'을 Abnormal로 간주

    task_printf("    %-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH - 4, "BIT 4 (Logger Door)",
                is_abnormal ? "Open" : "Closed");


    // BIT 5, 6, 7: 예비
    for (int i = 5; i <= 7; ++i)
    {
      is_abnormal = IS_BIT_SET(fields->status_X, i);
      sprintf(label_buf, "BIT %d (Reserve %d)", i, i - 4);

      task_printf("    %-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH - 4, label_buf,
                  is_abnormal ? "Abnormal" : "Normal");

    }
  }

  if (fields->valid_Y)
  {
    task_printf("  %-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH - 2, "Y. Logger Sensor Status",
                fields->status_Y);
    const char* sensor_names_Y[] = {
        "Wind Dir Sensor", "Wind Spd Sensor", "Temp Sensor",     "Precip Pres. Sensor",
        "Rainfall Sensor", "Humidity Sensor", "Pressure Sensor", "Reserve Y1",
        "Reserve Y2",      "Reserve Y3",      "Reserve Y4",      "Reserve Y5",
        "Reserve Y6",      "Reserve Y7",      "Reserve Y8",      "FAN Operation"};
    for (int i = 0; i < 16; ++i)
    {
      bool is_abnormal = IS_BIT_SET(fields->status_Y, i);
      sprintf(label_buf, "BIT %d (%s)", i, sensor_names_Y[i]);

      task_printf("    %-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH - 4, label_buf,
                  is_abnormal ? "Abnormal" : "Normal");

    }
  }

  if (fields->valid_Z && header->data_format_no == KMA2_DATA_FORMAT_PRECIPITATION)
  {
    task_printf("  %-*s : 0x%02X\r\n", KMA2_PRINT_LABEL_WIDTH - 2,
                "Z. Logger Sensor Status (Precip)", fields->status_Z);
    const char* sensor_names_Z[] = {"Rainfall Sensor", "Reserve Z1", "Reserve Z2", "Reserve Z3"};
    for (int i = 0; i < 4; ++i)
    {  // Z는 4비트만 유효 (문서상)
      bool is_abnormal = IS_BIT_SET(fields->status_Z, i);
      sprintf(label_buf, "BIT %d (%s)", i, sensor_names_Z[i]);

      task_printf("    %-*s : %s\r\n", KMA2_PRINT_LABEL_WIDTH - 4, label_buf,
                  is_abnormal ? "Abnormal" : "Normal");

    }
  }
#endif
}

void parse_kma2_response(const uint8_t* frame, uint32_t bytes_read)
{
  if (bytes_read > 0)
  {


    uint8_t parsed_data_format_no;
    uint16_t parsed_data_content_len;

    if (is_kma2_observation_packet_valid(frame, bytes_read, &parsed_data_format_no,
                                         &parsed_data_content_len))
    {
      task_printf("KMA2 Observation Packet is valid.c\r\n");
      const kma2_observation_packet_header_t* obs_header =
          (const kma2_observation_packet_header_t*)(frame);
      const uint8_t* data_content_start = frame + KMA2_OBS_PACKET_BASE_LEN;

      kma2_observation_fields_t obs_fields;
      if (parse_kma2_data_content(data_content_start, parsed_data_format_no,
                                  parsed_data_content_len, &obs_fields))
      {
        print_kma2_observation_data(obs_header, &obs_fields);

        const kma2_observation_packet_footer_t* obs_footer =
            (const kma2_observation_packet_footer_t*)(frame + KMA2_OBS_PACKET_BASE_LEN +
                                                      parsed_data_content_len);
        task_printf("--- Footer ---\r\n");
        task_printf("%-*s : 0x%02X\r\n", KMA2_PRINT_LABEL_WIDTH, "Checksum XOR",
                    obs_footer->checksum_xor);
        task_printf("%-*s : 0x%02X\r\n", KMA2_PRINT_LABEL_WIDTH, "Checksum SUM",
                    obs_footer->checksum_sum);
        task_printf("%-*s : 0x%04X\r\n", KMA2_PRINT_LABEL_WIDTH, "End Mark",
                    swap_bytes_uint16(obs_footer->end_mark));
      }
    }
  }
}
#pragma pack(push,1)
typedef struct
{
  uint16_t header;
  uint8_t protocol_year;
  uint8_t protocol_month;
  uint8_t protocol_day;
  uint8_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t dataType;
  uint8_t dataNum;
  uint16_t id;

  // 1. 기온 (1분 평균)
  uint16_t temperature;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500
                         // (관측값 * 10)

  // 2. 풍향 (1분 평균)
  uint16_t wind_direction_avg;  // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 3599
                                // (관측값 * 10)

  // 3. 풍속 (1분 평균)
  uint16_t wind_speed_avg;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                            // (관측값 * 10)

  // 4. 풍향 (1분 순간)
  uint16_t wind_direction_instant;  // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~
                                    // 3599 (관측값 * 10)

  // 5. 풍속 (1분 순간)
  uint16_t wind_speed_instant;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 6. 강수량 (0.5/1.0 mm)
  uint16_t precipitation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

  // 7. 기압 (1분 평균 현지 기압)
  uint16_t pressure;  // 사용비트: 13, 유효범위: 0 ~ 16383 (인치 코드), 표현범위: 5000 ~ 11000

  // 8. 강수 유무
  uint16_t precipitation_presence;  // 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 = 강수
                                    // 없음, 1 = 강수 있음

  // 9. 적설
  uint16_t
      snowfall;  // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 0 ~ 4095 (관측값 * 10)

  // 10. 상대습도 (1분 평균)
  uint16_t relative_humidity;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                               // (관측값 * 10)

  // 11. 강수량 (0.1 mm)
  uint16_t
      precipitation_fine;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767

  // 1. 일사 (누적값)
  uint16_t solar_radiation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767
                             // [관측값(MJ/m²) * 100]

  // 2. 일조 (누적 시간)
  uint16_t sunshine_duration;  // 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 65535
                               // [누적시간(초 단위)]

  // 3. 지면온도 (1분 평균)
  uint16_t surface_temperature;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                 // 2000 [(관측값 + 100) * 10]

  // 4. 초상온도 (1분 평균)
  uint16_t grass_temperature;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 2000
                               // [(관측값 + 100) * 10]

  // 5. 지중온도 (5cm, 1분 평균)
  uint16_t soil_temperature_5cm;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                  // 2000 [(관측값 + 100) * 10]

  // 6. 지중온도 (10cm, 1분 평균)
  uint16_t soil_temperature_10cm;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                   // 2000 [(관측값 + 100) * 10]

  // 7. 지중온도 (20cm, 1분 평균)
  uint16_t soil_temperature_20cm;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                   // 2000 [(관측값 + 100) * 10]

  // 8. 지중온도 (30cm, 1분 평균)
  uint16_t soil_temperature_30cm;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                   // 2000 [(관측값 + 100) * 10]

  // 9. 지중온도 (50cm, 1분 평균)
  uint16_t soil_temperature_50cm;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                   // 2000 [(관측값 + 100) * 10]

  // 10. 지중온도 (1.0m, 1분 평균)
  uint16_t soil_temperature_1m;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                 // 2000 [(관측값 + 100) * 10]

  // 11. 지중온도 (1.5m, 1분 평균)
  uint16_t soil_temperature_1_5m;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                   // 2000 [(관측값 + 100) * 10]

  // 12. 지중온도 (3.0m, 1분 평균)
  uint16_t soil_temperature_3m;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                 // 2000 [(관측값 + 100) * 10]

  // 13. 지중온도 (5.0m, 1분 평균)
  uint16_t soil_temperature_5m;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~
                                 // 2000 [(관측값 + 100) * 10]

  // 1. 1층 운고 (1분 평균)
  uint16_t cloud_height_1st;  // 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000
                              // (관측값[m])

  // 2. 2층 운고 (1분 평균)
  uint16_t cloud_height_2nd;  // 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000
                              // (관측값[m])

  // 3. 3층 운고 (1분 평균)
  uint16_t cloud_height_3rd;  // 사용비트: 12, 유효범위: 0 ~ 8191 (인치 코드), 표현범위: 0 ~ 8000
                              // (관측값[m])

  // 4. 운량
  uint16_t cloud_amount;  // 사용비트: 3, 유효범위: 0 ~ 15 (인치 코드), 표현범위: 0 ~ 10 (관측값)

  // 5. 시정 (1분 평균)
  uint16_t
      visibility;  // 사용비트: 15, 유효범위: 0 ~ 65535 (인치 코드), 표현범위: 0 ~ 50000 (관측값[m])

  // 6. PM10 (분진농도)
  uint16_t pm10_concentration;  // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599
                                // (관측값 [μg/m³] × 10)

  // 7. PM2.5 (분진농도)
  uint16_t pm25_concentration;  // 사용비트: 11, 유효범위: 0 ~ 4095 (인치 코드), 표현범위: 1 ~ 3599
                                // (관측값 [μg/m³] × 10)

  // 8. 순복사 (1분 평균)
  uint16_t net_radiation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767
                           // (관측값[W/m²] + 1000) × 10

  // 9. 전천복사 (1분 평균)
  uint16_t total_radiation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767
                             // (관측값[W/m²] + 1000) × 10

  // 10. 반사복사 (1분 평균)
  uint16_t reflected_radiation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~
                                 // 32767 (관측값[W/m²] + 1000) × 10

  // 11. 직달복사 (1분 평균)
  uint16_t direct_radiation;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767
                              // (관측값[W/m²] + 1000) × 10

  // 12. 현재 일기
  uint16_t
      current_weather;  // 사용비트: 6, 유효범위: 0 ~ 127 (인치 코드), 표현범위: 0 ~ 99 (관측값)

  uint16_t temp0[4];

  // 1. 토양수분 (10 cm)
  uint16_t soil_moisture_10cm;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 2. 토양수분 (20 cm)
  uint16_t soil_moisture_20cm;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 3. 토양수분 (30 cm)
  uint16_t soil_moisture_30cm;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 4. 토양수분 (50 cm)
  uint16_t soil_moisture_50cm;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                                // (관측값 * 10)

  // 5. 조도량 (1분 평균)
  uint16_t illuminance;  // 사용비트: 14, 유효범위: 0 ~ 32767 (인치 코드), 표현범위: 0 ~ 32767
                         // (관측값 * 100)

  // 6. 풍속 (1.5 m, 1분 평균)
  uint16_t wind_speed_1_5m;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                             // (관측값 * 10)

  // 7. 풍속 (4.0 m, 1분 평균)
  uint16_t wind_speed_4m;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                           // (관측값 * 10)

  // 8. 순간 풍속 (1.5 m)
  uint16_t instant_wind_speed_1_5m;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~
                                     // 1000 (관측값 * 10)

  // 9. 순간 풍속 (4.0 m)
  uint16_t instant_wind_speed_4m;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~
                                   // 1000 (관측값 * 10)

  // 10. 기온 (0.5 m)
  uint16_t temperature_0_5m;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500
                              // [(관측값 + 100) * 10]

  // 11. 기온 (4.0 m)
  uint16_t temperature_4m;  // 사용비트: 10, 유효범위: 0 ~ 2047 (인치 코드), 표현범위: 500 ~ 1500
                            // [(관측값 + 100) * 10]

  // 12. 습도 (0.5 m, 1분 평균)
  uint16_t humidity_0_5m;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000
                           // (관측값 * 10)

  // 13. 습도 (4.0 m, 1분 평균)
  uint16_t
      humidity_4m;  // 사용비트: 9, 유효범위: 0 ~ 1023 (인치 코드), 표현범위: 0 ~ 1000 (관측값 * 10)

  uint16_t temp1[9];

  uint16_t tacometer;
  uint8_t sensorStatus[8];
  uint8_t volateStatus;
  uint16_t crc;
  uint16_t end;
} kma_data_t;
#pragma pack(pop)

int16_t big_endian_to_little_endian(int16_t value)
{
  int16_t ret = 0;

  ret = (value >> 8 & 0x00FF);
  ret |= (value << 8);

  return ret;
}

void parse_kma3_response(const uint8_t* frame, uint32_t bytes_read)
{
  int16_t data;
  const char* dataNumName;
  const kma_data_t* kma_data = (kma_data_t*)frame;

      task_printf("=============================================================\r\n");
  task_printf("시작 표시    : 0x%04X\r\n", big_endian_to_little_endian(kma_data->header) & 0xFFFF);
  task_printf("프로토콜 버전: %02d-%02d-%02d\r\n", kma_data->protocol_year, kma_data->protocol_month,
         kma_data->protocol_day);
  task_printf("날짜/시간    : %02d-%02d-%02d %02d:%02d\r\n", kma_data->year, kma_data->month,
         kma_data->day, kma_data->hour, kma_data->min);
  task_printf("자료 구분    : %c\r\n", kma_data->dataType);

  switch (kma_data->dataNum)
  {
    case 0:
    case 1:
    case 2:
      dataNumName = "미사용";
      break;
    case 3:
      dataNumName = "일반용";
      break;
    case 0x0c:
      dataNumName = "관측요소에따라 부여(5~255)";
      break;
    default:
      dataNumName = "오류";
      break;
  }
  task_printf("자료형식번호 : %s\r\n", dataNumName);
  task_printf("지점 번호    : %d\r\n", big_endian_to_little_endian(kma_data->id));

  // 관측 데이터 출력
  data = big_endian_to_little_endian(kma_data->temperature);

  task_printf("A-1  기온      Temperature            :[%04d] %5.2f도C\r\n", data,
    (big_endian_to_little_endian(data) - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->wind_direction_avg);
  task_printf("A-2  풍향 Wind Direction Avg          :[%04d] %5.2f도\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->wind_speed_avg);
  task_printf("A-3  풍속 Wind Speed Avg              :[%04d] %5.2fm/s\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->wind_direction_instant);
  task_printf("A-4  순간 풍향 Wind Direction Instant :[%04d] %5.2f도\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->wind_speed_instant);
  task_printf("A-5  순간 풍속 Wind Speed Instant     :[%04d] %5.2fm/s\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->precipitation);
  task_printf("A-6  강수량    Precipitation          :[%04d] %dmm\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->pressure);
  task_printf("A-7  기압      Pressure               :[%04d] %5.2fhPa\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->precipitation_presence);
  task_printf("A-8  강수 유무 Precipitation Presence :[%04d] %d\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->snowfall);
  task_printf("A-9  적설      Snowfall               :[%04d] %5.2fcm\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->relative_humidity);
  task_printf("A-10 상대습도  Relative Humidity      :[%04d] %5.2f%%\r\n", data,
         data / 10.0);
  data = big_endian_to_little_endian(kma_data->precipitation_fine);
  task_printf("A-12 강수량    Precipitation Fine     :[%04d] %d\r\n", data, data);

  data = big_endian_to_little_endian(kma_data->solar_radiation);
  task_printf("B-1  일사      Solar Radiation        :[%04d] %d\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->sunshine_duration);
  task_printf("B-2  일조      Sunshine Duration      :[%04d] %d\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->surface_temperature);
  task_printf("B-3  지면온도  Surface Temperature    :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->grass_temperature);
  task_printf("B-4  초상온도  Grass Temperature      :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_5cm);
  task_printf("B-5  지중온도  Soil Temperature 5cm   :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_10cm);
  task_printf("B-6  지중온도  Soil Temperature 10cm  :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_20cm);
  task_printf("B-7  지중온도  Soil Temperature 20cm  :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_30cm);
  task_printf("B-8  지중온도  Soil Temperature 30cm  :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_50cm);
  task_printf("B-9  지중온도  Soil Temperature 50cm  :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_1m);
  task_printf("B-10 지중온도  Soil Temperature 1m    :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_1_5m);
  task_printf("B-11 지중온도  Soil Temperature 1.5m  :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_3m);
  task_printf("B-12 지중온도  Soil Temperature 3m    :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);
  data = big_endian_to_little_endian(kma_data->soil_temperature_5m);
  task_printf("B-13 지중온도  Soil Temperature 5m    :[%04d] %5.2f도C\r\n", data,
         (data - 1000) / 10.0);

  data = big_endian_to_little_endian(kma_data->cloud_height_1st);
  task_printf("C-1  1층 운고 Cloud Height 1st        :[%04d] %d\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->cloud_height_2nd);
  task_printf("C-2  2층 운고 Cloud Height 2nd        :[%04d] %d\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->cloud_height_3rd);
  task_printf("C-3  3층 운고 Cloud Height 3rd        :[%04d] %d\r\n", data, data);
  data = big_endian_to_little_endian(kma_data->cloud_amount);
  task_printf("C-4  운량     Cloud Amount            :[%04d] %d\r\n", data, data);
  task_printf("C-5  시정     Visibility              : %d\r\n",
         big_endian_to_little_endian(kma_data->visibility));
  task_printf("C-6  PM10     PM10 Concentration      : %d\r\n",
         big_endian_to_little_endian(kma_data->pm10_concentration));
  task_printf("C-7  PM2.5    Concentration           : %d\r\n",
         big_endian_to_little_endian(kma_data->pm25_concentration));
  task_printf("C-8  순복사   Net Radiation           : %d\r\n",
         big_endian_to_little_endian(kma_data->net_radiation));
  task_printf("C-9  전천복사 Total Radiation         : %d\r\n",
         big_endian_to_little_endian(kma_data->total_radiation));
  task_printf("C-10 반사복사 Reflected Radiation     : %d\r\n",
         big_endian_to_little_endian(kma_data->reflected_radiation));
  task_printf("C-11 직달일사 Direct Radiation        : %d\r\n",
         big_endian_to_little_endian(kma_data->direct_radiation));
  task_printf("C-12 현재일기 Current Weather         : %d\r\n",
         big_endian_to_little_endian(kma_data->current_weather));


  // 배열 데이터 출력
  for (int i = 0; i < 4; i++)
  {
    task_printf("L_%d:%d\r\n", i + 1, big_endian_to_little_endian(kma_data->temp0[i]));
  }

  task_printf("N-1  토양수분 soil_moisture_10cm      : %d\r\n", big_endian_to_little_endian(kma_data->soil_moisture_10cm));
  task_printf("N-2  토양수분 soil_moisture_20cm      : %d\r\n", big_endian_to_little_endian(kma_data->soil_moisture_20cm));
  task_printf("N-3  토양수분 soil_moisture_30cm      : %d\r\n", big_endian_to_little_endian(kma_data->soil_moisture_30cm));
  task_printf("N-4  토양수분 soil_moisture_50cm      : %d\r\n", big_endian_to_little_endian(kma_data->soil_moisture_50cm));
  task_printf("N-5  조도량   illuminance             : %d\r\n", big_endian_to_little_endian(kma_data->illuminance));
  task_printf("N-6  풍속     wind_speed_1_5m         : %d\r\n", big_endian_to_little_endian(kma_data->wind_speed_1_5m));
  task_printf("N-7  풍속     wind_speed_4m           : %d\r\n", big_endian_to_little_endian(kma_data->wind_speed_4m));
  task_printf("N-8  순간풍속 instant_wind_speed_1_5m : %d\r\n", big_endian_to_little_endian(kma_data->instant_wind_speed_1_5m));
  task_printf("N-9  순간풍속 instant_wind_speed_4m   : %d\r\n", big_endian_to_little_endian(kma_data->instant_wind_speed_4m));
  task_printf("N-10 기온     temperature_0_5m;       : %d\r\n", big_endian_to_little_endian(kma_data->temperature_0_5m));
  task_printf("N-11 기온     temperature_4m          : %d\r\n", big_endian_to_little_endian(kma_data->temperature_4m));
  task_printf("N-12 습도     humidity_0_5m           : %d\r\n", big_endian_to_little_endian(kma_data->humidity_0_5m));
  task_printf("N-13 습도     humidity_4m             : %d\r\n", big_endian_to_little_endian(kma_data->humidity_4m));


  for (int i = 0; i < 9; i++)
  {
    task_printf("L_%d:%d\r\n", i + 1, big_endian_to_little_endian(kma_data->temp1[i]));
  }

  task_printf("Tachometer: %d\r\n", big_endian_to_little_endian(kma_data->tacometer));

  for (int i = 0; i < 8; i++)
  {
    task_printf("SensorStatus[%d]: 0x%02X\r\n", i, kma_data->sensorStatus[i]);
  }

  task_printf("Voltage Status: 0x%02X\r\n", kma_data->volateStatus);
  task_printf("CRC: 0x%04X\r\n", big_endian_to_little_endian(kma_data->crc) & 0xFFFF);
  task_printf("End: 0x%04X\r\n", big_endian_to_little_endian(kma_data->end) & 0xFFFF);

}