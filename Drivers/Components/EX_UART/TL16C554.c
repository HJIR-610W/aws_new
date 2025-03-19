

#include "TL16C554.h"

#include <stdio.h>

#include "pcb_define.h"
#include "cmsis_os2.h"
#include "driver_di.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "utile.h"
#include "dev_io.h"
#include "system_err.h"
#include "driver_di.h"

#define STREAMBUFFER_USE 1 //데이터 수신을 freertos 스트림 버퍼 사용시 


#define UART_CLOCK_FREQ 3686400

// DLAB 비트 마스크
#define DLAB_BIT 0x80 // LCR 레지스터의 DLAB 비트

#define LSR_DR 0x01 // Data Ready 비트
// LSR의 비트 마스크
#define LSR_THRE 0x20 // Transmitter Holding Register Empty 비트
#define LSR_TEMT 0x40

  volatile uint8_t *exUartBaseAddress[8] = {(uint8_t*)0x68000000,
                                              (uint8_t*)0x68000010,
                                              (uint8_t*)0x68000020,
                                              (uint8_t*)0x68000030,
                                              (uint8_t*)0x68000040,
                                              (uint8_t*)0x68000050,
                                              (uint8_t*)0x68000060,
                                              (uint8_t*)0x68000070};


#define RBR(BASE) (void *)(BASE + 0x00) // Transmitter Holding Register
#define THR(BASE) (void *)(BASE + 0x00) // Transmitter Holding Register
#define DLL(BASE) (void *)(BASE + 0x00) // Divisor Latch Low
#define DLM(BASE) (void *)(BASE + 0x01) // Divisor Latch High

#define IER(BASE) (void *)(BASE + 0x01)


#define FCR(BASE) (void *)(BASE + 0x02) 
#define IIR(BASE) (void *)(BASE + 0x02) 
#define LCR(BASE) (void *)(BASE + 0x03) 
#define MCR(BASE) (void *)(BASE + 0x04) 
#define LSR(BASE) (void *)(BASE + 0x05)  //라인상태 레지스터터

#define MSR(BASE) (void *)(BASE + 0x06) 
#define SCR(BASE) (void *)(BASE + 0x07) 


// 레지스터 오프셋
#define DLL_OFFSET 0x00 // Divisor Latch Low
#define DLM_OFFSET 0x01 // Divisor Latch High
#define LCR_OFFSET 0x03 // Line Control Register
#define FCR_OFFSET 0x02 // FIFO Control Register
#define MCR_OFFSET 0x04 // Modem Control Register


#define QUAD_1_BUFF_SIZE 200 //D_SUB
#define QUAD_2_BUFF_SIZE 100 //TTL
#define QUAD_3_BUFF_SIZE 100 //EXT3
#define QUAD_4_BUFF_SIZE 100 //EXT4
#define QUAD_5_BUFF_SIZE 100 //RS485_A
#define QUAD_6_BUFF_SIZE 100 //RS485_B
#define QUAD_7_BUFF_SIZE 100 //EXT1
#define QUAD_8_BUFF_SIZE 100 //EXT2

typedef struct tl16c554_cfg_s
{
  driver_t *irq_io;
  uint8_t channel;
}tl16c554_cfg_t;

const uint8_t g_streamBuffSizeList[TL16C554_UART_MAX]={QUAD_1_BUFF_SIZE,QUAD_2_BUFF_SIZE,QUAD_3_BUFF_SIZE,
                                       QUAD_4_BUFF_SIZE,QUAD_5_BUFF_SIZE,QUAD_6_BUFF_SIZE,
                                       QUAD_7_BUFF_SIZE,QUAD_8_BUFF_SIZE};

StreamBufferHandle_t g_quad_xStreamBuffer[TL16C554_UART_MAX];


void irq_INTA_1(void *arg);
void irq_INTB_2(void *arg);
void irq_INTC_3(void *arg);
void irq_INTD_4(void *arg);
void irq_INTA_5(void *arg);
void irq_INTB_6(void *arg);
void irq_INTC_7(void *arg);
void irq_INTD_8(void *arg);
 void tl16c554_DMA_init(void);


uint8_t read_register(void * addr) 
{
  uint8_t data;

  data =  *((volatile  uint8_t *)addr);

  return data;
}

// 레지스터 쓰기 함수
void write_register(void * addr, uint8_t value) 
{
  *((volatile uint8_t *)addr) = value;
}

// 보오드레이트 확인 함수
void check_baud_rate(int uart_num)
{
    uint8_t data;

 
    data = read_register(LCR(exUartBaseAddress[uart_num]));
    // LCR의 DLAB 비트를 1로 설정하여 DLL 및 DLM 접근 허용

    data |= 0x80; // DLAB 비트 설정


    write_register(LCR(exUartBaseAddress[uart_num]),data);

    uint8_t dll_value = read_register(DLL(exUartBaseAddress[uart_num]));
    uint8_t dlm_value = read_register(DLM(exUartBaseAddress[uart_num]));

    data =   read_register(LCR(exUartBaseAddress[uart_num])) ;
                                                           
    // DLAB 비트를 다시 0으로 설정하여 DLL 및 DLM 접근 비허용
    data &= ~0x80;
    
    write_register(LCR(exUartBaseAddress[uart_num]),data);

    // Divisor 계산 (DLM은 상위 바이트, DLL은 하위 바이트)
    uint16_t divisor = (dlm_value << 8) | dll_value;

    if (divisor == 0) {
        debug_printf("Invalid divisor value.\n");
        return;
    }

    // 보오드레이트 계산
    uint32_t baud_rate = UART_CLOCK_FREQ / (16 * divisor);
    debug_printf("Calculated Baud Rate: %u\n", baud_rate);
}



// 보오드레이트 설정 함수
void set_baud_rate(int uart_num,uint32_t baud_rate) 
{
    uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate);

    // DLAB 비트를 1로 설정하여 DLL과 DLM에 접근 가능하게 함
    uint8_t lcr_value = read_register(LCR(exUartBaseAddress[uart_num]));
    write_register(LCR(exUartBaseAddress[uart_num]), lcr_value | DLAB_BIT);

    // Divisor 값 설정
    write_register(DLL(exUartBaseAddress[uart_num]), divisor & 0xFF); // 하위 바이트 설정
    write_register(DLM(exUartBaseAddress[uart_num]), (divisor >> 8) & 0xFF); // 상위 바이트 설정

    // DLAB 비트를 0으로 다시 설정하여 DLL과 DLM 접근 비허용
    write_register(LCR(exUartBaseAddress[uart_num]), lcr_value & ~DLAB_BIT);
}


#define PEN             (1 << 3)  // 패리티 활성화 비트
#define EPS             (1 << 4)  // 짝수 패리티 비트
#define SP              (1 << 5)  // 강제 패리티 비트
// 패리티 설정 함수
void set_parity(uint8_t uart_num, uint8_t parity_mode)
{
  volatile uint8_t lcr;


  lcr =     read_register(LCR(exUartBaseAddress[uart_num])) ;
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

  write_register(LCR(exUartBaseAddress[uart_num]), lcr_val);


}

void quad_init(driver_t *tls16c554,void *opt)
{
  uint8_t parity_mode;
  uint8_t flag=0;
  uint8_t g_reg;
  uart_config_t *config = opt;
  di_isr_set_cfg_t isr_cfg; 
  tl16c554_cfg_t *cfg= tls16c554->cfg;


  void (*isrTable[TL16C554_UART_MAX])(void *)={irq_INTA_1,irq_INTB_2,irq_INTC_3,irq_INTD_4,
                               irq_INTA_5,irq_INTB_6,irq_INTC_7,irq_INTD_8};
  const char *isrNameTable[TL16C554_UART_MAX]={TOSTRING(irq_INTB_1),TOSTRING(irq_INTB_2),
                                   TOSTRING(irq_INTC_3),TOSTRING(irq_INTD_4),
                                   TOSTRING( irq_INTA_5),TOSTRING(irq_INTB_6),
                                   TOSTRING(irq_INTC_7),TOSTRING(irq_INTD_8)};
  int baud_rate = config->baud;
  int uart_num = cfg->channel;

  // 보오드레이트 설정을 위한 Divisor 계산
  uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate);

  // DLAB 비트 설정 (LCR의 MSB 비트) 1로 해야 분주비 레지스터 접근 가능
  write_register(LCR(exUartBaseAddress[uart_num]),0x80);
      
  // DLL과 DLM에 divisor 값 설정

  write_register(DLL(exUartBaseAddress[uart_num]),divisor & 0xFF);
  write_register(DLM(exUartBaseAddress[uart_num]),(divisor >> 8) & 0xFF);
        
   // DLAB 비트를 0으로 설정하여 LCR 설정, 상태레지스터 접근 가능
  write_register(LCR(exUartBaseAddress[uart_num]),0x03);
  

  if(config->parityIdx ==PARITY_NONE)
  {
    parity_mode = 0;
  }
  else if(config->parityIdx == PARITY_ODD)
  {
    parity_mode = 1;
  }
  else //even
  {
    parity_mode = 2;
  }
  set_parity(uart_num,parity_mode);
    
  /*
  FIFO 설정 (FCR) 트리거 레벨 1바이트
  */
  write_register(FCR(exUartBaseAddress[uart_num]),0x07);// FIFO enable, RX/TX FIFO reset


  // MCR 설정 (필요에 따라 추가 설정)
  write_register(MCR(exUartBaseAddress[uart_num]),0x08);

//인터럽트 설정 Bit 3,2,1
#define IER_RDA          0x01 //데이터가 수신됨
#define IER_THRE         0x02 //송신버퍼 빈상태
#define IER_LINE_STATUS  0x04 //라인상태 변경됨됨
#define IER_MODEM_STATUS 0x08 //모뎀 상태 변경됨
#if STREAMBUFFER_USE
  flag = IER_RDA|IER_LINE_STATUS|IER_MODEM_STATUS;

  g_reg =read_register(IER(exUartBaseAddress[uart_num]))|(flag);;
  write_register(IER(exUartBaseAddress[uart_num]),g_reg);
#endif


  isr_cfg.call    = isrTable[uart_num];
  isr_cfg.name    = isrNameTable[uart_num];
  isr_cfg.trigger = eDI_RISING;
  isr_cfg.prio    = 6;
  isr_cfg.handle  = tls16c554;

  driver_di_set(cfg->irq_io,DI_SET_INTERRUPT,&isr_cfg);

}

/**
 * @brief 1바이트 전송
 * @retval <0 오류,1 정상 전송송
 */
int32_t send_data(uint8_t channel,uint8_t data)
{
  uint32_t startTime;
  uint8_t lsr;
  
  
  // 송신 버퍼가 비어있을 때까지 대기
  startTime = osKernelGetTickCount();
  
  do{
        if((osKernelGetTickCount()-startTime)>10)
    {
      return -1;
    }
  }while ((read_register(LSR(exUartBaseAddress[channel])) & LSR_THRE) == 0);
  
  // 데이터를 THR에 씁니다.
  write_register(THR(exUartBaseAddress[channel]), data);
  
  return 1;
}

// 데이터 읽기 함수
int read_byte(int uart_num, uint8_t *data) {
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[uart_num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[uart_num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } else {
        return 0; // 데이터가 준비되지 않음
    }
}




int quad_recv_byte(driver_t *drv,uint8_t *data)
{
  tl16c554_cfg_t *cfg = drv->cfg;
  
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[cfg->channel])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[cfg->channel])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } else {
        return 0; // 데이터가 준비되지 않음
    }  
}















/**
 * @brief 타임아웃을주고 최소 1바이트 수신된 이후부터 dataTimeOutMs동안 데이터 수신 못하면
 *  종료 처리
 * 
 * 예)modbus 활용 
 */
uint16_t tls16c554_uart_recvsOpt(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,
                                  uint32_t waitTimeOutMs,uint32_t dataTimeOutMs)
{
#if STREAMBUFFER_USE // 레지스터 직접 접근
    uint32_t starTick;
    uint32_t stopTick;
    uint32_t elapseTick;
    uint32_t timeout;
   // uint32_t lastTick=0;
    uint32_t waitTimeOut=0;
    uint32_t startTime;
    size_t xBytesAvailable;
    size_t xBytesRead;
    size_t remainBuffSize = buffSize;
    size_t cnt = 0;
    uint8_t waitCnt=0;
  bool once=true;

  (void)waitTimeOut;
    timeout = waitTimeOutMs;

tl16c554_cfg_t *cfg = drv->cfg;
    startTime  = osKernelGetTickCount();
    while(1)
    {

        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable( g_quad_xStreamBuffer[cfg->channel] );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = osKernelGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[cfg->channel], ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              waitCnt++;
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[cfg->channel], ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
            if(xBytesRead ==1)
            {
              cnt += 1;
              waitCnt++;
            }
        }
        //데이터가 하나라도 수신되기 전까지는 총 지연시간만큼 기다리고
        //데이터가 하나라도 수신된 이후 부터는 데이터 타임아웃 만큼 기다림
        if(once&&cnt)
        {
          once = false;
          timeout = dataTimeOutMs;
        }

        stopTick = osKernelGetTickCount();
        elapseTick = stopTick-starTick;
        

        if(once)//데이터가 하나도 수신안되었으면 경과시간 확인
        {
          if(elapseTick >= timeout)
          {
            return 0;
          }
        }
        else// 데이터가 하나라도 수신되었으면 최초부터
        { 
          if((stopTick - startTime)>waitTimeOutMs || cnt >= buffSize)
          {
            return cnt;
          }
        }

        if(waitCnt>=2 && elapseTick>dataTimeOutMs)
        {
          return cnt;
        }

        remainBuffSize -= xBytesAvailable;
        timeout = timeout - elapseTick; 
    }

#else
  uint32_t startTick;
  uint16_t cnt=0;

  startTick = xTaskGetTickCount();
  while(1)
  {
    if (read_register(LSR(exUartBaseAddress[drv->num])) & LSR_DR)
    {
      pBuff[cnt++] = read_register(RBR(exUartBaseAddress[drv->num])); 
    } 
    if(cnt==buffSize)
    {
      break;
    }
    if((xTaskGetTickCount()-startTick)>timeOutMs)
    {
      break;
    }
  }
    
    return cnt; // 데이터가 준비되지 않음

    #endif

   // return 0;
}

uint16_t tls16c554_uart_recvsOpt2(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,
                                  uint32_t waitTimeOutMs,uint32_t dataTimeOutMs)
{
#if STREAMBUFFER_USE // 레지스터 직접 접근
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
    uint8_t waitCnt=0;
  bool once=true;
tl16c554_cfg_t *cfg = drv->cfg;

    timeout = waitTimeOutMs;

    startTime  = osKernelGetTickCount();
    while(1)
    {

        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable( g_quad_xStreamBuffer[cfg->channel] );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = osKernelGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[cfg->channel], ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              waitCnt++;
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[cfg->channel], ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
            if(xBytesRead ==1)
            {
              cnt += 1;
              waitCnt++;
            }
        }
        //데이터가 하나라도 수신되기 전까지는 총 지연시간만큼 기다리고
        //데이터가 하나라도 수신된 이후 부터는 데이터 타임아웃 만큼 기다림
        if(once&&cnt)
        {
          once = false;
          timeout = dataTimeOutMs;
        }

        stopTick = osKernelGetTickCount();
        elapseTick = stopTick-starTick;
        

        if(once)//데이터가 하나도 수신안되었으면 경과시간 확인
        {
          if(elapseTick >= timeout)
          {
            return 0;
          }
        }
        else// 데이터가 하나라도 수신되었으면 최초부터
        { 
          if((stopTick - startTime)>waitTimeOutMs || cnt >= buffSize)
          {
            return cnt;
          }
        }

        if(waitCnt>=2 && elapseTick>dataTimeOutMs)
        {
          return cnt;
        }

        remainBuffSize -= xBytesAvailable;
        timeout = timeout - elapseTick; 
    }

#else
  uint32_t startTick;
  uint16_t cnt=0;

  startTick = xTaskGetTickCount();
  while(1)
  {
    if (read_register(LSR(exUartBaseAddress[drv->num])) & LSR_DR)
    {
      pBuff[cnt++] = read_register(RBR(exUartBaseAddress[drv->num])); 
    } 
    if(cnt==buffSize)
    {
      break;
    }
    if((xTaskGetTickCount()-startTick)>timeOutMs)
    {
      break;
    }
  }
    
    return cnt; // 데이터가 준비되지 않음

    #endif

    //return 0;
}






#define UART_IIR_INTTERUPT_PENDING 0x01
#define UART_IIR_RX_LINE_STAT  0x06  // 수신 라인 상태 (OE, PE, FE, BI)
#define UART_IIR_RX_DATA_AVAIL 0x04  // 수신 데이터 사용 가능 (FIFO 모드에서 트리거 레벨 도달)
#define UART_IIR_CHAR_TIMEOUT  0x0c  // 문자 타임아웃 발생
#define UART_IIR_THRE          0x02  // 송신기 홀딩 레지스터 비어 있음 (THRE)
#define UART_IIR_MODEM_STATUS   0x00  // 모뎀 상태 변화 (CTS, DSR, RI, DCD)


void irq_tl16c554(driver_t *drv)
{
  uint8_t iir;
  uint8_t interruptType ;
  uint8_t data; 
  uint8_t reg;
  uint8_t lineStatus;
  uint8_t modemStatus;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  size_t xBytesSent;
  tl16c554_cfg_t *cfg = drv->cfg;
  
 while (((iir = read_register(IIR(exUartBaseAddress[cfg->channel]))) & UART_IIR_INTTERUPT_PENDING) == 0)
  {
    interruptType = iir&0x0F;

    switch (interruptType) 
    {
        case UART_IIR_RX_DATA_AVAIL://데이터 수신
             data = read_register(RBR(exUartBaseAddress[cfg->channel])); // RBR에서 데이터 읽기

            /* 데이터를 스트림 버퍼에 전송 */
            xBytesSent = xStreamBufferSendFromISR(g_quad_xStreamBuffer[cfg->channel],&data, 
                                        1, &xHigherPriorityTaskWoken);
            /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

            if(!(xBytesSent > 0))
            {
              __asm("BKPT #0"); 
            }
            break;
        case UART_IIR_THRE:  // Transmitter Holding Register Empty
            // Handle TX Ready
            write_register(THR(exUartBaseAddress[cfg->channel]), '1');
            break;
        case UART_IIR_RX_LINE_STAT:  // Receiver Line Status
            // Handle Line Error
            lineStatus = read_register(LSR(exUartBaseAddress[cfg->channel])); // RBR에서 데이터 읽기
            (void)lineStatus;
            // Check for specific errors
            break;

        case UART_IIR_MODEM_STATUS:  // Modem Status
            // Handle Modem Status
            modemStatus = read_register(MSR(exUartBaseAddress[cfg->channel])); // RBR에서 데이터 읽기
            (void)modemStatus;
            break;
    case UART_IIR_CHAR_TIMEOUT:
                  reg = read_register(RBR(exUartBaseAddress[cfg->channel])); // RBR에서 데이터 읽기
                  (void)reg;
            break;
      break;


        default:
            // Handle other cases (if applicable)
            break;
    }
  }

}


void irq_INTA_1(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTB_2(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTC_3(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTD_4(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTA_5(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTB_6(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTC_7(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}

void irq_INTD_8(void *arg)
{
  irq_tl16c554((driver_t *)arg);
}


// DMA 핸들러 선언
DMA_HandleTypeDef hdma_memtomem;

void tl16c554_DMA_init(void)
{
    __HAL_RCC_DMA2_CLK_ENABLE(); // DMA2 클럭 활성화

    // DMA 핸들러 설정
    hdma_memtomem.Instance = DMA2_Stream0; // DMA2의 Stream 0 사용
    hdma_memtomem.Init.Channel = DMA_CHANNEL_0; // DMA 채널 0
    hdma_memtomem.Init.Direction = DMA_MEMORY_TO_MEMORY; // 메모리에서 메모리로 전송
    hdma_memtomem.Init.PeriphInc = DMA_PINC_ENABLE; // 주변 장치 주소 증가
    hdma_memtomem.Init.MemInc = DMA_MINC_ENABLE; // 메모리 주소 증가
    hdma_memtomem.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; // 주변 장치 데이터 정렬 (바이트 단위)
    hdma_memtomem.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE; // 메모리 데이터 정렬 (바이트 단위)
    hdma_memtomem.Init.Mode = DMA_NORMAL; // 단일 전송 모드
    hdma_memtomem.Init.Priority = DMA_PRIORITY_LOW; // 낮은 우선순위
    hdma_memtomem.Init.FIFOMode = DMA_FIFOMODE_DISABLE; // FIFO 비활성화

    // DMA 초기화
    if (HAL_DMA_Init(&hdma_memtomem) != HAL_OK) {
        // 초기화 실패 처리
        debug_printf("DMA Initialization Failed\n");
        while (1);
    }

    // DMA 전송 완료 인터럽트 활성화
    HAL_NVIC_SetPriority(DMA2_Stream0_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream0_IRQn);
}

// DMA 전송 완료 콜백 함수
void HAL_DMA_XferCpltCallback(DMA_HandleTypeDef *hdma) {
    if (hdma->Instance == DMA2_Stream0) { // DMA 스트림 확인
        debug_printf("ok\n"); // 전송 완료 메시지 출력
    }
}
void HAL_DMA_XferErrorCallback(DMA_HandleTypeDef *hdma) {
    debug_printf("DMA Transfer Error\n");
}


void tls16c554_send_DMA(int num,const uint8_t *pData,uint16_t dataLen)
{
  
  uint32_t dest_address = (uint32_t )THR(exUartBaseAddress[num]);
    // DMA 전송 시작
    if (HAL_DMA_Start_IT(&hdma_memtomem, (uint32_t)pData, dest_address, dataLen) != HAL_OK) {
        // DMA 시작 실패 처리
        debug_printf("DMA Start Failed\n");
        while (1);
    }
}


driver_t tls16c554_driver[TL16C554_UART_MAX];
tl16c554_cfg_t g_tl16c554_cfg[TL16C554_UART_MAX];


void tls16c554_close(driver_t *handle);
int32_t tls16c554_send(driver_t *handle, const uint8_t *pData, uint16_t dataLen);
int32_t tls16c554_recv(driver_t *handle, uint8_t *buffer, uint16_t length, uint32_t timeOutMs);
void tls16c554_flush_rx(driver_t *handle);
int32_t tls16c554_recv_opt(driver_t *handle, uint8_t *buffer, uint16_t length, eUART_RECV_OPT_t cmd,void *opt);
void tls16c554_set(driver_t *handle, uart_set_option_t option, void *value);

uart_api_t tl16c554_api={.close = tls16c554_close,
                         .send = tls16c554_send,
                         .recv =tls16c554_recv,
                         .flush_rx = tls16c554_flush_rx,
                         .recv_opt = tls16c554_recv_opt,
                         .set = tls16c554_set};


void tls16c554_irq_init(driver_t *drv,uint8_t prio)
{
  di_isr_set_cfg_t isr_cfg; 
  tl16c554_cfg_t *cfg = drv->cfg;

  void (*isrTable[TL16C554_UART_MAX])(void *)={irq_INTA_1,irq_INTB_2,irq_INTC_3,irq_INTD_4,
                               irq_INTA_5,irq_INTB_6,irq_INTC_7,irq_INTD_8};
  const char *isrNameTable[TL16C554_UART_MAX]={TOSTRING(irq_INTB_1),TOSTRING(irq_INTB_2),
                                   TOSTRING(irq_INTC_3),TOSTRING(irq_INTD_4),
                                   TOSTRING( irq_INTA_5),TOSTRING(irq_INTB_6),
                                   TOSTRING(irq_INTC_7),TOSTRING(irq_INTD_8)};
  isr_cfg.call    = isrTable[cfg->channel];
  isr_cfg.name    = isrNameTable[cfg->channel];
  isr_cfg.trigger = eDI_RISING;
  isr_cfg.prio    = prio;
  isr_cfg.handle  = drv;

  driver_di_set(cfg->irq_io,DI_SET_INTERRUPT,&isr_cfg);
}

driver_t *tls16c554_open(uint32_t num,void *opt)
{
  const char *portNameList[8]={"QUAD_1","QUAD_2","QUAD_3","QUAD_4","QUAD_5","QUAD_6","QUAD_7","QUAD_8"};
  
  if(tls16c554_driver[num].opened)
  {
    return &tls16c554_driver[num];
  }
  
  tls16c554_driver[num].name   = portNameList[num];
  tls16c554_driver[num].opened = true;
  tls16c554_driver[num].api    = &tl16c554_api;

  g_tl16c554_cfg[num].channel = num;
  tls16c554_driver[num].cfg = &g_tl16c554_cfg[num];

  g_quad_xStreamBuffer[num] =   xStreamBufferCreate(g_streamBuffSizeList[num], 1 ); // Trigger level = 1
  
  switch(num)
  {
    case TL16C554_UART_0_D_SUB:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTA_1,0);
    break;
    case TL16C554_UART_1_TTL_TTL:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTB_2,0);
    break;
    case TL16C554_UART_EXT3:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTC_3,0);
    break;
    case TL16C554_UART_EXT4:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTD_4,0);
    break;
    case TL16C554_UART_4_RS485_A:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTA_5,0);
    break;
    case TL16C554_UART_5_RS485_B:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTB_6,0);
    break;
    case TL16C554_UART_6_EXT1:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTC_7,0);
    break;
    case TL16C554_UART_7_EXT2:
      g_tl16c554_cfg[num].irq_io   = driver_di_open(DI_QUAD_UARTD_8,0);
    break;
  }

  quad_init(&tls16c554_driver[num],opt);

  return &tls16c554_driver[num];

}

void tls16c554_close(driver_t *handle)
{
  

}



int32_t tls16c554_send(driver_t *handle, const uint8_t *pData, uint16_t dataLen)
{
  driver_t *drv = handle;
  tl16c554_cfg_t *cfg = drv->cfg;
  int32_t cnt=0;
  uint32_t startTime;
  
  while(dataLen)
  {
    dataLen--;
    if(send_data(cfg->channel,*pData++)==1)
    {
      cnt++;
    }
  }

  startTime = osKernelGetTickCount();

  do
  {
   if((osKernelGetTickCount()-startTime)>1000)
   {
     cnt = -1;
    break;
   }
  } while ((read_register(LSR(exUartBaseAddress[cfg->channel])) & LSR_TEMT) == 0);
  
/*
송신 레지스터 비어있음
THR 및 TSR이 모두 비어있을 때 설정됨
THR에 문자가 로드되면 LSR6은 클리어되며 문자가 완전히 송신될 때 까지 유지됨
*/

  return cnt;
}

int32_t tls16c554_recv(driver_t *handle, uint8_t *pBuff, uint16_t buffSize, uint32_t timeOutMs)
{
#if STREAMBUFFER_USE // 레지스터 직접 접근
    uint32_t starTick;
    uint32_t stopTick;
    uint32_t elapseTick;
    uint32_t timeout;
    uint32_t lastTick=0;
    size_t xBytesAvailable;
    size_t xBytesRead;
    size_t remainBuffSize = buffSize;
    size_t cnt = 0;
    driver_t *drv = handle;
    tl16c554_cfg_t *cfg = drv->cfg;
    uint8_t channel = cfg->channel;
    timeout = timeOutMs;
    
    (void)lastTick;

    while(1)
    {
        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable( g_quad_xStreamBuffer[channel] );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = osKernelGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[channel], ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              lastTick = osKernelGetTickCount();
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[channel], ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
            if(xBytesRead ==1)
            {
              cnt += 1;
              lastTick = osKernelGetTickCount();
            }
        }
        stopTick = osKernelGetTickCount();
        elapseTick = stopTick-starTick;

        
  
        if(elapseTick >= timeout  || cnt >= buffSize)
        {
          return cnt;
        }
        remainBuffSize -= xBytesAvailable;
        timeout = timeout - elapseTick; 
    }

#else
  uint32_t startTick;
  uint16_t cnt=0;

  startTick = xTaskGetTickCount();
  while(1)
  {
    if (read_register(LSR(exUartBaseAddress[channel])) & LSR_DR)
    {
      pBuff[cnt++] = read_register(RBR(exUartBaseAddress[channel])); 
    } 
    if(cnt==buffSize)
    {
      break;
    }
    if((xTaskGetTickCount()-startTick)>timeOutMs)
    {
      break;
    }
  }
    
    return cnt; // 데이터가 준비되지 않음

    #endif

    //return 0;
}



int32_t tls16c554_recv_1(driver_t *drv, uint8_t *pBuff, uint16_t buffSize,void *opt)
{
  uart_optTimeOut_t *optTimeOut=opt;
  uint32_t starTick;
  uint32_t stopTick;
  uint32_t elapseTick;
  uint32_t timeout;
  uint32_t lastTick=0;
  size_t xBytesAvailable;
  size_t xBytesRead;
  size_t remainBuffSize = buffSize;
  size_t cnt = 0;
  tl16c554_cfg_t *cfg = drv->cfg;
  uint8_t channel = cfg->channel;


  timeout = optTimeOut->frameTimeOutMs;
    
  (void)lastTick;

  while(1)
  {
        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable( g_quad_xStreamBuffer[channel] );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = osKernelGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[channel], ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
              lastTick = osKernelGetTickCount();
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[channel], ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
            if(xBytesRead ==1)
            {
              cnt += 1;
              lastTick = osKernelGetTickCount();
            }
        }


        stopTick = osKernelGetTickCount();
        elapseTick = stopTick-starTick;

        
  
        if(elapseTick >= timeout  || cnt >= buffSize)
        {
          return cnt;
        }
        remainBuffSize -= xBytesAvailable;


        if(cnt)
        {
          timeout = optTimeOut->dataTimeOutMs;
        }
        else
        {
        timeout = timeout - elapseTick; 
        }
    }
}

int32_t tls16c554_recv_opt(driver_t *drv, uint8_t *pBuff, uint16_t buffSize, eUART_RECV_OPT_t cmd,void *opt)
{
  uint16_t len = 0;
  int32_t cnt = 0;
  
  switch (cmd)
  {

  case eUART_OPT_DATA_TIMEOUT_1:
   cnt = tls16c554_recv_1(drv,pBuff,buffSize,opt);
  break;

  }
  return len;
}

void tls16c554_flush_rx(driver_t *drv)
{
  uint8_t data;
  
  while(tls16c554_recv(drv,&data,1,0));

}
int32_t tls16c554_available(driver_t *handle)
{

return 0;
}


void tls16c554_set(driver_t *handle, uart_set_option_t option, void *value)
{
  driver_t *drv = handle;
  tl16c554_cfg_t *cfg = drv->cfg;
  uart_config_t *config;

  switch(option)
  {
    case eUART_SET_CONFIG:
    config = (uart_config_t*)option;

    set_baud_rate(cfg->channel,config->baud) ;
    break;
  }
}


