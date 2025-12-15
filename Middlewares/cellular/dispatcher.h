#ifndef DISPATCHER_H
#define DISPATCHER_H

#include <stdint.h>
#include <stddef.h>

int dispatcher_init(void);

/* Handle an incoming AT/frame from the parser */
void dispatcher_handle_frame(const uint8_t* data, size_t len);

/* Synchronous send: send command bytes and wait for response matching prefix
 * match_prefix may be NULL to match any response. Returns bytes copied to resp_buf (>0)
 * or -1 on timeout/error.
 */
int dispatcher_send_sync(const uint8_t* cmd, size_t cmd_len, const char* const* p_ack_list, uint32_t ack_list_cnt, uint32_t* p_matched_index,
                         uint8_t* resp_buf, size_t resp_buf_size, uint32_t timeout_ms);

/* Asynchronous send (fire-and-forget). Returns 0 on send success, -1 on error */
int dispatcher_send_async(const uint8_t* cmd, size_t cmd_len);

/* Subscribe to unsolicited frames matching prefix. Returns subscription id or -1. */
int dispatcher_subscribe(const char* prefix, void (*cb)(const uint8_t*, size_t, void*), void* ctx);
void dispatcher_unsubscribe(int sub_id);

/* Wait for any incoming frame (no command send). Returns bytes received or -1 on timeout/error */
int dispatcher_wait_any(uint8_t* resp_buf, size_t resp_buf_size, uint32_t timeout_ms);

#endif
