
#ifndef HTTP_FW_H
#define HTTP_FW_H


#define UPLOAD_FILE_SIZE 2097152

extern uint8_t *g_fwBuff;
extern uint32_t g_fwLen;


uint8_t check_firmwareFile(void);


#endif