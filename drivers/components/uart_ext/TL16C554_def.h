

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
#endif