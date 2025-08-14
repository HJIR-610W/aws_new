

#ifndef TL16C554_DEF_H
#define TL16C554_DEF_H

#define UART_CLOCK_FREQ 3686400

#define DLAB_BIT 0x80 // LCR 레지스터의 DLAB 비트
#define LSR_DR 0x01   // Data Ready 비트
#define LSR_THRE 0x20 // Transmitter Holding Register Empty 비트
#define LSR_TEMT 0x40



#define RBR(BASE) (void *)(BASE + 0x00) // Transmitter Holding Register
#define THR(BASE) (void *)(BASE + 0x00) // Transmitter Holding Register
#define DLL(BASE) (void *)(BASE + 0x00) // Divisor Latch Low
#define DLM(BASE) (void *)(BASE + 0x01) // Divisor Latch High
#define IER(BASE) (void *)(BASE + 0x01)
#define FCR(BASE) (void *)(BASE + 0x02)
#define IIR(BASE) (void *)(BASE + 0x02)
#define LCR(BASE) (void *)(BASE + 0x03)
#define MCR(BASE) (void *)(BASE + 0x04)
#define LSR(BASE) (void *)(BASE + 0x05) // 라인상태 레지스터터
#define MSR(BASE) (void *)(BASE + 0x06)
#define SCR(BASE) (void *)(BASE + 0x07)

// 레지스터 오프셋
#define DLL_OFFSET 0x00 // Divisor Latch Low
#define DLM_OFFSET 0x01 // Divisor Latch High
#define LCR_OFFSET 0x03 // Line Control Register
#define FCR_OFFSET 0x02 // FIFO Control Register
#define MCR_OFFSET 0x04 // Modem Control Register

//IIR 레지스터 bit 필트
#define UART_IIR_INTTERUPT_ID_MASK 0x0E

#define UART_IIR_INTTERUPT_PENDING 0x01

#define UART_IIR_RX_LINE_STAT  0x06  // 수신 라인 상태 변경 발생(OE, PE, FE, BI)
#define UART_IIR_RX_DATA_AVAILABLE 0x04 // 수신 데이터 사용 가능 (FIFO 모드에서 트리거 레벨 도달)
#define UART_IIR_CHAR_TIMEOUT 0x0c 
/*
문자 타임아웃 발생
수신기 FIFO에서 지난 4문자 시간 동안 어떠한 문자도 제거되거나 입력되지 않았으며
이 시간 동안 최소한 하나 이상의 문자가 FIFO에 존재하고 있다.
*/
#define UART_IIR_THRE 0x02          // 송신기 홀딩 레지스터 비어 있음 (THRE)
#define UART_IIR_MODEM_STATUS 0x00  // 모뎀 상태 변화 (CTS, DSR, RI, DCD)


#endif