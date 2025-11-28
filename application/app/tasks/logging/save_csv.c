

#include "save_csv.h"

#include <stdio.h>
#include <stdint.h>

#include "app_file.h"
#include "old_aws_define.h"
#include "user_heap.h"

void save_aws_csv(uint8_t *p_data,uint16_t data_len)
{
    enum {CSV_SIZE=102400};
    int16_t len=0;
    AWS_DATA_STRUCT *p_aws;

    char *p_buffer = user_malloc(CSV_SIZE);

 //   len = snprintf(&p_buffer[len],CSV_SIZE-len,"%04d-%02d-%02d %02d:%02d,");


    
}