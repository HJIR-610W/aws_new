
#ifndef WEBSOCKET_H
#define WEBSOCKET_H

typedef enum opcode_e
{
  eOPCODE_CONTINUATION=0,//계속 프레임
  eOPCODE_TEXT_FRAME,    //텍스트트
  eOPCODE_BINARY_FRAME,  //바이너리
  eOPCODE_CLOSE_FRAME,   //연결 닫기
  eOPCODE_PING_FRAME,    //핑
  eOPCODE_PONG_FRAME     //퐁
}eOPCODE_t;

#endif