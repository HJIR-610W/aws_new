
#ifndef PARSE_AWS_H
#define PARSE_AWS_H

#include <stdint.h>
#include "kma_define.h"

void print_kma2_command_request(const kma2_command_request_t* req);
void print_kma3_command_request(const kma3_command_request_t* req);
void parse_kma2_response(const uint8_t* frame, uint32_t bytes_read);
void parse_kma3_response(const uint8_t* frame, uint32_t bytes_read);

#endif