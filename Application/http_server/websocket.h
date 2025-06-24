#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stdint.h>
#include <stddef.h>

#define WS_OPCODE_CONTINUATION 0x0
#define WS_OPCODE_TEXT         0x1
#define WS_OPCODE_BINARY       0x2
#define WS_OPCODE_CLOSE        0x8
#define WS_OPCODE_PING         0x9
#define WS_OPCODE_PONG         0xA

typedef struct {
    uint8_t fin;
    uint8_t opcode;
    uint8_t masked;
    uint64_t payload_len;
    uint8_t mask[4];
    uint8_t* payload;
    size_t header_len;
} ws_frame_t;

int websocket_handshake(int client_socket, const char* key);
void websocket_handle_connection(int client_socket);
void websocket_send_text_frame(int client_socket, const char* text, size_t len);

#endif // WEBSOCKET_H