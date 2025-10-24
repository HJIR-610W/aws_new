

#ifndef BAROMETER_RMYOUNG_61402V_H
#define BAROMETER_RMYOUNG_61402V_H


#include <stdint.h>
int32_t rmyoung_61402v_init(void *opt);
float read_baromater_rmyoung_61402v(uint8_t *err);
#endif