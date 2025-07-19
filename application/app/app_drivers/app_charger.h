
#ifndef APP_CHARGER_H
#define APP_CHARGER_H

#include <stdint.h>

#include <stdbool.h>

#define APP_CHARGER_HJ 0
#define APP_CHARGER_LS 1


void update_charger(int32_t charger);

bool is_chargerValid(void);
void read_chargerStatus(char *pBuff,uint16_t buffSize);

float read_solarVoltage1(uint8_t *err);
float read_solarVoltage2(uint8_t *err);
float read_solarCurrrent1(uint8_t *err);
float read_solarCurrent2(uint8_t *err);
float read_batteryVoltage1(uint8_t *err);
float read_batteryVoltage2(uint8_t *err);
float read_loadCurrent1(uint8_t *err);
float read_loadCurrent2(uint8_t *err);
float read_loadCurrent3(uint8_t *err);


#endif