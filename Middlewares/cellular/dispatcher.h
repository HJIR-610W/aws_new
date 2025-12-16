#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdint.h>
#include <stddef.h>

int dispatcher_init(void);
void dispatcher_handle_frame(const uint8_t* data, size_t len);
int dispatcher_send_sync(const uint8_t* cmd, size_t cmd_len, const char* const* p_ack_list, uint32_t ack_list_cnt, uint32_t* p_matched_index,
                         uint8_t* resp_buf, size_t resp_buf_size, uint32_t timeout_ms);

int dispatcher_send_async(const uint8_t* cmd, size_t cmd_len);
int dispatcher_subscribe(const char* prefix, void (*cb)(const uint8_t*, size_t, void*), void* ctx);
void dispatcher_unsubscribe(int sub_id);


#endif
