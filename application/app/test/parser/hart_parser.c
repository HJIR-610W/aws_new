

#include <string.h>


#include "hart_parser.h"
#include "debug_io.h"

#define printf dbg_printf

static const char* hart_command_to_str(uint8_t cmd)
{
  switch (cmd)
  {
  case 0x00: return "Read Unique Identifier";
  case 0x01: return "Read Primary Variable";
  case 0x02: return "Read Loop Current and Percent of Range";
  case 0x03: return "Read Dynamic Variables and Loop Current";
  case 0x09: return "Read Device Variables with Status";
  default: return "Unknown/Manufacturer Specific Command";
  }
}

static void parse_start_delimiter(uint8_t sd, const char** frame_type_str, const char** phy_str)
{
  uint8_t frame_type = sd & 0x07;
  uint8_t physical_layer = (sd >> 3) & 0x03;

  switch (frame_type)
  {
  case 0x01: *frame_type_str = "Burst Frame"; break;
  case 0x02: *frame_type_str = "Master to Field Device"; break;
  case 0x06: *frame_type_str = "Field Device to Master"; break;
  default: *frame_type_str = "Unknown"; break;
  }

  switch (physical_layer)
  {
  case 0x00: *phy_str = "FSK (Async)"; break;
  case 0x01: *phy_str = "PSK (Sync)"; break;
  default: *phy_str = "Reserved/Unknown"; break;
  }
}
static void parse_status_bytes(uint8_t status1, uint8_t status2)
{
  printf("---- Status Byte Analysis ----\n\r");
  printf("Status Byte 1: 0x%02X\n\r", status1);

  if (status1 & (1 << 7))  // 통신 오류
  {
    printf(" (Communication Error Detected)\n\r");
    if (status1 & (1 << 6)) printf(" - Parity Error\n\r");
    if (status1 & (1 << 5)) printf(" - Overrun Error\n\r");
    if (status1 & (1 << 4)) printf(" - Framing Error\n\r");
    if (status1 & (1 << 3)) printf(" - Checksum Error\n\r");
    if (status1 & (1 << 1)) printf(" - RX Buffer Overflow\n\r");
    if (status1 & (1 << 0)) printf(" - Overflow (Undefined)\n\r");
    printf("Status Byte 2: 0x%02X (should be 0 when communication error)\n\r", status2);
  }
  else  // Command 응답 해석
  {
    // 표준 상태 코드 해석
    switch (status1)
    {
    case 0x00: printf(" - No command-specific error\n\r"); break;
    case 0x02: printf(" - Invalid selection\n\r"); break;
    case 0x03: printf(" - Passed parameter too large\n\r"); break;
    case 0x04: printf(" - Passed parameter too small\n\r"); break;
    case 0x05: printf(" - Too few data bytes received\n\r"); break;
    case 0x06: printf(" - Device-specific command error\n\r"); break;
    case 0x07: printf(" - In write-protect mode\n\r"); break;
    case 0x10: printf(" - Access restricted\n\r"); break;
    case 0x20: printf(" - Device is busy\n\r"); break;
    case 0x40: printf(" - Command not implemented\n\r"); break;
    default:
      if (status1 != 0)
        printf(" - Unknown status code: 0x%02X (check protocol spec)\n\r", status1);
      else
        printf(" - No errors (Status OK)\n\r");
      break;
    }

    // 상태 바이트 2 해석
    printf("Status Byte 2: 0x%02X\n\r", status2);
    if (status2 & (1 << 7)) printf(" - Field device malfunction\n\r");
    if (status2 & (1 << 6)) printf(" - Configuration changed\n\r");
    if (status2 & (1 << 5)) printf(" - Cold start\n\r");
    if (status2 & (1 << 4)) printf(" - More status available\n\r");
    if (status2 & (1 << 3)) printf(" - Analog output current fixed\n\r");
    if (status2 & (1 << 2)) printf(" - Analog output saturated\n\r");
    if (status2 & (1 << 1)) printf(" - Non-primary variable out of limits\n\r");
    if (status2 & (1 << 0)) printf(" - Primary variable out of limits\n\r");
    if (status2 == 0x00) printf(" - No additional device status info\n\r");
  }

}
static const char* get_manufacturer_name(uint16_t id)
{
  switch (id)
  {
  case 0x0062: return "VEGA";
  default: return "Unknown Manufacturer";
  }
}static const char* get_device_type_name(uint16_t manufacturer_id, uint16_t expanded_device_type)
{
  if (manufacturer_id == 0x62)  // VEGA
  {
    switch (expanded_device_type)
    {
    case 0x62DC: return "레이다61";
    case 0x62DD: return "압력식";
    case 0x62BE: return "레이다64";
    default: return "Unknown VEGA Device";
    }
  }
  else if (manufacturer_id == 0x0017)  // Emerson 예시
  {
    switch (expanded_device_type)
    {
    case 0x1234: return "Rosemount 3051 Pressure Transmitter";
    default: return "Unknown Emerson Device";
    }
  }
  // 다른 제조사도 필요 시 추가
  return "Unknown Device Type";
}


static void parse_command_0_response_7(const uint8_t* data, uint8_t len)
{
  if (len < 2 + 22)  // 상태코드(2) + 최소 22바이트 데이터 필요
  {
    printf("Error: Command 0 response too short (expected at least 24 bytes including status, got %d).\n\r", len);
    return;
  }

  uint8_t status1 = data[0];
  uint8_t status2 = data[1];





  const uint8_t* rdata = &data[2];  // 실제 응답 데이터는 Status 2바이트 이후부터

  uint8_t fixed_value = rdata[0];
  uint16_t expanded_device_type = (rdata[1] << 8) | rdata[2];

  uint8_t min_preambles_req = rdata[3];
  uint8_t protocol_rev = rdata[4];
  uint8_t device_rev = rdata[5];
  uint8_t software_rev = rdata[6];

  uint8_t byte7 = rdata[7];
  uint8_t hardware_rev = (byte7 >> 3);
  uint8_t physical_signaling_code = (byte7 & 0x07);

  uint8_t flags = rdata[8];

  uint32_t device_id = (rdata[9] << 16) | (rdata[10] << 8) | rdata[11];

  uint8_t min_preambles_resp = rdata[12];
  uint8_t max_device_variables = rdata[13];

  uint16_t config_change_counter = (rdata[14] << 8) | rdata[15];
  uint8_t extended_field_device_status = rdata[16];

  uint16_t manufacturer_id = (rdata[17] << 8) | rdata[18];
  uint16_t private_label_distributor_code = (rdata[19] << 8) | rdata[20];

  uint8_t device_profile = rdata[21];

  const char* manufacturer_name = get_manufacturer_name(manufacturer_id);
  const char* device_type_name = get_device_type_name(manufacturer_id, expanded_device_type);


  printf("=========== Command 0 Response (Full Decode) ===========\n\r");
  printf("Status Byte 1: 0x%02X\n\r", status1);
  printf("Status Byte 2: 0x%02X\n\r", status2);
  parse_status_bytes(status1, status2);
  printf("--------------------------------------------------------\n\r");
  printf("Byte 0: Fixed Value: %d\n\r", fixed_value);
  printf("Expanded Device Type: 0x%04X (%s)\n\r", expanded_device_type, device_type_name);
  printf("Byte 3: Minimum Preambles Required (Master to Slave): %d\n\r", min_preambles_req);
  printf("Byte 4: HART Protocol Major Revision: %d\n\r", protocol_rev);
  printf("Byte 5: Device Revision: %d\n\r", device_rev);
  printf("Byte 6: Software Revision: %d\n\r", software_rev);
  printf("Byte 7: Hardware Revision (upper 5 bits): %d\n\r", hardware_rev);
  printf("Byte 7: Physical Signaling Code (lower 3 bits): %d\n\r", physical_signaling_code);
  printf("Byte 8: Flags: 0x%02X\n\r", flags);
  printf("Byte 9-11: Device ID (Serial Number): %06X\n\r", device_id);
  printf("Byte 12: Minimum Preambles (Slave to Master): %d\n\r", min_preambles_resp);
  printf("Byte 13: Maximum Number of Device Variables: %d\n\r", max_device_variables);
  printf("Byte 14-15: Configuration Change Counter: %d\n\r", config_change_counter);
  printf("Byte 16: Extended Field Device Status: 0x%02X\n\r", extended_field_device_status);
  printf("Manufacturer ID: 0x%04X (%s)\n\r", manufacturer_id, manufacturer_name);
  printf("Byte 19-20: Private Label Distributor Code: 0x%04X\n\r", private_label_distributor_code);
  printf("Byte 21: Device Profile: 0x%02X\n\r", device_profile);
  printf("========================================================\n\r");
}


static void parse_command_1_response(const uint8_t* data, uint8_t len)
{
  if (len < 5)
  {
    printf("Warning: Command 1 response too short.\n\r");
    return;
  }
  uint8_t unit_code = data[0];
  float pv;
  memcpy(&pv, &data[1], sizeof(float));

  printf("---- Command 1 (Primary Variable) ----\n\r");
  printf("Unit Code: %d\n\r", unit_code);
  printf("Primary Variable: %.3f\n\r", pv);
  printf("-------------------------------------\n\r");
}

static void parse_command_2_response(const uint8_t* data, uint8_t len)
{
  if (len < 8)
  {
    printf("Warning: Command 2 response too short.\n\r");
    return;
  }
  float loop_current, percent_range;
  memcpy(&loop_current, &data[0], sizeof(float));
  memcpy(&percent_range, &data[4], sizeof(float));

  printf("---- Command 2 (Loop Current & Percent Range) ----\n\r");
  printf("Loop Current: %.3f mA\n\r", loop_current);
  printf("Percent of Range: %.3f %%\n\r", percent_range);
  printf("--------------------------------------------------\n\r");
}

static void parse_command_3_response(const uint8_t* data, uint8_t len)
{
  if (len < 9)
  {
    printf("Warning: Command 3 response too short.\n\r");
    return;
  }

  float loop_current;
  memcpy(&loop_current, &data[0], sizeof(float));
  printf("---- Command 3 (Dynamic Variables & Loop Current) ----\n\r");
  printf("Loop Current: %.3f mA\n\r", loop_current);

  uint8_t offset = 4;
  uint8_t dv_units;
  float dv_value;
  int slot = 1;

  while (offset + 5 <= len && slot <= 4)
  {
    dv_units = data[offset];
    memcpy(&dv_value, &data[offset + 1], sizeof(float));
    printf("Dynamic Variable %d: %.3f (Unit Code: %d)\n\r", slot, dv_value, dv_units);
    offset += 5;
    slot++;
  }
  printf("-----------------------------------------------------\n\r");
}
static void parse_address(uint8_t start_delim, uint8_t address)
{
  uint8_t address_type = (start_delim >> 7) & 0x01;

  printf("---- Address Field Analysis ----\n\r");
  printf("Raw Address Byte: 0x%02X\n\r", address);

  if (address_type == 0)
  {
    uint8_t polling_address = address & 0x7F;
    printf("Address Type: Short (Polling Address)\n\r");
    printf("Polling Address: %d\n\r", polling_address);
  }
  else
  {
    printf("Address Type: Long (Unique Address)\n\r");
    printf("(Note: Full long address requires 5 bytes; this field is partial)\n\r");
    printf("First Byte: 0x%02X (Typically Manufacturer ID or part of unique address)\n\r", address);
  }
  printf("--------------------------------\n\r");
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"

#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')



static void parse_start_delimiter_detailed(uint8_t sd)
{
  uint8_t address_type = (sd >> 7) & 0x01;
  uint8_t expansion_bytes = (sd >> 5) & 0x03;
  uint8_t physical_layer = (sd >> 3) & 0x03;
  uint8_t frame_type = sd & 0x07;

  // 주소 타입 해석
  const char* address_type_str = (address_type == 0) ? "Short Address (Polling)" : "Long Address (Unique)";

  // Expansion 해석
  const char* expansion_str;
  switch (expansion_bytes)
  {
  case 0: expansion_str = "0 Expansion Bytes"; break;
  case 1: expansion_str = "1 Expansion Byte"; break;
  case 2: expansion_str = "2 Expansion Bytes"; break;
  case 3: expansion_str = "3 Expansion Bytes"; break;
  default: expansion_str = "Invalid"; break;  // 이론상 불가
  }

  // 물리 계층 해석
  const char* phy_str;
  switch (physical_layer)
  {
  case 0: phy_str = "FSK (Async)"; break;
  case 1: phy_str = "PSK (Sync)"; break;
  default: phy_str = "Reserved/Unknown"; break;
  }

  // 프레임 타입 해석
  const char* frame_str;
  switch (frame_type)
  {
  case 0x01: frame_str = "Burst Frame"; break;
  case 0x02: frame_str = "Master to Field Device"; break;
  case 0x06: frame_str = "Field Device to Master"; break;
  default: frame_str = "Reserved/Unknown"; break;
  }

  printf("---- Start Delimiter Detailed Analysis ----\n\r");

  printf("Bit7: Address Type: %s\n\r", address_type_str);
  printf("Bit6-5: Expansion Bytes: %s\n\r", expansion_str);
  printf("Bit4-3: Physical Layer: %s\n\r", phy_str);
  printf("Bit2-0: Frame Type: %s\n\r", frame_str);
  printf("------------------------------------------\n\r");
}

void hart_parse(uint8_t* packet, uint16_t len)
{
  if (len < 5)
  {
    printf("Error: Packet too short.\n\r");
    return;
  }

  uint16_t index = 0;
  uint8_t preamble_count = 0;

  while (index < len && packet[index] == 0xFF)
  {
    preamble_count++;
    index++;
  }

  if (preamble_count < 5)
  {
    printf("Error: Insufficient preamble (%d bytes).\n\r", preamble_count);
    return;
  }

  if (index + 4 > len)
  {
    printf("Error: Packet too short after preamble.\n\r");
    return;
  }

  uint8_t start_delim = packet[index++];
  uint8_t address = packet[index++];
  uint8_t command = packet[index++];
  uint8_t byte_count = packet[index++];

  printf("start delimiter:%02X\r\n\r", start_delim);
  parse_start_delimiter_detailed(start_delim);
  parse_address(start_delim, address);
  if (index + byte_count + 1 > len)
  {
    printf("Error: Packet length mismatch (Byte Count: %d).\n\r", byte_count);
    return;
  }

  const uint8_t* data = &packet[index];
  uint8_t checksum = packet[index + byte_count];

  const char* frame_type_str;
  const char* phy_str;
  parse_start_delimiter(start_delim, &frame_type_str, &phy_str);

  // Checksum verification
  uint8_t calc_checksum = 0;
  for (uint16_t i = preamble_count; i < len - 1; i++)
  {
    calc_checksum ^= packet[i];
  }

  printf("========== HART 7 Packet Analysis ==========\n\r");
  printf("Preamble: %d bytes\n\r", preamble_count);
  printf("Start Delimiter: 0x%02X\n\r", start_delim);
  printf("Frame Type: %s\n\r", frame_type_str);
  printf("Physical Layer: %s\n\r", phy_str);
  printf("Address: 0x%02X\n\r", address);
  printf("Command: 0x%02X (%s)\n\r", command, hart_command_to_str(command));
  printf("Byte Count: %d\n\r", byte_count);

  printf("Data: ");
  for (uint8_t i = 0; i < byte_count; i++)
  {
    printf("0x%02X ", data[i]);
  }
  printf("\n\r");

  printf("Checksum: 0x%02X [%s]\n\r", checksum,
    (checksum == calc_checksum) ? "Valid" : "Invalid");
  printf("-------------------------------------------\n\r");

  if (strcmp(frame_type_str, "Field Device to Master") == 0)
  {
    // 응답이면 Command별로 파싱
    switch (command)
    {
    case 0x00: 
      if (byte_count == 0x18)
      {
        parse_command_0_response_7(data, byte_count);
      }
      else
      {
        printf("하트버전 확인\r\n\r");
      }
      break;
    case 0x01: parse_command_1_response(data, byte_count); break;
    case 0x02: parse_command_2_response(data, byte_count); break;
    case 0x03: parse_command_3_response(data, byte_count); break;
    default:
      printf("No detailed parser for this command yet.\n\r");
      break;
    }
  }
  else
  {
    printf("(This is a Master request packet; no response data parsing.)\n\r");
  }

}
