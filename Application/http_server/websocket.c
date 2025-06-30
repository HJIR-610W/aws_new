#include "websocket.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cmsis_os2.h"
#include "lwip/sockets.h"
#include "task_logging.h"
#include "user_heap.h"
#include "app_file.h"
#include "terminal_bridge.h"
// mbedTLS includes for WebSocket handshake
#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"

#define WS_MAGIC_STRING "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"
#define WS_FRAME_HEADER_SIZE 14
#define WS_BUFFER_SIZE 1024
#define WS_UPLOAD_CHUNK_SIZE 1024

typedef struct {
    char filename[64];
    uint8_t* buffer;
    size_t total_size;
    size_t received_size;
    bool is_receiving;
} file_upload_t;

static void websocket_send_binary_frame(int client_socket, const uint8_t* data, size_t len);
static int websocket_parse_frame(const uint8_t* buffer, size_t len, ws_frame_t* frame);
static void websocket_handle_binary_download(int client_socket, const char* filename);
static void websocket_handle_file_upload_start(int client_socket, const char* filename, size_t file_size);
static void websocket_handle_file_upload_chunk(int client_socket, const uint8_t* data, size_t len);
static void websocket_handle_file_upload_complete(int client_socket);
static int websocket_save_uploaded_file(const char* filename, const uint8_t* data, size_t size);
static int websocket_generate_accept_key(const char* client_key, char* accept_key, size_t accept_key_size);

int websocket_handshake(int client_socket, const char* key)
{
    if (!key) {
        task_printf("WebSocket: No key provided for handshake\r\n");
        return -1;
    }

    char handshake_response[512];
    char accept_key[64];
    
    task_printf("WebSocket: Processing handshake with key: %s\r\n", key);
    
    // RFC 6455에 따른 올바른 Accept 키 생성
    if (websocket_generate_accept_key(key, accept_key, sizeof(accept_key)) != 0) {
        task_printf("WebSocket: Failed to generate accept key\r\n");
        return -1;
    }
    
    int header_len = snprintf(handshake_response, sizeof(handshake_response),
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "\r\n",
        accept_key);

    task_printf("WebSocket: Sending handshake response (%d bytes)\r\n", header_len);
    task_printf("WebSocket: Accept key: %s\r\n", accept_key);
    
    int sent = send(client_socket, handshake_response, header_len, 0);
    if (sent < 0) {
        task_printf("WebSocket: Failed to send handshake response (error: %d)\r\n", errno);
        return -1;
    }
    
    if (sent != header_len) {
        task_printf("WebSocket: Partial handshake sent (%d/%d bytes)\r\n", sent, header_len);
        return -1;
    }

    task_printf("WebSocket: Handshake completed successfully\r\n");
    return 0;
}

static int websocket_parse_frame(const uint8_t* buffer, size_t len, ws_frame_t* frame)
{
    if (len < 2) {
        task_printf("WebSocket: Frame too short (len: %zu)\r\n", len);
        return -1;
    }

    frame->fin = (buffer[0] & 0x80) != 0;
    frame->opcode = buffer[0] & 0x0F;
    frame->masked = (buffer[1] & 0x80) != 0;
    frame->payload_len = buffer[1] & 0x7F;

    size_t header_len = 2;

    task_printf("WebSocket: Frame header - FIN: %d, opcode: 0x%02X, masked: %d, initial_len: %zu\r\n",
               frame->fin, frame->opcode, frame->masked, frame->payload_len);

    if (frame->payload_len == 126) {
        if (len < 4) {
            task_printf("WebSocket: Not enough data for extended length (len: %zu)\r\n", len);
            return -1;
        }
        frame->payload_len = (buffer[2] << 8) | buffer[3];
        header_len = 4;
        task_printf("WebSocket: Extended length (16-bit): %zu\r\n", frame->payload_len);
    } else if (frame->payload_len == 127) {
        if (len < 10) {
            task_printf("WebSocket: Not enough data for 64-bit length (len: %zu)\r\n", len);
            return -1;
        }
        frame->payload_len = 0;
        for (int i = 0; i < 8; i++) {
            frame->payload_len = (frame->payload_len << 8) | buffer[2 + i];
        }
        header_len = 10;
        task_printf("WebSocket: Extended length (64-bit): %zu\r\n", frame->payload_len);
    }

    if (frame->masked) {
        if (len < header_len + 4) {
            task_printf("WebSocket: Not enough data for mask (len: %zu, needed: %zu)\r\n", len, header_len + 4);
            return -1;
        }
        memcpy(frame->mask, buffer + header_len, 4);
        header_len += 4;
        task_printf("WebSocket: Mask: %02X %02X %02X %02X\r\n", 
                   frame->mask[0], frame->mask[1], frame->mask[2], frame->mask[3]);
    }

    frame->header_len = header_len;
    frame->payload = (uint8_t*)(buffer + header_len);

    if (len < header_len + frame->payload_len) {
        task_printf("WebSocket: Not enough data for payload (len: %zu, needed: %zu)\r\n", 
                   len, header_len + frame->payload_len);
        return -1;
    }

    if (frame->masked) {
        for (size_t i = 0; i < frame->payload_len; i++) {
            frame->payload[i] ^= frame->mask[i % 4];
        }
    }

    task_printf("WebSocket: Frame parsed successfully - header_len: %zu, payload_len: %zu\r\n",
               frame->header_len, frame->payload_len);

    return frame->header_len + frame->payload_len;
}

static void websocket_send_binary_frame(int client_socket, const uint8_t* data, size_t len)
{
    uint8_t header[10];
    int header_len = 0;

    header[0] = 0x82;

    if (len < 126) {
        header[1] = (uint8_t)len;
        header_len = 2;
    } else if (len < 65536) {
        header[1] = 126;
        header[2] = (len >> 8) & 0xFF;
        header[3] = len & 0xFF;
        header_len = 4;
    } else {
        header[1] = 127;
        for (int i = 0; i < 8; i++) {
            header[2 + i] = (len >> (56 - i * 8)) & 0xFF;
        }
        header_len = 10;
    }

    send(client_socket, header, header_len, 0);
    
    const uint8_t* data_ptr = data;
    size_t remaining = len;
    
    while (remaining > 0) {
        size_t chunk_size = (remaining > 1024) ? 1024 : remaining;
        int sent = send(client_socket, data_ptr, chunk_size, 0);
        
        if (sent <= 0) {
            task_printf("WebSocket: Failed to send binary data\r\n");
            break;
        }
        
        data_ptr += sent;
        remaining -= sent;
    }
}

static void websocket_handle_binary_download(int client_socket, const char* filename)
{
    char file_path[128];
    FSIZE_t file_size = 0;
    FRESULT res;
    uint8_t* file_buffer = NULL;

    snprintf(file_path, sizeof(file_path), "0:/%s", filename);
    
    res = get_file_size(file_path, &file_size);
    if (res != FR_OK || file_size == 0) {
        task_printf("WebSocket: File not found or empty: %s (error: %d)\r\n", filename, res);
        
        const char* error_msg = "File not found";
        websocket_send_text_frame(client_socket, error_msg, strlen(error_msg));
        return;
    }

    if (file_size > (1024 * 1024 * 10)) {
        task_printf("WebSocket: File too large: %lu bytes\r\n", (unsigned long)file_size);
        
        const char* error_msg = "File too large";
        websocket_send_text_frame(client_socket, error_msg, strlen(error_msg));
        return;
    }

    file_buffer = (uint8_t*)aws_malloc(file_size);
    if (file_buffer == NULL) {
        task_printf("WebSocket: Failed to allocate memory for file: %lu bytes\r\n", (unsigned long)file_size);
        
        const char* error_msg = "Memory allocation failed";
        websocket_send_text_frame(client_socket, error_msg, strlen(error_msg));
        return;
    }

    res = read_file(file_path, file_buffer, (uint32_t)file_size, 0);
    if (res != FR_OK) {
        task_printf("WebSocket: Failed to read file: %s (error: %d)\r\n", filename, res);
        aws_free(file_buffer);
        
        const char* error_msg = "File read failed";
        websocket_send_text_frame(client_socket, error_msg, strlen(error_msg));
        return;
    }

    websocket_send_binary_frame(client_socket, file_buffer, file_size);
    
    task_printf("WebSocket: Binary file download completed - %s (%lu bytes)\r\n", 
                filename, (unsigned long)file_size);
    
    aws_free(file_buffer);
}

void websocket_send_text_frame(int client_socket, const char* text, size_t len)
{
    uint8_t header[10];
    int header_len = 0;

    header[0] = 0x81;

    if (len < 126) {
        header[1] = (uint8_t)len;
        header_len = 2;
    } else if (len < 65536) {
        header[1] = 126;
        header[2] = (len >> 8) & 0xFF;
        header[3] = len & 0xFF;
        header_len = 4;
    } else {
        header[1] = 127;
        for (int i = 0; i < 8; i++) {
            header[2 + i] = (len >> (56 - i * 8)) & 0xFF;
        }
        header_len = 10;
    }

    send(client_socket, header, header_len, 0);
    send(client_socket, text, len, 0);
}

void websocket_handle_connection(int client_socket)
{
    uint8_t* buffer = (uint8_t*)aws_malloc(WS_BUFFER_SIZE);
    if (buffer == NULL) {
        task_printf("WebSocket: Failed to allocate buffer\r\n");
        return;
    }

    static file_upload_t upload_state = {0};
    uint8_t* frame_buffer = NULL;
    size_t frame_buffer_size = 0;
    size_t total_received = 0;
   // bool expecting_continuation = false;
    
    task_printf("WebSocket: Connection established\r\n");

    while (1) {
        int bytes_received = recv(client_socket, buffer, WS_BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            task_printf("WebSocket: Connection closed or error (bytes: %d, errno: %d)\r\n", bytes_received, errno);
            break;
        }

        task_printf("WebSocket: Received %d bytes\r\n", bytes_received);

        // 프레임 버퍼가 없으면 새로 할당
        if (frame_buffer == NULL) {
            frame_buffer_size = bytes_received;
            frame_buffer = (uint8_t*)aws_malloc(frame_buffer_size);
            if (frame_buffer == NULL) {
                task_printf("WebSocket: Failed to allocate frame buffer\r\n");
                break;
            }
            memcpy(frame_buffer, buffer, bytes_received);
            total_received = bytes_received;
        } else {
            // 기존 프레임 버퍼에 새 데이터 추가
            size_t new_size = total_received + bytes_received;
            uint8_t* new_buffer = (uint8_t*)aws_malloc(new_size);
            if (new_buffer == NULL) {
                task_printf("WebSocket: Failed to reallocate frame buffer\r\n");
                aws_free(frame_buffer);
                frame_buffer = NULL;
                break;
            }
            memcpy(new_buffer, frame_buffer, total_received);
            memcpy(new_buffer + total_received, buffer, bytes_received);
            aws_free(frame_buffer);
            frame_buffer = new_buffer;
            frame_buffer_size = new_size;
            total_received = new_size;
        }

        // 프레임 파싱 시도
        ws_frame_t frame;
        int frame_size = websocket_parse_frame(frame_buffer, total_received, &frame);
        
        if (frame_size < 0) {
            // 프레임이 완료되지 않았으면 더 기다림
            if (total_received < 65536) { // 최대 64KB까지 버퍼링
                continue;
            } else {
                task_printf("WebSocket: Frame too large, discarding\r\n");
                aws_free(frame_buffer);
                frame_buffer = NULL;
                total_received = 0;
                continue;
            }
        }

        // 완전한 프레임을 받았으면 처리
        task_printf("WebSocket: Complete frame received (size: %d)\r\n", frame_size);

        task_printf("WebSocket: Parsed frame - opcode: 0x%02X, fin: %d, masked: %d, payload_len: %zu\r\n", 
                   frame.opcode, frame.fin, frame.masked, frame.payload_len);

        switch (frame.opcode) {
            case WS_OPCODE_TEXT:
                {
                    char* text_msg = (char*)aws_malloc(frame.payload_len + 1);
                    if (text_msg) {
                        memcpy(text_msg, frame.payload, frame.payload_len);
                        text_msg[frame.payload_len] = '\0';
                        
                        task_printf("WebSocket: Received text: %s\r\n", text_msg);
                        
                        if (strncmp(text_msg, "download:", 9) == 0) {
                            const char* filename = text_msg + 9;
                            websocket_handle_binary_download(client_socket, filename);
                        }
                        else if (strncmp(text_msg, "upload_start:", 13) == 0) {
                            // Format: "upload_start:filename:filesize"
                            char* filename_start = text_msg + 13;
                            char* size_start = strchr(filename_start, ':');
                            if (size_start) {
                                *size_start = '\0';
                                size_start++;
                                size_t file_size = atol(size_start);
                                
                                task_printf("WebSocket: Upload start request - filename: %s, size: %zu\r\n", 
                                           filename_start, file_size);
                                
                                websocket_handle_file_upload_start(client_socket, filename_start, file_size);
                                
                                // 기존 버퍼 정리
                                if (upload_state.buffer) {
                                    aws_free(upload_state.buffer);
                                    upload_state.buffer = NULL;
                                }
                                
                                // 업로드 상태 초기화
                                strncpy(upload_state.filename, filename_start, sizeof(upload_state.filename) - 1);
                                upload_state.filename[sizeof(upload_state.filename) - 1] = '\0';
                                upload_state.total_size = file_size;
                                upload_state.received_size = 0;
                                upload_state.is_receiving = false; // 버퍼 할당 후에 true로 설정
                                
                                if (file_size > 0 && file_size <= (10 * 1024 * 1024)) { // 10MB 제한
                                    upload_state.buffer = (uint8_t*)aws_malloc(file_size);
                                    if (upload_state.buffer) {
                                        upload_state.is_receiving = true;
                                        task_printf("WebSocket: Upload buffer allocated successfully (%zu bytes)\r\n", file_size);
                                    } else {
                                        task_printf("WebSocket: Failed to allocate upload buffer (%zu bytes)\r\n", file_size);
                                    }
                                } else {
                                    task_printf("WebSocket: Invalid file size (%zu bytes)\r\n", file_size);
                                }
                            } else {
                                task_printf("WebSocket: Invalid upload_start format\r\n");
                            }
                        }
                        else if (strcmp(text_msg, "upload_complete") == 0) {
                            if (upload_state.is_receiving) {
                                websocket_handle_file_upload_complete(client_socket);
                                
                                // 파일 저장
                                if (websocket_save_uploaded_file(upload_state.filename, 
                                                               upload_state.buffer, 
                                                               upload_state.received_size) == 0) {
                                    task_printf("WebSocket: File upload completed - %s (%zu bytes)\r\n", 
                                              upload_state.filename, upload_state.received_size);
                                }
                                
                                // 상태 초기화
                                if (upload_state.buffer) {
                                    aws_free(upload_state.buffer);
                                    upload_state.buffer = NULL;
                                }
                                upload_state.is_receiving = false;
                            }
                        }
                        
                        aws_free(text_msg);
                    }
                }
                break;

            case WS_OPCODE_BINARY:
                task_printf("WebSocket: Binary frame received - payload_len: %zu\r\n", frame.payload_len);
                if (upload_state.is_receiving && upload_state.buffer) {
                    websocket_handle_file_upload_chunk(client_socket, frame.payload, frame.payload_len);
                    
                    // 데이터를 버퍼에 복사
                    size_t copy_size = frame.payload_len;
                    if (upload_state.received_size + copy_size > upload_state.total_size) {
                        copy_size = upload_state.total_size - upload_state.received_size;
                        task_printf("WebSocket: Warning - chunk size truncated from %zu to %zu\r\n", 
                                   frame.payload_len, copy_size);
                    }
                    
                    if (copy_size > 0) {
                        memcpy(upload_state.buffer + upload_state.received_size, frame.payload, copy_size);
                        upload_state.received_size += copy_size;
                        
                        task_printf("WebSocket: Upload progress: %zu/%zu bytes (%.1f%%)\r\n", 
                                  upload_state.received_size, upload_state.total_size,
                                  (float)upload_state.received_size / upload_state.total_size * 100.0f);
                    }
                } else {
                    task_printf("WebSocket: Received binary data (%zu bytes) - not in upload mode (is_receiving: %d, buffer: %p)\r\n", 
                               frame.payload_len, upload_state.is_receiving, upload_state.buffer);
                }
                break;

            case WS_OPCODE_CLOSE:
                task_printf("WebSocket: Close frame received\r\n");
                if (upload_state.buffer) {
                    aws_free(upload_state.buffer);
                }
                aws_free(buffer);
                return;

            case WS_OPCODE_PING:
                {
                    uint8_t pong_frame[2] = {0x8A, 0x00};
                    send(client_socket, pong_frame, 2, 0);
                    task_printf("WebSocket: Pong sent\r\n");
                }
                break;

            default:
                task_printf("WebSocket: Unknown opcode: 0x%02X\r\n", frame.opcode);
                break;
        }
        
        // 남은 데이터가 있는지 확인
        if (frame_size < (int)total_received) {
            size_t remaining = total_received - frame_size;
            uint8_t* new_buffer = (uint8_t*)aws_malloc(remaining);
            if (new_buffer) {
                memcpy(new_buffer, frame_buffer + frame_size, remaining);
                aws_free(frame_buffer);
                frame_buffer = new_buffer;
                total_received = remaining;
            } else {
                aws_free(frame_buffer);
                frame_buffer = NULL;
                total_received = 0;
            }
        } else {
            // 프레임 처리 완료, 버퍼 정리
            aws_free(frame_buffer);
            frame_buffer = NULL;
            total_received = 0;
        }
    }

    // 연결 종료 시 모든 메모리 정리
    if (frame_buffer) {
        aws_free(frame_buffer);
    }
    if (upload_state.buffer) {
        aws_free(upload_state.buffer);
        upload_state.buffer = NULL;
        upload_state.is_receiving = false;
    }
    aws_free(buffer);
    task_printf("WebSocket: Connection handler terminated\r\n");
}

static void websocket_handle_file_upload_start(int client_socket, const char* filename, size_t file_size)
{
    task_printf("WebSocket: File upload started - %s (%zu bytes)\r\n", filename, file_size);
    
    char response[128];
    snprintf(response, sizeof(response), "upload_ready:%s:%zu", filename, file_size);
    websocket_send_text_frame(client_socket, response, strlen(response));
}

static void websocket_handle_file_upload_chunk(int client_socket, const uint8_t* data, size_t len)
{
    // 청크 수신 확인 (선택사항)
    // 너무 많은 로그를 방지하기 위해 주석 처리 가능
    // task_printf("WebSocket: Received chunk: %zu bytes\r\n", len);
}

static void websocket_handle_file_upload_complete(int client_socket)
{
    task_printf("WebSocket: File upload process completed\r\n");
    
    const char* response = "upload_success";
    websocket_send_text_frame(client_socket, response, strlen(response));
}

static int websocket_save_uploaded_file(const char* filename, const uint8_t* data, size_t size)
{
    char file_path[128];
    FRESULT res;
    
    snprintf(file_path, sizeof(file_path), "0:/%s", filename);
    
    task_printf("WebSocket: Saving uploaded file to %s (%zu bytes)\r\n", file_path, size);
    
    // app_file.h의 write_file 함수 사용 (세마포어로 보호됨)
    // offset 0에서 전체 파일을 새로 작성
    res = write_file(file_path, (uint8_t*)data, (uint32_t)size, 0);
    
    if (res != FR_OK) {
        task_printf("WebSocket: Failed to write file %s (error: %d)\r\n", file_path, res);
        return -1;
    }
    
    task_printf("WebSocket: File saved successfully - %s (%zu bytes)\r\n", filename, size);
    return 0;
}

static int websocket_generate_accept_key(const char* client_key, char* accept_key, size_t accept_key_size)
{
    if (!client_key || !accept_key || accept_key_size < 29) {
        task_printf("WebSocket: Invalid parameters for accept key generation\r\n");
        return -1;
    }
    
    // 1. 클라이언트 키와 WebSocket 매직 스트링을 연결
    char combined_key[128];
    int combined_len = snprintf(combined_key, sizeof(combined_key), "%s%s", client_key, WS_MAGIC_STRING);
    
    if (combined_len >= sizeof(combined_key)) {
        task_printf("WebSocket: Combined key too long\r\n");
        return -1;
    }
    
    task_printf("WebSocket: Combined key: %s\r\n", combined_key);
    
    // 2. SHA-1 해시 계산
    unsigned char sha1_output[20];
    int ret = mbedtls_sha1_ret((const unsigned char*)combined_key, combined_len, sha1_output);
    if (ret != 0) {
        task_printf("WebSocket: SHA-1 calculation failed (error: %d)\r\n", ret);
        return -1;
    }
    
    // 3. Base64 인코딩
    size_t olen = 0;
    ret = mbedtls_base64_encode((unsigned char*)accept_key, accept_key_size - 1, &olen, sha1_output, 20);
    if (ret != 0) {
        task_printf("WebSocket: Base64 encoding failed (error: %d)\r\n", ret);
        return -1;
    }
    
    accept_key[olen] = '\0';
    
    task_printf("WebSocket: Generated accept key: %s\r\n", accept_key);
    return 0;
}

// 터미널 WebSocket 연결용 전역 변수
static int g_terminal_client_socket = -1;
//static void (*g_terminal_output_callback)(const char* data, size_t len) = NULL;

// 터미널 WebSocket 데이터 전송 함수
void websocket_terminal_send_data(int client_socket, const char* data, size_t len)
{
    if (client_socket < 0 || !data || len == 0) {
        return;
    }
    
    websocket_send_text_frame(client_socket, data, len);
}

// 터미널 브리지에서 WebSocket으로 데이터 전송하는 콜백 함수
static void terminal_to_websocket_callback(const char* data, size_t len)
{
    if (g_terminal_client_socket >= 0 && data && len > 0) {
        websocket_terminal_send_data(g_terminal_client_socket, data, len);
    }
}

// 터미널 WebSocket 연결 처리 함수
void websocket_terminal_handle_connection(int client_socket)
{
    uint8_t* buffer = (uint8_t*)aws_malloc(WS_BUFFER_SIZE);
    if (buffer == NULL) {
        task_printf("Terminal WebSocket: Failed to allocate buffer\r\n");
        return;
    }
    
    g_terminal_client_socket = client_socket;
    
    // 터미널 브리지 초기화 및 콜백 설정
    terminal_bridge_init();
    terminal_bridge_set_output_callback(terminal_to_websocket_callback);
    
    task_printf("Terminal WebSocket: Connection established\r\n");
    
    // 환영 메시지 전송
    const char* welcome_msg = "\r\n=== Terminal WebSocket Connected ===\r\n";
    websocket_terminal_send_data(client_socket, welcome_msg, strlen(welcome_msg));
    
    uint8_t* frame_buffer = NULL;
    size_t frame_buffer_size = 0;
    size_t total_received = 0;
    
    while (1) {
        int bytes_received = recv(client_socket, buffer, WS_BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            task_printf("Terminal WebSocket: Connection closed or error (bytes: %d, errno: %d)\r\n", 
                       bytes_received, errno);
            break;
        }
        
        task_printf("Terminal WebSocket: Received %d bytes\r\n", bytes_received);
        
        // 프레임 버퍼 관리 (기존 웹소켓과 동일한 로직)
        if (frame_buffer == NULL) {
            frame_buffer_size = bytes_received;
            frame_buffer = (uint8_t*)aws_malloc(frame_buffer_size);
            if (frame_buffer == NULL) {
                task_printf("Terminal WebSocket: Failed to allocate frame buffer\r\n");
                break;
            }
            memcpy(frame_buffer, buffer, bytes_received);
            total_received = bytes_received;
        } else {
            size_t new_size = total_received + bytes_received;
            uint8_t* new_buffer = (uint8_t*)aws_malloc(new_size);
            if (new_buffer == NULL) {
                task_printf("Terminal WebSocket: Failed to reallocate frame buffer\r\n");
                aws_free(frame_buffer);
                frame_buffer = NULL;
                break;
            }
            memcpy(new_buffer, frame_buffer, total_received);
            memcpy(new_buffer + total_received, buffer, bytes_received);
            aws_free(frame_buffer);
            frame_buffer = new_buffer;
            frame_buffer_size = new_size;
            total_received = new_size;
        }
        
        // 프레임 파싱
        ws_frame_t frame;
        int frame_size = websocket_parse_frame(frame_buffer, total_received, &frame);
        
        if (frame_size < 0) {
            if (total_received < 65536) {
                continue;
            } else {
                task_printf("Terminal WebSocket: Frame too large, discarding\r\n");
                aws_free(frame_buffer);
                frame_buffer = NULL;
                total_received = 0;
                continue;
            }
        }
        
        // 프레임 처리
        switch (frame.opcode) {
            case WS_OPCODE_TEXT:
                {
                    char* text_msg = (char*)aws_malloc(frame.payload_len + 1);
                    if (text_msg) {
                        memcpy(text_msg, frame.payload, frame.payload_len);
                        text_msg[frame.payload_len] = '\0';
                        
                        task_printf("Terminal WebSocket: Received command: %s\r\n", text_msg);
                        
                        // 터미널 브리지로 명령 전송
                        terminal_bridge_send_command(text_msg, frame.payload_len);
                        
                        aws_free(text_msg);
                    }
                }
                break;
                
            case WS_OPCODE_CLOSE:
                task_printf("Terminal WebSocket: Close frame received\r\n");
                aws_free(buffer);
                if (frame_buffer) {
                    aws_free(frame_buffer);
                }
                terminal_bridge_cleanup();
                g_terminal_client_socket = -1;
                return;
                
            case WS_OPCODE_PING:
                {
                    uint8_t pong_frame[2] = {0x8A, 0x00};
                    send(client_socket, pong_frame, 2, 0);
                    task_printf("Terminal WebSocket: Pong sent\r\n");
                }
                break;
                
            default:
                task_printf("Terminal WebSocket: Ignoring opcode: 0x%02X\r\n", frame.opcode);
                break;
        }
        
        // 남은 데이터 처리 (기존 웹소켓과 동일한 로직)
        if (frame_size < (int)total_received) {
            size_t remaining = total_received - frame_size;
            uint8_t* new_buffer = (uint8_t*)aws_malloc(remaining);
            if (new_buffer) {
                memcpy(new_buffer, frame_buffer + frame_size, remaining);
                aws_free(frame_buffer);
                frame_buffer = new_buffer;
                total_received = remaining;
            } else {
                aws_free(frame_buffer);
                frame_buffer = NULL;
                total_received = 0;
            }
        } else {
            aws_free(frame_buffer);
            frame_buffer = NULL;
            total_received = 0;
        }
    }
    
    // 연결 종료 시 정리
    if (frame_buffer) {
        aws_free(frame_buffer);
    }
    aws_free(buffer);
    terminal_bridge_cleanup();
    g_terminal_client_socket = -1;
    task_printf("Terminal WebSocket: Connection handler terminated\r\n");
}

// 터미널 브리지 함수들은 terminal_bridge.c에서 구현됨