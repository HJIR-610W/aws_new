
#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "sockets.h"





extern unsigned char favicon_data[4286];
extern uint32_t parse_argsWithToken(char* str, char* argv[10], char token);
extern void mbedtls_sha1( const unsigned char *input,
                   size_t ilen,
                   unsigned char output[20] );

extern int mbedtls_base64_encode( unsigned char *dst, size_t dlen, size_t *olen,
                   const unsigned char *src, size_t slen );
const char *html_source1 =   "<!DOCTYPE html>"
    "<html lang=\"en\">"
    "<head>"
    "  <meta charset=\"UTF-8\">"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "  <title>Web Console with File Upload</title>"
    "  <style>"
    "    body {"
    "      font-family: monospace;"
    "      background-color: #f4f4f4;"
    "      margin: 0;"
    "      padding: 0;"
    "    }"
    "    #console {"
    "      width: 100%;"
    "      height: 70vh;"
    "      border: 1px solid #ccc;"
    "      background: #fff;"
    "      overflow-y: auto;"
    "      padding: 10px;"
    "      box-sizing: border-box;"
    "    }"
    "    #input-container {"
    "      width: 100%;"
    "      display: flex;"
    "      flex-direction: column;"
    "      border-top: 1px solid #ccc;"
    "      background: #eee;"
    "      padding: 10px;"
    "      box-sizing: border-box;"
    "    }"
    "    #input {"
    "      width: 100%;"
    "      padding: 10px;"
    "      border: 1px solid #ccc;"
    "      margin-bottom: 10px;"
    "      font-size: 16px;"
    "    }"
    "    #file-container {"
    "      display: flex;"
    "      align-items: center;"
    "      margin-bottom: 10px;"
    "    }"
    "    #file-input {"
    "      flex: 1;"
    "    }"
    "    #send-file {"
    "      padding: 10px;"
    "      background: #007BFF;"
    "      color: #fff;"
    "      border: none;"
    "      cursor: pointer;"
    "      margin-left: 10px;"
    "    }"
    "    #progress-bar {"
    "      width: 100%;"
    "      height: 20px;"
    "      margin-top: 10px;"
    "    }"
    "    .user-message {"
    "      color: green;" /* 사용자 입력을 녹색으로 표시 */
    "    }"
    "  </style>"
    "</head>"
    "<body>"
    "  <div id=\"console\"></div>"
    "  <div id=\"input-container\">"
    "    <input type=\"text\" id=\"input\" placeholder=\"Type your command and press Enter\">"
    "    <div id=\"file-container\">"
    "      <input type=\"file\" id=\"file-input\">"
    "      <button id=\"send-file\">Send File</button>"
    "    </div>"
    "    <progress id=\"progress-bar\" value=\"0\" max=\"100\"></progress>" /* Progress Bar 추가 */
    "  </div>"
    "  <script>"
    "    const consoleDiv = document.getElementById('console');"
    "    const inputField = document.getElementById('input');"
    "    const fileInput = document.getElementById('file-input');"
    "    const sendFileButton = document.getElementById('send-file');"
    "    const progressBar = document.getElementById('progress-bar');";

const char *html_source2= "    socket.onopen = () => {"
    "      appendToConsole('Connected to server.');"
    "    };"
    ""
    "    socket.onmessage = (event) => {"
    "      appendToConsole('HJIR-510: ' + event.data);"
    "    };"
    ""
    "    socket.onerror = (error) => {"
    "      appendToConsole('Error: Unable to connect to server.');"
    "    };"
    ""
    "    socket.onclose = () => {"
    "      appendToConsole('Disconnected from server.');"
    "    };"
    ""
    "    inputField.addEventListener('keypress', (event) => {"
    "      if (event.key === 'Enter' && inputField.value.trim() !== '') {"
    "        const command = inputField.value.trim();"
    "        appendToConsole('<span class=\"user-message\">USER: ' + command + '</span>');" // 녹색 스타일 적용"
    "        socket.send(command);"
    "        inputField.value = '';"
    "      }"
    "    });"
    ""
    "    sendFileButton.addEventListener('click', () => {"
    "      const userInput = prompt('Enter the access code:');"
    "      if (userInput !== '7777') {"
    "        appendToConsole('Access denied: Invalid code.');"
    "        return;"
    "      }"
    ""
    "      const file = fileInput.files[0];"
    "      if (!file) {"
    "        appendToConsole('No file selected.');"
    "        return;"
    "      }"
    ""
    "      const reader = new FileReader();"
    ""
    "      reader.onloadstart = () => {"
    "        progressBar.value = 0;"
    "        progressBar.style.display = 'block';"
    "      };"
    ""
    "      reader.onprogress = (event) => {"
    "        if (event.lengthComputable) {"
    "          const percent = Math.round((event.loaded / event.total) * 100);"
    "          progressBar.value = percent;"
    "        }"
    "      };"
    ""
    "      reader.onload = () => {"
    "        const fileData = reader.result;"
    "        socket.send(fileData);"
    "        appendToConsole('File \"' + file.name + '\" sent.');"
    "        progressBar.value = 100;"
    "      };"
    ""
    "      reader.onloadend = () => {"
    "        setTimeout(() => {"
    "          progressBar.style.display = 'none';"
    "        }, 1000);"
    "      };"
    ""
    "      reader.readAsArrayBuffer(file);"
    "    });"
    ""
    "    function appendToConsole(message) {"
    "      const messageDiv = document.createElement('div');"
    "      messageDiv.innerHTML  = message;" // HTML 태그 해석"
    "      consoleDiv.appendChild(messageDiv);"
    "      consoleDiv.scrollTop = consoleDiv.scrollHeight;"
    "    }"
    "  </script>"
    "</body>"
    "</html>";



const char *websocket_handshake_response =
    "HTTP/1.1 101 Switching Protocols\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Sec-WebSocket-Accept: %s\r\n"
    "\r\n";


typedef enum opcode_e
{
  eOPCODE_CONTINUATION,//계속 프레임
  eOPCODE_TEXT_FRAME,
  eOPCODE_BINARY_FRAME,
  eOPCODE_CLOSE_FRAME,
  eOPCODE_PING_FRAME,
  eOPCODE_PONG_FRAME
}eOPCODE_t;


// 흰색 16x16 ICO 파일 데이터
const uint8_t favicon_ico[] = {
    0x00, 0x00, 0x01, 0x00, // ICONDIR: Reserved, Type, Count
    0x01, 0x00, 0x10, 0x10, // ICONDIR: Width, Height
    0x00, 0x00, 0x01, 0x00, // ICONDIR: Color Planes, BPP
    0x20, 0x00, 0x68, 0x00, // ICONDIR: Size, Offset
    0x28, 0x00, 0x00, 0x00, // BITMAPINFOHEADER: Header size
    0x10, 0x00, 0x00, 0x00, // BITMAPINFOHEADER: Width
    0x20, 0x00, 0x00, 0x00, // BITMAPINFOHEADER: Height (16x2)
    0x01, 0x00, 0x20, 0x00, // BITMAPINFOHEADER: Planes, BPP
    0x00, 0x00, 0x00, 0x00, // Compression
    0x40, 0x00, 0x00, 0x00, // SizeImage
    0x13, 0x0B, 0x00, 0x00, // XPelsPerMeter
    0x13, 0x0B, 0x00, 0x00, // YPelsPerMeter
    0x00, 0x00, 0x00, 0x00, // ColorsUsed
    0x00, 0x00, 0x00, 0x00, // ColorsImportant
    // Pixel Data (16x16, 32-bit RGBA)
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, // Row 1
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
    // (Repeat rows for simplicity)
};




void send_icon(int conn)
{
  char buff[256];
  
  snprintf(buff,sizeof(buff),"HTTP/1.1 200 OK\r\nContent-Type: image/x-icon\r\nConnection: close\r\nContent-Length: %d\r\n\r\n",sizeof(favicon_data));

      send(conn, buff, strlen(buff),0);
  send(conn, favicon_data, sizeof(favicon_data),0);
}

void send_html(int conn)
{
  char buff[256];
  char ipBuff[100];
  int len;

  snprintf(ipBuff,sizeof(ipBuff),"const socket = new WebSocket('ws://%d.%d.%d.%d:8080');",
  ConfigRTU.ethConfig.local_ip[0],ConfigRTU.ethConfig.local_ip[1],
  ConfigRTU.ethConfig.local_ip[2],ConfigRTU.ethConfig.local_ip[3]);


  len = strlen(ipBuff) +strlen(html_source1)+strlen(html_source2);

  snprintf(buff,sizeof(buff),"HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=UTF-8\r\nConnection: close\r\nContent-Length: %d\r\n\r\n",len);

  send(conn, buff, strlen(buff),0);
  send(conn, html_source1, strlen(html_source1),0);
  send(conn, ipBuff, strlen(ipBuff),0);
  send(conn, html_source2, strlen(html_source2),0);
}

void generate_websocket_accept_key(const char *key, char *accept_key) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", key);

    unsigned char sha1_hash[20];
    mbedtls_sha1((unsigned char *)buffer, strlen(buffer), sha1_hash);

    size_t olen = 0;
    mbedtls_base64_encode((unsigned char *)accept_key, 64, &olen, sha1_hash, 20);
}


int extract_sec_websocket_key(const char *request, char *key_buffer, size_t buffer_size) {
    const char *key_start = strstr(request, "Sec-WebSocket-Key: ");
    if (!key_start) {
        // 키워드가 없을 경우
        return -1;
    }

    // "Sec-WebSocket-Key: "의 길이 (키 값의 시작 위치로 이동)
    key_start += strlen("Sec-WebSocket-Key: ");

    // 키의 끝을 찾기 (CRLF 기준)
    const char *key_end = strstr(key_start, "\r\n");
    if (!key_end) {
        // 끝을 찾지 못했을 경우
        return -1;
    }

    // 키의 길이 계산
    size_t key_length = key_end - key_start;

    // 버퍼 크기 검사
    if (key_length >= buffer_size) {
        // 버퍼가 너무 작아서 키를 복사할 수 없음
        return -1;
    }

    // 키 값을 버퍼로 복사
    strncpy(key_buffer, key_start, key_length);
    key_buffer[key_length] = '\0'; // null-terminate

    return 0; // 성공
}




#define SEND_DATA_BUFF_SIZE 1024

void websocket_send_data(int conn, const char *message) {

    char buffer[SEND_DATA_BUFF_SIZE]; // 고정 크기 버퍼
    size_t message_len = strlen(message);
    size_t header_len = 2; // 기본 헤더 길이

    size_t sent_bytes = 0; // 이미 보낸 바이트 수

    while (sent_bytes < message_len) {
        size_t remaining = message_len - sent_bytes; // 남은 데이터 크기
        size_t payload_size = (remaining > (SEND_DATA_BUFF_SIZE - 10)) ? (SEND_DATA_BUFF_SIZE - 10) : remaining;

        header_len = 2; // 기본 헤더 초기화
        buffer[0] = (sent_bytes + payload_size < message_len) ? 0x01 : 0x81; // FIN 플래그 설정

        // Payload Length 처리
        if (payload_size <= 125) {
            buffer[1] = payload_size; // 7비트 길이
        } else if (payload_size <= 65535) {
            buffer[1] = 126; // 2바이트로 길이를 표시
            buffer[2] = (payload_size >> 8) & 0xFF; // 상위 바이트
            buffer[3] = payload_size & 0xFF;        // 하위 바이트
            header_len = 4; // 2바이트 추가됨
        } else {
            buffer[1] = 127; // 8바이트로 길이를 표시
            for (int i = 0; i < 8; i++) {
                buffer[2 + i] = (payload_size >> (8 * (7 - i))) & 0xFF; // 8바이트 길이 설정
            }
            header_len = 10; // 8바이트 추가됨
        }

        // 메시지 복사
        memcpy(&buffer[header_len], &message[sent_bytes], payload_size);

        // 데이터 전송
        size_t total_len = header_len + payload_size;
        ssize_t len = send(conn, buffer, total_len, 0);

        if (len < 0) {
            //perror("WebSocket send failed");
            return;
        }

        sent_bytes += payload_size; // 보낸 바이트 누적
    }
}


void handle_websocket_handshake(int conn, const char *request) 
{
  char key_start[50];
  char accept_key[64] = {0};
  char response[256];

  extract_sec_websocket_key(request,key_start,sizeof(key_start));

  generate_websocket_accept_key(key_start, accept_key);


  snprintf(response, sizeof(response), websocket_handshake_response, accept_key);
  send(conn, response, strlen(response),0);
}

int decode_websocket_frame(int conn, uint8_t *output, size_t outputSize, size_t *payload_length,uint8_t *op)
{
  uint8_t header[14];    // WebSocket 헤더 최대 크기 (최대 14바이트)
  size_t header_length = 0;

  uint8_t fin ;
  uint8_t opcode = 0; // Opcode
  uint8_t mask = 0;
  size_t length = 0;
  size_t offset ;
  size_t total_payload_len=0;
  uint8_t init=1;
  do
  {
    // 1. 헤더 수신
    int ret = recv(conn, &header[0], 2,0);
    if (ret <= 0)
    {
      return -1;  // 오류 발생
    }
    header_length += ret;

    // 2. Parse Header
    fin     = header[0] & 0x80;
    opcode  = header[0] & 0x0F; // Opcode
    mask    = header[1] & 0x80;
    length  = header[1] & 0x7F;
    offset = 2;

    if(init)
    {
      init=0;
    *op = opcode;
    }
    
    // Close Frame 처리
    if (opcode == 0x8) 
    {  // Close Frame
       return -1;  // 연결 종료를 나타냄
    }

    // 3. Extended Payload Length 처리
    if (length == 126)
    {
       int ret = recv(conn, &header[2], 2,0);
            if (ret <= 0) {
              //  perror("Socket read error (extended payload length)");
                return -1;
            }
            header_length += ret;

        length = (header[2] << 8) | header[3];
        offset += 2;
    } 
    else if (length == 127)
    {
      int ret = recv(conn, &header[2], 8,0);
      if (ret <= 0)
      {
        return -1;
      }
      header_length += ret;

        length = 0;
        for (int i = 0; i < 8; i++)
        {
            length = (length << 8) | header[offset + i];
        }
        offset += 8;
    }



    // 4. Masking Key 처리
    uint8_t mask_key[4] = {0};
    if (mask)
    {
        int ret = recv(conn, &header[header_length], 4,0);
        if (ret <= 0) {
          //  perror("Socket read error (masking key)");
            return -1;
        }


        memcpy(mask_key, &header[header_length], 4);
        offset += 4;
    }

    // 5. Payload 데이터 수신 (크기 제한 적용)
    size_t total_received = 0;
    size_t to_read = length > outputSize ? outputSize : length;

    {
      
    }
while (total_received < to_read) {
    // 읽어야 할 크기 계산
    size_t chunk_size = (to_read - total_received) > 1024 ? 1024 : (to_read - total_received);

    // 데이터를 읽음
    int ret = recv(conn, &output[total_payload_len+total_received], chunk_size, 0);
    if (ret <= 0) {
        // 소켓 오류 처리
      //  perror("Socket read error (payload)");
        return -1;  // 데이터 수신 실패
    }

    total_received += ret; // 수신된 바이트를 누적
}


    // 추가 데이터를 무시하고 -2 반환
    if (length > outputSize) {
        size_t remaining = length - outputSize;
        uint8_t discard_buffer[256];
        while (remaining > 0) {
            size_t chunk = remaining > sizeof(discard_buffer) ? sizeof(discard_buffer) : remaining;
            int ret = recv(conn, discard_buffer, chunk,0);
            if (ret <= 0) {
              //  perror("Socket read error (discard remaining payload)");
                return -1;
            }
            remaining -= ret;
        }
        return -2; // 데이터 초과 발생
    }

    // 6. 마스킹 해제
    if (mask)
    {
        for (size_t i = 0; i < to_read; i++)
        {
          output[total_payload_len+i] ^= mask_key[i % 4];
        }
    }
        total_payload_len += length;
  }while(fin!=0x80);//마지막 프레임이 아니면계속속

*payload_length = total_payload_len;
    return 0;  // 성공
}


#define WEBSOCKET_BUFF_SIZE 2*1024*1024
#define OUT_BUFF_SIZE 4096
void handle_websocket_connection(int conn) 
{

  size_t length;
  int ret;
  char *payLoad = EwAlloc(WEBSOCKET_BUFF_SIZE);
  char *outBuff = EwAlloc(OUT_BUFF_SIZE);
  uint8_t opcode;
void (*cmd_func)(char *,char *,char *,uint32_t );
char *inputList[3]={0,0,0};

  while(payLoad)
  {
    memset(payLoad,0,WEBSOCKET_BUFF_SIZE);
    ret = decode_websocket_frame(conn,(uint8_t *)payLoad,WEBSOCKET_BUFF_SIZE,&length,&opcode);

    if(ret == -1)
    {

      EwFree(payLoad);
      EwFree(outBuff);
      return;
    }
    else if(ret ==0)
    {
      switch(opcode)
      {
        case 0x02://파일
        if(length <= UPLOAD_FILE_SIZE)
        {
          if(g_fwBuff==NULL)
          {
            g_fwBuff = EwAlloc(UPLOAD_FILE_SIZE);
          }
          memcpy(g_fwBuff,payLoad,length);
          g_fwLen = length;

          websocket_send_data(conn, "File has been successfully uploaded.");
        }
        else
        {
          websocket_send_data(conn, "The file size exceeds the allocated memory.");
        }
        break;
        case 0x01://문자


        parse_argsWithToken(payLoad,inputList,' ');
   

        if(strncmp(inputList[0],"get",3)==0)
        {
          cmd_func = cli_cmd_get;
        }
        else{
          cmd_func = cli_cmd_post;
        }


              cmd_func(inputList[1],inputList[2],outBuff,OUT_BUFF_SIZE);

                websocket_send_data(conn, outBuff);
                if(g_767UpdateReq==true)
                {
                  osDelay(2000);// tcp데이터 전송 지연, tcp옵션처리 검토
                  closesocket(conn);
                  update_frimware767();
                }


        break;
      }
    }
        

  }



}


