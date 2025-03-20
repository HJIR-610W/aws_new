
#ifndef DRIVER_MODBUS_MASTER_H
#define DRIVER_MODBUS_MASTER_H

#include "driver_interface.h"

/*

프로토콜      물리 계층       데이터 형식      오류 검사 방식         주요 장점
Modbus RTU    RS232, RS485    바이너리(Binary) CRC-16                 데이터 전송 효율이 높음
Modbus ASCII  RS232, RS485    ASCII(텍스트)    LR                     사람이 직접 읽을 수 있음
Modbus TCP/IP Ethernet (TCP)  바이너리(Binary) TCP/IP 자체 오류검사   네트워크에서 사용 가능
Modbus UDP    Ethernet (UDP)  바이너리(Binary) 없음 (UDP 신뢰성 없음) 속도가 빠름
Modbus Secure Ethernet (TLS)  바이너리(Binary) TLS 보안 적용          보안 강화
Modbus Plus   전용 네트워크   전용 포맷 전용   오류검사               Schneider 전용

*/

#define DRIVER_MODBUS_RTU_OVER_485     0
#define DRIVER_MODBUS_ASCII_OVER_485   1
#define DRIVER_MODBUS_TCP              2

driver_t * driver_modbus_open(int32_t num,void *opt);
#endif