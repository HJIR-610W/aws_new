
#ifndef KEYBOARD_KEY_H
#define KEYBOARD_KEY_H

#include <stdint.h>

/*
ASCII 제어 문자 표 
Hex	Dec	Abbr	Name(영문)	설명	사용 예시 
0x00   0 NUL Null 문자열 종료 등 제어용 C언어 문자열 종료 '\0' 
0x01   1 SOH Start of Header 헤더 시작 프로토콜 헤더 시작 표시
0x02   2 STX Start of Text 본문 시작 텍스트 데이터 시작 구분 
0x03   3 ETX End of Text 본문 종료 텍스트 데이터 끝 표시 
0x04   4 EOT End of Transmission 전송 종료 파일 전송 완료 알림 
0x05   5 ENQ Enquiry 상태 요청(질의)프린터 상태 질의 
0x06   6 ACK Acknowledge 수신 확인 응답 UART ACK 응답 
0x07   7 BEL Bell 벨(소리) 터미널에서 소리 발생 
0x08   8 BS Backspace 백스페이스 텍스트 수정 시 한 글자 삭제 
0x09   9 HT Horizontal Tab 가로 탭 코드 정렬(탭) 
0x0A  10 LF Line Feed 줄바꿈(리눅스 / 유닉스) '\n'(Linux 줄바꿈)
0x0B  11 VT Vertical Tab 세로 탭 텍스트 포맷 구분
0x0C  12 FF Form Feed 페이지 넘김 프린터 새 페이지 시작
0x0D  13 CR Carriage Return 캐리지 리턴(윈도우 CR + LF) '\r\n'(Windows 줄바꿈)
0x0E  14 SO Shift Out 문자 세트 전환 터미널 글꼴 전환
0x0F  15 SI Shift In 문자 세트 복귀 터미널 글꼴 복귀
0x10  16 DLE Data Link Escape 데이터 링크 제어 전송 제어 신호 보호
0x11  17 DC1 Device Control 1 장치 제어(XON)소프트웨어 플로우 제어 시작 
0x12  18 DC2 Device Control 2 장치 제어 프린터 제어 
0x13  19 DC3 Device Control 3 장치 제어(XOFF)	소프트웨어 플로우 제어 중지
0x14	20	DC4	Device Control 4	장치 제어	특수 장치 모드 전환
0x15	21	NAK	Negative Acknowledge	부정 응답	UART NAK 응답
0x16	22	SYN	Synchronous Idle	동기 유휴	동기화 유지 패턴
0x17	23	ETB	End of Transmission Block	블록 전송 종료	블록 단위 전송 마침
0x18	24	CAN	Cancel	취소	전송 취소 명령
0x19	25	EM	End of Medium	매체 종료	저장 매체 끝 표시
0x1A	26	SUB	Substitute	대체 문자	손상된 문자 대체
0x1B	27	ESC	Escape	이스케이프	ANSI 이스케이프 시퀀스 시작 (\x1B[)
0x1C	28	FS	File Separator	파일 구분	데이터 파일 구분자
0x1D	29	GS	Group Separator	그룹 구분	데이터 그룹 구분자
0x1E	30	RS	Record Separator	레코드 구분	데이터 레코드 구분자
0x1F	31	US	Unit Separator	단위 구분	데이터 필드 구분자
0x7F	127	DEL	Delete	삭제 제어	터미널에서 문자 삭제
*/
typedef enum
{
  KEY_CODE_NONE = 0,

  KEY_CODE_ENTER = '\n',
  KEY_CODE_BACKSPACE = 0x08,
  KEY_CODE_TAB = '\t',
  KEY_CODE_ESC = 0x1B,
  KEY_CODE_SPACE = ' ',
  KEY_CODE_UP = 1000,
  KEY_CODE_DOWN,
  KEY_CODE_LEFT,
  KEY_CODE_RIGHT,
  KEY_CODE_HOME,
  KEY_CODE_END,
  KEY_CODE_INSERT,
  KEY_CODE_DELETE,
  KEY_CODE_PAGEUP,
  KEY_CODE_PAGEDOWN,

  KEY_CODE_CTRL_A = 0x01,
  KEY_CODE_CTRL_B = 0x02,
  KEY_CODE_CTRL_C = 0x03,
  KEY_CODE_CTRL_D = 0x04,
  KEY_CODE_CTRL_E = 0x05,
  KEY_CODE_CTRL_F = 0x06,
  KEY_CODE_CTRL_G = 0x07,
  KEY_CODE_CTRL_H = 0x08,
  KEY_CODE_CTRL_I = 0x09,
  KEY_CODE_CTRL_J = 0x0A,
  KEY_CODE_CTRL_K = 0x0B,
  KEY_CODE_CTRL_L = 0x0C,
  KEY_CODE_CTRL_M = 0x0D,
  KEY_CODE_CTRL_N = 0x0E,
  KEY_CODE_CTRL_O = 0x0F,
  KEY_CODE_CTRL_P = 0x10,
  KEY_CODE_ENTER_LONG = 0x10,
  KEY_CODE_CTRL_Q = 0x11,
  KEY_CODE_ESC_LONG = 0x11,
  KEY_CODE_CTRL_R = 0x12,
  KEY_CODE_CTRL_S = 0x13,
  KEY_CODE_CTRL_T = 0x14,
  KEY_CODE_CTRL_U = 0x15,
  KEY_CODE_CTRL_V = 0x16,
  KEY_CODE_CTRL_W = 0x17,
  KEY_CODE_CTRL_X = 0x18,
  KEY_CODE_CTRL_Y = 0x19,
  KEY_CODE_CTRL_Z = 0x1A,

  KEY_CODE_UNKNOWN = -1

} keycode_t;



#endif
