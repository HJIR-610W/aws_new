

#include "TL16C554.h"
#include "TL16C554_def.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "dev_io.h"
#include "bsp_di.h"
#include "os_user_def.h"
#include "pcb_define.h"
#include "stream_buffer.h"
#include "system_err.h"
#include "util_memory.h"

#define STREAMBUFFER_USE 1  // 데이터 수신을 freertos 스트림 버퍼 사용시

#define QUAD_1_BUFF_SIZE 200  // D_SUB
#define QUAD_2_BUFF_SIZE 100  // TTL
#define QUAD_3_BUFF_SIZE 100  // EXT3
#define QUAD_4_BUFF_SIZE 100  // EXT4
#define QUAD_5_BUFF_SIZE 100  // RS485_A
#define QUAD_6_BUFF_SIZE 100  // RS485_B
#define QUAD_7_BUFF_SIZE 100  // RS232_C
#define QUAD_8_BUFF_SIZE 100  // EXT2


typedef struct tl16c554_instance_s
{
  int irq_di_num; //uart 수신 입터럽트 번호 
  uint32_t baud;//tx 시간계산시 필요
  uint8_t parityIdx;
  bool opened;
  volatile uint8_t *base_address;
  StreamBufferHandle_t quad_stream;
  void *tx_sem;//송신용 sem 일반적인 app에서는 tx_sem만 있어도됨
  void *rx_sem;//수신용 sem
} tl16c554_instance_t;

static tl16c554_instance_t tl16c554_inst[TL16C554_UART_MAX] = {
    [TL16C554_UART_1_D_SUB] = {.irq_di_num = BSP_DI_QUAD_UARTA_1, .base_address = (volatile uint8_t *)0x68000000},
    [TL16C554_UART_2_TTL_TTL] = {.irq_di_num = BSP_DI_QUAD_UARTB_2, .base_address = (volatile uint8_t *)0x68000010},
    [TL16C554_UART_3_RS232_A] = {.irq_di_num = BSP_DI_QUAD_UARTC_3, .base_address = (volatile uint8_t *)0x68000020},
    [TL16C554_UART_4_RS232_B] = {.irq_di_num = BSP_DI_QUAD_UARTD_4, .base_address = (volatile uint8_t *)0x68000030},
    [TL16C554_UART_5_RS485_A] = {.irq_di_num = BSP_DI_QUAD_UARTA_5, .base_address = (volatile uint8_t *)0x68000040},
    [TL16C554_UART_6_RS485_B] = {.irq_di_num = BSP_DI_QUAD_UARTB_6, .base_address = (volatile uint8_t *)0x68000050},
    [TL16C554_UART_7_RS232_C] = {.irq_di_num = BSP_DI_QUAD_UARTC_7, .base_address = (volatile uint8_t *)0x68000060},
    [TL16C554_UART_8_RS232_D] = {.irq_di_num = BSP_DI_QUAD_UARTD_8, .base_address = (volatile uint8_t *)0x68000070}};

static const uint8_t buff_size_list[TL16C554_UART_MAX] = {
    QUAD_1_BUFF_SIZE, QUAD_2_BUFF_SIZE, QUAD_3_BUFF_SIZE, QUAD_4_BUFF_SIZE,
    QUAD_5_BUFF_SIZE, QUAD_6_BUFF_SIZE, QUAD_7_BUFF_SIZE, QUAD_8_BUFF_SIZE};


void irq_INTA_1(int32_t arg);
void irq_INTB_2(int32_t  arg);
void irq_INTC_3(int32_t  arg);
void irq_INTD_4(int32_t  arg);
void irq_INTA_5(int32_t  arg);
void irq_INTB_6(int32_t  arg);
void irq_INTC_7(int32_t  arg);
void irq_INTD_8(int32_t  arg);


static inline uint8_t read_register(void *addr)
{
  uint8_t data;

  data = *((volatile uint8_t *)addr);

  return data;                                        
}

static inline void write_register(void *addr, uint8_t value)
{ 
  *((volatile uint8_t *)addr) = value;
}

// 보오드레이트 확인 함수
void check_baud_rate(int uart_num)
{
  uint8_t data;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];
  data = read_register(LCR(uart->base_address));
  // LCR의 DLAB 비트를 1로 설정하여 DLL 및 DLM 접근 허용

  data |= 0x80;  // DLAB 비트 설정

  write_register(LCR(uart->base_address), data);

  uint8_t dll_value = read_register(DLL(uart->base_address));
  uint8_t dlm_value = read_register(DLM(uart->base_address));

  data = read_register(LCR(uart->base_address));

  // DLAB 비트를 다시 0으로 설정하여 DLL 및 DLM 접근 비허용
  data &= ~0x80;

  write_register(LCR(uart->base_address), data);

  // Divisor 계산 (DLM은 상위 바이트, DLL은 하위 바이트)
  uint16_t divisor = (dlm_value << 8) | dll_value;

  if (divisor == 0)
  {
    io_printf("Invalid divisor value.\n");
    return;
  }

  // 보오드레이트 계산
  uint32_t baud_rate = UART_CLOCK_FREQ / (16 * divisor);
  io_printf("Calculated Baud Rate: %u\n", baud_rate);
}

// 보오드레이트 설정 함수
void set_baud_rate(int uart_num, uint32_t baud_rate)
{
  uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate);
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];


  // DLAB 비트를 1로 설정하여 DLL과 DLM에 접근 가능하게 함
  uint8_t lcr_value = read_register(LCR(uart->base_address));
  write_register(LCR(uart->base_address), lcr_value | DLAB_BIT);

  // Divisor 값 설정
  write_register(DLL(uart->base_address),
                 divisor & 0xFF); // 하위 바이트 설정
  write_register(DLM(uart->base_address),
                 (divisor >> 8) & 0xFF); // 상위 바이트 설정

  // DLAB 비트를 0으로 다시 설정하여 DLL과 DLM 접근 비허용
  write_register(LCR(uart->base_address), lcr_value & ~DLAB_BIT);
}

#define PEN (1 << 3)  // 패리티 활성화 비트
#define EPS (1 << 4)  // 짝수 패리티 비트
#define SP (1 << 5)   // 강제 패리티 비트
#define STB (1 << 2)  // Stop bit 설정 비트
// 패리티 설정 함수
void set_parity(uint8_t uart_num, uint8_t parity_mode)
{
  volatile uint8_t lcr;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];


  lcr = read_register(LCR(uart->base_address));
  uint8_t lcr_val = lcr & 0xC7;  // LCR에서 parity 관련 비트(3~5)만 초기화

  switch (parity_mode)
  {
    case 0:  // No Parity
      lcr_val &= ~PEN;
      break;
    case 1:  // Odd Parity
      lcr_val |= PEN;
      lcr_val &= ~EPS;
      break;
    case 2:  // Even Parity
      lcr_val |= PEN | EPS;
      break;
    case 3:  // Forced Parity 1
      lcr_val |= PEN | SP;
      lcr_val &= ~EPS;
      break;
    case 4:  // Forced Parity 0
      lcr_val |= PEN | EPS | SP;
      break;
    default:
      return;  // 잘못된 입력값이면 무시
  }

  write_register(LCR(uart->base_address), lcr_val);


}

void set_stop_bit(uint8_t uart_num, uint8_t stop_bits)
{
  volatile uint8_t lcr;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];
  // 현재 LCR 레지스터 값 읽기
  lcr = read_register(LCR(uart->base_address));

  switch (stop_bits)
  {
    case 0:         // 1 stop bit
      lcr &= ~STB;  // STB 비트 클리어
      break;

    case 1:        // 1.5 stop bits (5-bit word) 또는 2 stop bits (6,7,8-bit word)
      lcr |= STB;  // STB 비트 설정
      break;

    default:
      return;  
  }


  write_register(LCR(uart->base_address), lcr);
}
void quad_init(int uart_num, void *opt)
{
  uint8_t parity_mode;
  uint8_t flag = 0;
  uint8_t g_reg;
  uart_config_t *config = opt;
  di_isr_set_cfg_t isr_cfg;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];
  int baud_rate = config->baud;



  uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate); // 보오드레이트 설정을 위한 Divisor 계산
  // DLAB 비트 설정 (LCR의 MSB 비트) 1로 해야 분주비 레지스터 접근 가능
  write_register(LCR(uart->base_address), 0x80);
  // DLL과 DLM에 divisor 값 설정
  write_register(DLL(uart->base_address), divisor & 0xFF);
  write_register(DLM(uart->base_address), (divisor >> 8) & 0xFF);
  // DLAB 비트를 0으로 설정하여 LCR 설정, 상태레지스터 접근 가능
  write_register(LCR(uart->base_address), 0x03);

  if (config->parityIdx == PARITY_NONE)
  {
    parity_mode = 0;
  }
  else if (config->parityIdx == PARITY_ODD)
  {
    parity_mode = 1;
  }
  else  // even
  {
    parity_mode = 2;
  }

  set_parity(uart_num, parity_mode);

  //FIFO 설정 (FCR) 트리거 레벨 1바이트
  write_register(FCR(uart->base_address), 0x07);  // FIFO enable, RX/TX FIFO reset
  // MCR 설정 (필요에 따라 추가 설정)
  write_register(MCR(uart->base_address), 0x08);

// 인터럽트 설정 Bit 3,2,1
#define IER_RDA 0x01           // 데이터가 수신됨
#define IER_THRE 0x02          // 송신버퍼 빈상태
#define IER_LINE_STATUS 0x04   // 라인상태 변경됨됨
#define IER_MODEM_STATUS 0x08  // 모뎀 상태 변경됨
#if STREAMBUFFER_USE
  flag = IER_RDA | IER_LINE_STATUS | IER_MODEM_STATUS;

  g_reg = read_register(IER(uart->base_address)) | (flag);
  
  write_register(IER(uart->base_address), g_reg);
#endif

  void (*isrTable[TL16C554_UART_MAX])(int32_t) = {irq_INTA_1, irq_INTB_2, irq_INTC_3, irq_INTD_4,
                                                  irq_INTA_5, irq_INTB_6, irq_INTC_7, irq_INTD_8};
  const char *isrNameTable[TL16C554_UART_MAX] = {
      TOSTRING(irq_INTB_1), TOSTRING(irq_INTB_2), TOSTRING(irq_INTC_3), TOSTRING(irq_INTD_4),
      TOSTRING(irq_INTA_5), TOSTRING(irq_INTB_6), TOSTRING(irq_INTC_7), TOSTRING(irq_INTD_8)};

  isr_cfg.call = isrTable[uart_num];
  isr_cfg.name = isrNameTable[uart_num];
  isr_cfg.trigger = eDI_RISING_FALLING;
  isr_cfg.prio = 6;
  isr_cfg.handle = uart_num;


  bsp_di_set_interrupt(uart->irq_di_num, &isr_cfg);
}

/**
 * @brief 1바이트 전송
 * @retval <0 오류,1 정상 전송
 */
int32_t quad_send_data(uint8_t uart_num, uint8_t data)
{
  uint32_t startTime;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  // 송신 버퍼가 비어있을 때까지 대기
  startTime = OS_GET_TICK();
  do
  {
    if ((OS_GET_TICK() - startTime) > 10)
    {
      return -1;
    }
  } while ((read_register(LSR(uart->base_address)) & LSR_THRE) == 0);

  // 데이터를 THR에 씁니다.
  write_register(THR(uart->base_address), data);

  return 1;
}

// 데이터 읽기 함수
int32_t read_byte(int uart_num, uint8_t *data)
{
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];
  // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
  if (read_register(LSR(uart->base_address)) & LSR_DR)
  {
    *data = read_register(RBR(uart->base_address));  // RBR에서 데이터 읽기
    return 1;                                        // 데이터 읽기 성공
  }
  else
  {
    return 0;  // 데이터가 준비되지 않음
  }
}

int32_t quad_recv_byte(int uart_num, uint8_t *data)
{
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  if (read_register(LSR(uart->base_address)) & LSR_DR)
  {
    *data = read_register(RBR(uart->base_address)); // RBR에서 데이터 읽기
    return 1;                                       
  }
  else
  {
    return 0;
  }
}

/**
 * @brief 타임아웃을주고 최소 1바이트 수신된 이후부터 dataTimeOutMs동안 데이터
 * 수신 못하면 종료 처리
 *
 * 예)modbus 활용
 */
uint16_t tl16c554_uart_recvsOpt(int uart_num, uint8_t *p_buff, uint16_t buffSize,
                                uint32_t waitTimeOutMs, uint32_t dataTimeOutMs)
{
#if STREAMBUFFER_USE  // 레지스터 직접 접근
  uint32_t starTick;
  uint32_t stopTick;
  uint32_t elapseTick;
  uint32_t timeout;
  // uint32_t lastTick=0;
  uint32_t waitTimeOut = 0;
  uint32_t startTime;
  size_t xBytesAvailable;
  size_t xBytesRead;
  size_t remainBuffSize = buffSize;
  size_t cnt = 0;
  uint8_t waitCnt = 0;
  bool once = true;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  (void)waitTimeOut;
  timeout = waitTimeOutMs;

  startTime = OS_GET_TICK();
  while (1)
  {
    /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
    xBytesAvailable = xStreamBufferBytesAvailable(uart->quad_stream);

    if (remainBuffSize < xBytesAvailable)
    {
      xBytesAvailable = remainBuffSize;  // 버퍼 수만큼만 읽기
    }

    starTick = OS_GET_TICK();
    if (xBytesAvailable > 0)
    {
      /* 데이터를 읽을 수 있다면, 데이터를 수신 */
      xBytesRead = xStreamBufferReceive(uart->quad_stream, (void *)&p_buff[cnt],
                                        xBytesAvailable, pdMS_TO_TICKS(timeout));

      if (xBytesRead > 0)
      {
        cnt += xBytesRead;
        waitCnt++;
      }
    }
    else
    {
      /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
      xBytesRead = xStreamBufferReceive(uart->quad_stream, (void *)&p_buff[cnt], 1,
                                        pdMS_TO_TICKS(timeout));
      if (xBytesRead == 1)
      {
        cnt += 1;
        waitCnt++;
      }
    }
    // 데이터가 하나라도 수신되기 전까지는 총 지연시간만큼 기다리고
    // 데이터가 하나라도 수신된 이후 부터는 데이터 타임아웃 만큼 기다림
    if (once && cnt)
    {
      once = false;
      timeout = dataTimeOutMs;
    }

    stopTick = OS_GET_TICK();
    elapseTick = stopTick - starTick;

    if (once)  // 데이터가 하나도 수신안되었으면 경과시간 확인
    {
      if (elapseTick >= timeout)
      {
        return 0;
      }
    }
    else  // 데이터가 하나라도 수신되었으면 최초부터
    {
      if ((stopTick - startTime) > waitTimeOutMs || cnt >= buffSize)
      {
        return cnt;
      }
    }

    if (waitCnt >= 2 && elapseTick > dataTimeOutMs)
    {
      return cnt;
    }

    remainBuffSize -= xBytesAvailable;
    timeout = timeout - elapseTick;
  }

#else
  uint32_t startTick;
  uint16_t cnt = 0;

  startTick = xTaskGetTickCount();
  while (1)
  {
    if (read_register(LSR(uart_base_adress[drv->num])) & LSR_DR)
    {
      p_buff[cnt++] = read_register(RBR(uart_base_adress[drv->num]));
    }
    if (cnt == buffSize)
    {
      break;
    }
    if ((xTaskGetTickCount() - startTick) > timeOutMs)
    {
      break;
    }
  }

  return cnt;  // 데이터가 준비되지 않음

#endif

  // return 0;
}

uint16_t tl16c554_uart_recvsOpt2(int uart_num, uint8_t *p_buff, uint16_t buffSize,
                                 uint32_t waitTimeOutMs, uint32_t dataTimeOutMs)
{
#if STREAMBUFFER_USE  // 레지스터 직접 접근
  uint32_t starTick;
  uint32_t stopTick;
  uint32_t elapseTick;
  uint32_t timeout;
  // uint32_t lastTick=0;
  //  uint32_t waitTimeOut;
  uint32_t startTime;
  size_t xBytesAvailable;
  size_t xBytesRead;
  size_t remainBuffSize = buffSize;
  size_t cnt = 0;
  uint8_t waitCnt = 0;
  bool once = true;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  timeout = waitTimeOutMs;

  startTime = OS_GET_TICK();
  while (1)
  {
    /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
    xBytesAvailable = xStreamBufferBytesAvailable(uart->quad_stream);

    if (remainBuffSize < xBytesAvailable)
    {
      xBytesAvailable = remainBuffSize;  // 버퍼 수만큼만 읽기
    }

    starTick = OS_GET_TICK();
    if (xBytesAvailable > 0)
    {
      /* 데이터를 읽을 수 있다면, 데이터를 수신 */
      xBytesRead = xStreamBufferReceive(uart->quad_stream, (void *)&p_buff[cnt],
                                        xBytesAvailable, pdMS_TO_TICKS(timeout));

      if (xBytesRead > 0)
      {
        cnt += xBytesRead;
        waitCnt++;
      }
    }
    else
    {
      /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
      xBytesRead = xStreamBufferReceive(uart->quad_stream, (void *)&p_buff[cnt], 1,
                                        pdMS_TO_TICKS(timeout));
      if (xBytesRead == 1)
      {
        cnt += 1;
        waitCnt++;
      }
    }
    // 데이터가 하나라도 수신되기 전까지는 총 지연시간만큼 기다리고
    // 데이터가 하나라도 수신된 이후 부터는 데이터 타임아웃 만큼 기다림
    if (once && cnt)
    {
      once = false;
      timeout = dataTimeOutMs;
    }

    stopTick = OS_GET_TICK();
    elapseTick = stopTick - starTick;

    if (once)  // 데이터가 하나도 수신안되었으면 경과시간 확인
    {
      if (elapseTick >= timeout)
      {
        return 0;
      }
    }
    else  // 데이터가 하나라도 수신되었으면 최초부터
    {
      if ((stopTick - startTime) > waitTimeOutMs || cnt >= buffSize)
      {
        return cnt;
      }
    }

    if (waitCnt >= 2 && elapseTick > dataTimeOutMs)
    {
      return cnt;
    }

    remainBuffSize -= xBytesAvailable;
    timeout = timeout - elapseTick;
  }

#else
  uint32_t startTick;
  uint16_t cnt = 0;

  startTick = xTaskGetTickCount();
  while (1)
  {
    if (read_register(LSR(uart_base_adress[drv->num])) & LSR_DR)
    {
      p_buff[cnt++] = read_register(RBR(uart_base_adress[drv->num]));
    }
    if (cnt == buffSize)
    {
      break;
    }
    if ((xTaskGetTickCount() - startTick) > timeOutMs)
    {
      break;
    }
  }

  return cnt;  // 데이터가 준비되지 않음

#endif

  // return 0;
}

#define UART_IIR_INTTERUPT_PENDING 0x01
#define UART_IIR_RX_LINE_STAT 0x06   // 수신 라인 상태 (OE, PE, FE, BI)
#define UART_IIR_RX_DATA_AVAIL 0x04  // 수신 데이터 사용 가능 (FIFO 모드에서 트리거 레벨 도달)
#define UART_IIR_CHAR_TIMEOUT 0x0c   // 문자 타임아웃 발생
#define UART_IIR_THRE 0x02           // 송신기 홀딩 레지스터 비어 있음 (THRE)
#define UART_IIR_MODEM_STATUS 0x00   // 모뎀 상태 변화 (CTS, DSR, RI, DCD)

int g_channel;
void irq_tl16c554(int uart_num)
{
  uint8_t iir;
  uint8_t interruptType;
  uint8_t data;
  uint8_t reg;
  uint8_t lineStatus;
  uint8_t modemStatus;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  size_t xBytesSent;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  while (((iir = read_register(IIR(uart->base_address))) &
          UART_IIR_INTTERUPT_PENDING) == 0)
  {
    interruptType = iir & 0x0F;

    switch (interruptType)
    {
      case UART_IIR_RX_DATA_AVAIL:                             
        data = read_register(RBR(uart->base_address));               // RBR에서 데이터 읽기

        /* 데이터를 스트림 버퍼에 전송 */
        xBytesSent = xStreamBufferSendFromISR(uart->quad_stream, &data, 1,
                                              &xHigherPriorityTaskWoken);
        /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

        if (!(xBytesSent > 0))
        {
          __asm("BKPT #0");
        }
        break;
      case UART_IIR_THRE:  // Transmitter Holding Register Empty
        //write_register(THR(uart->base_address), '1');
        break;
      case UART_IIR_RX_LINE_STAT:  
        lineStatus = read_register(LSR(uart->base_address)); // RBR에서 데이터 읽기
        (void)lineStatus;
        break;

      case UART_IIR_MODEM_STATUS:  
        modemStatus = read_register(MSR(uart->base_address)); // RBR에서 데이터 읽기
        (void)modemStatus;
        break;
      case UART_IIR_CHAR_TIMEOUT:
        reg = read_register(RBR(uart->base_address)); // RBR에서 데이터 읽기
        (void)reg;
        break;
        break;

      default:

        break;
    }
  }
}

void irq_INTA_1(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTB_2(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTC_3(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTD_4(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTA_5(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTB_6(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTC_7(int32_t arg) { irq_tl16c554((int)arg); }

void irq_INTD_8(int32_t arg) { irq_tl16c554((int)arg); }

// DMA 핸들러 선언
DMA_HandleTypeDef hdma_memtomem;

// DMA 전송 완료 콜백 함수
void HAL_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma)
{
  if (hdma->Instance == DMA2_Stream0)
  {                        // DMA 스트림 확인

  }
}
void HAL_DMA_XferErrorCallback(DMA_HandleTypeDef *hdma)
{ 

}

void tl16c554_send_DMA(int uart_num, const uint8_t *pData, uint16_t dataLen)
{
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  uint32_t dest_address = (uint32_t)THR(uart->base_address);
  // DMA 전송 시작
  if (HAL_DMA_Start_IT(&hdma_memtomem, (uint32_t)pData, dest_address, dataLen) != HAL_OK)
  {
    // DMA 시작 실패 처리
    io_printf("DMA Start Failed\n");
    while (1);
  }
}


void tl16c554_irq_init(int num, uint8_t prio)
{
  di_isr_set_cfg_t isr_cfg;


  void (*isrTable[TL16C554_UART_MAX])(int32_t) = {irq_INTA_1, irq_INTB_2, irq_INTC_3, irq_INTD_4,
                                                 irq_INTA_5, irq_INTB_6, irq_INTC_7, irq_INTD_8};
  const char *isrNameTable[TL16C554_UART_MAX] = {
      TOSTRING(irq_INTB_1), TOSTRING(irq_INTB_2), TOSTRING(irq_INTC_3), TOSTRING(irq_INTD_4),
      TOSTRING(irq_INTA_5), TOSTRING(irq_INTB_6), TOSTRING(irq_INTC_7), TOSTRING(irq_INTD_8)};

  isr_cfg.call = isrTable[num];
  isr_cfg.name = isrNameTable[num];
  isr_cfg.trigger = eDI_RISING;
  isr_cfg.prio = prio;
  isr_cfg.handle = num;

  bsp_di_set_interrupt(tl16c554_inst[num].irq_di_num, &isr_cfg);
}

/**
 * @brief 초기화
 * @param uart_num 초기화할 uart 번호
 * @param opt 초기화시 사용할 구조체 포인터
 */
int32_t tl16c554_init(int32_t uart_num, void *opt)
{
  uart_config_t *config = opt;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  if (uart->opened)
  {
    return 1;
  }

  uart->baud = config->baud;
  uart->parityIdx = config->parityIdx;
  uart->quad_stream = xStreamBufferCreate(buff_size_list[uart_num], 1);

  OS_CREATE_BINARY_SEM(uart->tx_sem);
  OS_CREATE_BINARY_SEM(uart->rx_sem);

  quad_init(uart_num, opt);

  uart->opened = true;

  return 1;
}

void tl16c554_close(int num)
{
  //구현 예정
}

int32_t tl16c554_send(int uart_num, const uint8_t *pData, uint16_t data_len)
{
  int32_t cnt = 0;
  uint32_t start_time;
  uint32_t timeout;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  OS_PEND_SEM(uart->tx_sem, osWaitForever);

  while (data_len)
  {
    data_len--;
    if (quad_send_data(uart_num, *pData++) == 1)
    {
      cnt++;
    }
  }

  
  start_time = OS_GET_TICK();
  timeout = (uint32_t)(((float)1 / uart->baud * 10) * data_len + 100);
  do
  {
    if ((OS_GET_TICK() - start_time) > timeout)
    {
      cnt = -1;
      break;
    }
  }
  while ((read_register(LSR(uart->base_address)) & LSR_TEMT) == 0);

  /*
  송신 레지스터 비어있음
  THR 및 TSR이 모두 비어있을 때 설정됨
  THR에 문자가 로드되면 LSR6은 클리어되며 문자가 완전히 송신될 때 까지 유지됨
  */

  OS_POST_SEM(uart->tx_sem);

  return cnt;
}

int32_t tl16c554_recv2(int uart_num, uint8_t *p_buff, uint16_t buffSize, uint32_t timeOutMs)
{

  uint32_t starTick;
  uint32_t stopTick;
  uint32_t elapseTick;
  uint32_t timeout;
  uint32_t lastTick = 0;
  size_t xBytesAvailable;
  size_t xBytesRead;
  size_t remainBuffSize = buffSize;
  size_t cnt = 0;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  timeout = timeOutMs;

  (void)lastTick;

  OS_PEND_SEM(uart->rx_sem, osWaitForever);

  while (1)
  {
    /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
    xBytesAvailable = xStreamBufferBytesAvailable(uart->quad_stream);

    if (remainBuffSize < xBytesAvailable)
    {
      xBytesAvailable = remainBuffSize;  // 버퍼 수만큼만 읽기
    }

    starTick = osKernelGetTickCount();
    if (xBytesAvailable > 0)
    {
      /* 데이터를 읽을 수 있다면, 데이터를 수신 */
      xBytesRead = xStreamBufferReceive(uart->quad_stream, (void *)&p_buff[cnt],
                                        xBytesAvailable, pdMS_TO_TICKS(timeout));

      if (xBytesRead > 0)
      {
        cnt += xBytesRead;
        lastTick = osKernelGetTickCount();
      }
    }
    else
    {
      /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
      xBytesRead = xStreamBufferReceive(uart->quad_stream, (void *)&p_buff[cnt], 1,
                                        pdMS_TO_TICKS(timeout));
      if (xBytesRead == 1)
      {
        cnt += 1;
        lastTick = osKernelGetTickCount();
      }
    }
    stopTick = osKernelGetTickCount();
    elapseTick = stopTick - starTick;

    if (elapseTick >= timeout || cnt >= buffSize)
    {

      OS_POST_SEM(uart->rx_sem);

      return cnt;
    }
    remainBuffSize -= xBytesAvailable;
    timeout = timeout - elapseTick;
  }


}

int32_t tl16c554_recv(int uart_num, uint8_t *p_buff, uint16_t buffSize, uint32_t timeOutMs)
{
  uint32_t startTick = osKernelGetTickCount();
  size_t cnt = 0;
  size_t xBytesAvailable;
  size_t xBytesRead;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  OS_PEND_SEM(uart->rx_sem, osWaitForever);

  // timeOutMs가 0인 경우: 논블로킹 모드 (데이터가 있으면 읽고 없으면 즉시 리턴)
  if (timeOutMs == 0)
  {
    xBytesAvailable = xStreamBufferBytesAvailable(uart->quad_stream);

    if (xBytesAvailable > 0)
    {
      size_t bytesToRead = (xBytesAvailable > buffSize) ? buffSize : xBytesAvailable;
      xBytesRead = xStreamBufferReceive(uart->quad_stream,
                                        p_buff,
                                        bytesToRead,
                                        0); // 대기시간 0
      cnt = xBytesRead;
    }
    // 데이터가 없으면 cnt는 0으로 리턴
  }
  // timeOutMs가 0xFFFFFFFF인 경우: 무한 대기 모드
  else if (timeOutMs == 0xFFFFFFFF)
  {
    while (cnt < buffSize)
    {
      xBytesAvailable = xStreamBufferBytesAvailable(uart->quad_stream);

      size_t bytesToRead = buffSize - cnt;
      if (xBytesAvailable > bytesToRead)
      {
        xBytesAvailable = bytesToRead;
      }

      if (xBytesAvailable == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신까지 무한 대기
        xBytesRead = xStreamBufferReceive(uart->quad_stream,
                                          &p_buff[cnt],
                                          1,
                                          osWaitForever);
      }
      else
      {
        // 사용 가능한 데이터를 읽음
        xBytesRead = xStreamBufferReceive(uart->quad_stream,
                                          &p_buff[cnt],
                                          xBytesAvailable,
                                          osWaitForever);
      }

      if (xBytesRead > 0)
      {
        cnt += xBytesRead;
      }
    }
  }
  // timeOutMs가 양수인 경우: 지정된 타임아웃 적용
  else
  {
    uint32_t timeoutTick = timeOutMs;

    while (cnt < buffSize)
    {
      uint32_t elapsedTick = osKernelGetTickCount() - startTick;

      if (elapsedTick >= timeoutTick)
      {
        break; // Timeout 발생
      }

      uint32_t remainingTime = timeoutTick - elapsedTick;

      xBytesAvailable = xStreamBufferBytesAvailable(uart->quad_stream);

      size_t bytesToRead = buffSize - cnt;
      if (xBytesAvailable > bytesToRead)
      {
        xBytesAvailable = bytesToRead;
      }

      if (xBytesAvailable == 0)
      {
        // 데이터가 없으면 최소 1바이트 수신 대기
        xBytesRead = xStreamBufferReceive(uart->quad_stream,
                                          &p_buff[cnt],
                                          1,
                                          remainingTime);
      }
      else
      {
        // 데이터를 읽음
        xBytesRead = xStreamBufferReceive(uart->quad_stream,
                                          &p_buff[cnt],
                                          xBytesAvailable,
                                          remainingTime);
      }

      if (xBytesRead > 0)
      {
        cnt += xBytesRead;
      }
      else
      {
        // xStreamBufferReceive가 0을 리턴하면 타임아웃 발생
        break;
      }
    }
  }

  OS_POST_SEM(uart->rx_sem);

  return cnt;
}

void tl16c554_flush_rx(int uart_num)
{
  uint8_t data;

  while (tl16c554_recv(uart_num, &data, 1, 0));

}


void tl16c554_set(int uart_num, uart_set_option_t option, void *value)
{

  uart_config_t *config;

  switch (option)
  {
    case eUART_SET_CONFIG:
      config = (uart_config_t *)option;
      set_baud_rate(uart_num, config->baud);
      break;
  }
}

void tl16c554_uart_get(int num, uart_get_option_t cmd, void *option)
{
  uart_config_t *opt_cfg = option;

  switch (cmd)
  {
    case UART_GET_CONFIG:
      opt_cfg->baud = tl16c554_inst[num].baud;
      opt_cfg->parityIdx = tl16c554_inst[num].parityIdx;
      break;
  }
}


/**
 * @brief 특정 용도 첫번째 바이트가 특정 시간안에 수신되어야 하며 그다음부터 특정시간안에 데이터가
 * 수신 안되면 종료 처리
 * 사용 예) 토큰 구분이 없는 프레임 수신
 * 일반적으로 데이터 수신은 연속된 바이트 수신이라고 가정 
 * 프레임이 길이를 판단할수 없는 프레임인 경우 응용하여 사용
 * 
 */
int32_t tl16c554_recv_opt(int uart_num, uint8_t *buffer, uint16_t buffer_size,
                           uint32_t timeout1_ms, uint32_t timeout2_ms)
{

  int32_t received = 0;
  uint8_t *p = buffer;

  int32_t ret = tl16c554_recv(uart_num, p, 1, timeout1_ms);
  if (ret <= 0)
    return 0;  

  received += ret;
  p += ret;

  while (received < buffer_size)
  {
    ret = tl16c554_recv(uart_num, p, 1, timeout2_ms);
    if (ret <= 0)
      break; 

    received += ret;
    p += ret;
  }

  return received;
}

int32_t tl16c554_recv_ll(int uart_num, uint8_t *p_buff, uint16_t buff_size, uint32_t timeout_ms)
{
  uint32_t start_tick;
  int32_t recved_cnt = 0;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];
  start_tick = OS_GET_TICK();
  while (1)
  {
    if (read_register(LSR(uart->base_address)) & LSR_DR)
    {
      p_buff[recved_cnt++] = read_register(RBR(uart->base_address));
    }
    if (recved_cnt == buff_size)
    {
      break;
    }
    if ((OS_GET_TICK() - start_tick) > timeout_ms)
    {
      break;
    }
  }

  return recved_cnt; // 데이터가 준비되지 않음
}

/**
 * @brief 가상 입력 처리 
 */
int32_t tl16c554_uart_inject(int uart_num, const uint8_t *pData, uint16_t dataLen)
{
  size_t xBytesSent;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  xBytesSent = xStreamBufferSend(uart->quad_stream, pData, dataLen,pdMS_TO_TICKS(10));

  return xBytesSent;
}

int32_t tl16c554_uart_recv_crlf(int num, char *p_buff, uint16_t bSize, uint32_t tout_ms)
{
  uint8_t data;
  uint16_t cnt = 0;
  uint32_t startTime, startTick, stopTick, elapseTick;
  uint32_t timeout;
  uint32_t len;

  startTime = OS_GET_TICK();
  timeout = tout_ms;

  do
  {
    startTick = OS_GET_TICK();
    len = tl16c554_recv(num, &data, 1, tout_ms);

    if (len)
    {
      p_buff[cnt++] = data;

      if ((cnt == 1) && ((data == '\r') || (data == '\n')))
      {
        cnt = 0;
        continue;
      }

      if ((data == '\r') || (data == '\n'))
      {
        p_buff[cnt - 1] = 0;
        return (cnt - 1); /* \r 또는 \n 를 제외한 문자열 길이 리턴*/
      }

      if (cnt == bSize)
      {
        return 0;
      }
    }

    stopTick = OS_GET_TICK();
    elapseTick = stopTick - startTick;

    if ((tout_ms == 0) || ((stopTick - startTime) >= tout_ms))
    {
      break;
    }
    if (tout_ms != osWaitForever)
    {
      timeout = timeout - elapseTick;
    }
  } while (1);

  return 0;
}


//테스트 필요
//세마포어 적용 필요
void tl16c554_uart_set_config(int uart_num, uart_config_t *config)
{
  uint8_t parity_mode;
  tl16c554_instance_t *uart = &tl16c554_inst[uart_num];

  if (!uart->opened)
  {
    return;
  }

  uart->baud = config->baud;
  uart->parityIdx = config->parityIdx;

  set_baud_rate(uart_num, config->baud);

  if (config->parityIdx == PARITY_NONE)
  {
    parity_mode = 0;
  }
  else if (config->parityIdx == PARITY_ODD)
  {
    parity_mode = 1;
  }
  else if (config->parityIdx == PARITY_EVEN)
  {
    parity_mode = 2;
  }
  else
  {
    parity_mode = 0;
  }

  set_parity(uart_num, parity_mode);
  set_stop_bit(uart_num, config->stop_bit);

}