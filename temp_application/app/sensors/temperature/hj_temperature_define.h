#ifndef HJ_TEMPERATURE_DEFINE_H
#define HJ_TEMPERATURE_DEFINE_H

#define HJ_REG_NUM_TEMP 0
#define HJ_REG_NUM_HUMI 1

#define HJ_REG_NUM_TEMP_OFFSET  9
#define HJ_REG_NUM_HUMI_OFFSET 10

typedef struct
{
  uint16_t temperature;          // 40001 ?⑤룄 (-4000 ~ 12500)
  uint16_t humidity;             // 40002 ?듬룄 (0 ~ 10000)
  uint16_t temp_alarm;           // 40003 ?⑤룄 ?댁긽 ?뚮엺 (0, 1)
  uint16_t humi_alarm;           // 40004 ?듬룄 ?댁긽 ?뚮엺 (0, 1)
  uint16_t sw_version;           // 40005 SW Version (0100 ~ 9999)
  uint16_t hw_version;           // 40006 HW Version (0100 ~ 9999)
  uint16_t device_id;            // 40007 Device ID (1 ~ 255)
  uint16_t heater_power;         // 40008 Heater ?꾨쪟 ?ㅼ젙 (20, 110, 200)
  uint16_t heater_on_time;       // 40009 Heater ON ?쒓컙 ?ㅼ젙 (100 ~ 10000)
  uint16_t primary_temp_offset;  // 40010 Primary ?⑤룄 OFFSET ?ㅼ젙 (-500 ~ 500)
  uint16_t primary_humi_offset;  // 40011 Primary ?듬룄 OFFSET ?ㅼ젙 (-1000 ~ 1000)
  uint16_t aux_temp_offset;      // 40012 Auxiliary ?⑤룄 OFFSET ?ㅼ젙 (-500 ~ 500)
  uint16_t aux_humi_offset;      // 40013 Auxiliary ?듬룄 OFFSET ?ㅼ젙 (-1000 ~ 1000)
  uint16_t primary_temp;         // 40014 Primary ?⑤룄 (-4000 ~ 12500)
  uint16_t primary_humi;         // 40015 Primary ?듬룄 (0 ~ 10000)
  uint16_t aux_temp;             // 40016 Auxiliary ?⑤룄 (-4000 ~ 12500)
  uint16_t aux_humi;             // 40017 Auxiliary ?듬룄 (0 ~ 10000)
  uint16_t reserved[13];         // 40018 ~ 40030
  uint16_t heater_on_control;    // 40031 Heater ON ?쒖뼱 (0x00FF)
} hjtemp_register_map_t;


#endif