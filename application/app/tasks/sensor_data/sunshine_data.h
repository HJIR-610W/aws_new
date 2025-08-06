

#ifndef SUNSHINE_DATA_H
#define SUNSHINE_DATA_H

#include <stdint.h>

#define SUNSHINE_1MIN_FILE_NAME "SUNSHINE_01.rcd"
int32_t read_sunshine_1min(uint16_t year, uint16_t *sunshine_data, uint32_t read_size);

    int32_t sunshine_file_zero(int year);
void calculate_sunshine(void);

#endif