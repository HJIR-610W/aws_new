
#ifndef BAROMETER_RMYOUNG_61302V_RS232_H
#define BAROMETER_RMYOUNG_61302V_RS232_H



#include <stdint.h>
int32_t rmyoung_61302v_rs232_init(void *opt);

float read_rmyoung_61302v_rs232_baromater(uint8_t *err);
#endif