

#include "TL16C554.h"

#include <stdio.h>


#include "driver_digitalIn.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include "utile.h"
#include "io.h"


#define STREAMBUFFER_USE 1 //데이터 수신을 freertos 스트림 버퍼 사용시 


#define UART_CLOCK_FREQ 3686400

// DLAB 비트 마스크
#define DLAB_BIT 0x80 // LCR 레지스터의 DLAB 비트

#define LSR_DR 0x01 // Data Ready 비트
// LSR의 비트 마스크
#define LSR_THRE 0x20 // Transmitter Holding Register Empty 비트
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
#define LSR(BASE) (void *)(BASE + 0x05) 

#define MSR(BASE) (void *)(BASE + 0x06) 
#define SCR(BASE) (void *)(BASE + 0x07) 


// 레지스터 오프셋
#define DLL_OFFSET 0x00 // Divisor Latch Low
#define DLM_OFFSET 0x01 // Divisor Latch High
#define LCR_OFFSET 0x03 // Line Control Register
#define FCR_OFFSET 0x02 // FIFO Control Register
#define MCR_OFFSET 0x04 // Modem Control Register

typedef struct adc_api_s
{
    void (*send)(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen);
    int32_t (*recv)(driver_t *tls16c554,uint8_t *pBuff);
    int32_t (*recv_byte)(driver_t *tls16c554,uint8_t *pData);
    void (*set)(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option);
    void (*init)(driver_t *tls16c554);
}tl16c554_api_t;

typedef struct tl16c554_cfg_s
{
  driver_t *irq_io;
}tl16c554_cfg_t;


#define QUAD_1_BUFF_SIZE 100
#define QUAD_2_BUFF_SIZE 100
#define QUAD_3_BUFF_SIZE 100
#define QUAD_4_BUFF_SIZE 100
#define QUAD_5_BUFF_SIZE 100
#define QUAD_6_BUFF_SIZE 100
#define QUAD_7_BUFF_SIZE 100
#define QUAD_8_BUFF_SIZE 100

const uint8_t g_streamBuffSizeList[8]={QUAD_1_BUFF_SIZE,QUAD_2_BUFF_SIZE,QUAD_3_BUFF_SIZE,
                                       QUAD_4_BUFF_SIZE,QUAD_5_BUFF_SIZE,QUAD_6_BUFF_SIZE,
                                       QUAD_7_BUFF_SIZE,QUAD_8_BUFF_SIZE};

StreamBufferHandle_t g_quad_xStreamBuffer[8];



void irq_INTA_1(void *arg);
void irq_INTB_2(void *arg);
void irq_INTC_3(void *arg);
void irq_INTD_4(void *arg);
void irq_INTA_5(void *arg);
void irq_INTB_6(void *arg);
void irq_INTC_7(void *arg);
void irq_INTD_8(void *arg);


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
        printf("Invalid divisor value.\n");
        return;
    }

    // 보오드레이트 계산
    uint32_t baud_rate = UART_CLOCK_FREQ / (16 * divisor);
    printf("Calculated Baud Rate: %u\n", baud_rate);
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

uint8_t g_reg;

void quad_init(driver_t *tls16c554)
{
  di_isr_set_cfg_t isr_cfg; 
  tl16c554_cfg_t *cfg;
  void (*isrTable[8])(void *)={irq_INTA_1,irq_INTB_2,irq_INTC_3,irq_INTD_4,
                               irq_INTA_5,irq_INTB_6,irq_INTC_7,irq_INTD_8};
  const char *isrNameTable[8]={TOSTRING(irq_INTB_1),TOSTRING(irq_INTB_2),
                                   TOSTRING(irq_INTC_3),TOSTRING(irq_INTD_4),
                                   TOSTRING( irq_INTA_5),TOSTRING(irq_INTB_6),
                                   TOSTRING(irq_INTC_7),TOSTRING(irq_INTD_8)};
  int baud_rate = 115200;
  int uart_num = tls16c554->num;

  // 보오드레이트 설정을 위한 Divisor 계산
  uint16_t divisor = UART_CLOCK_FREQ / (16 * baud_rate);

  // DLAB 비트 설정 (LCR의 MSB 비트)
  write_register(LCR(exUartBaseAddress[uart_num]),0x80);
      
  // DLL과 DLM에 divisor 값 설정

  write_register(DLL(exUartBaseAddress[uart_num]),divisor & 0xFF);
  write_register(DLM(exUartBaseAddress[uart_num]),(divisor >> 8) & 0xFF);
        
   // DLAB 비트를 0으로 설정하여 LCR 설정
  write_register(LCR(exUartBaseAddress[uart_num]),0x03);
        
    
  // FIFO 설정 (FCR)
  write_register(FCR(exUartBaseAddress[uart_num]),0x07);// FIFO enable, RX/TX FIFO reset


  // MCR 설정 (필요에 따라 추가 설정)
  write_register(MCR(exUartBaseAddress[uart_num]),0x08);


g_reg =read_register(IER(exUartBaseAddress[uart_num]));
  write_register(IER(exUartBaseAddress[uart_num]),0x05);


  cfg = tls16c554->cfg;

  isr_cfg.call    = isrTable[tls16c554->num];
  isr_cfg.name    = isrNameTable[tls16c554->num];
  isr_cfg.trigger = eDI_RISING;
  isr_cfg.prio    = 6;
  isr_cfg.handle  = tls16c554;

  driver_di_set(cfg->irq_io,DI_SET_INTERRUT,&isr_cfg);



}

// 데이터 전송 함수
void send_data(int uart_num,uint8_t data)
{
    // 송신 버퍼가 비어있을 때까지 대기
    while ((read_register(LSR(exUartBaseAddress[uart_num])) & LSR_THRE) == 0);

    // 데이터를 THR에 씁니다.
    write_register(THR(exUartBaseAddress[uart_num]), data);
}

void send_data_n(int uart_num,uint8_t *p_data,uint16_t dataLen)
{

  
  while(dataLen)
  {
    dataLen--;
    send_data(uart_num,*p_data++);
  }
}


// 데이터 읽기 함수
int read_data(int uart_num, uint8_t *data) {
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[uart_num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[uart_num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } else {
        return 0; // 데이터가 준비되지 않음
    }
}


void quad_send(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen)
{

}

int quad_recv_byte(driver_t *tls16c554,uint8_t *data)
{
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[tls16c554->num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[tls16c554->num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } else {
        return 0; // 데이터가 준비되지 않음
    }  
}

void quad_set(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option)
{


}








tl16c554_cfg_t g_tl16c554_cfg[8];
tl16c554_api_t g_tl16c554_api={.send = quad_send,
                               .recv_byte = quad_recv_byte,
                               .set  = quad_set,
                               .init = quad_init};
driver_t g_drv_quad_uart[8];



driver_t *tls16c554_open(int num)
{
  const char *portNameList[8]={"QUAD_1","QUAD_2","QUAD_3","QUAD_4","QUAD_5","QUAD_6","QUAD_7","QUAD_8"};
  
  if(g_drv_quad_uart[num].opened == true)
  {
    return &g_drv_quad_uart[num];
  }
  
  g_drv_quad_uart[num].name = portNameList[num];

  g_drv_quad_uart[num].opened = true;
  g_drv_quad_uart[num].num    = num;
  g_drv_quad_uart[num].api    = &g_tl16c554_api;//모두 같은 api 사용

  g_tl16c554_cfg[num].irq_io = driver_di_open(num +DI_QUAD_UARTA_1);

  g_drv_quad_uart[num].cfg = &g_tl16c554_cfg[num];


  g_quad_xStreamBuffer[num] =   xStreamBufferCreate( g_streamBuffSizeList[num], 1 ); // Trigger level = 1

  quad_init(&g_drv_quad_uart[num]);
  
  return &g_drv_quad_uart[num];
}






void tls16c554_send(driver_t *tls16c554,uint8_t *pData,uint16_t dataLen)
{
 
  while(dataLen)
  {
    dataLen--;
    send_data(tls16c554->num,*pData++);
  }
}

int32_t tls16c554_recv(driver_t *drv,uint8_t *data)
{
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[drv->num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[drv->num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } 

    
    return 0; // 데이터가 준비되지 않음
   
}

void tls16c554_set(driver_t *tls16c554,eTLS16C554_CMD_t cmd,void *option)
{
    uart_baud_config_t *config;

  switch(cmd)
  {
    case eUART_SET_CONFIG:
    config = (uart_baud_config_t*)option;

    set_baud_rate(tls16c554->num,config->baud) ;
    break;
  }
    
}

int tls16c554_recv_byte(driver_t *drv,uint8_t *data)
{
    // LSR의 DR 비트를 확인하여 수신 버퍼에 데이터가 있는지 확인
    if (read_register(LSR(exUartBaseAddress[drv->num])) & LSR_DR) {
        *data = read_register(RBR(exUartBaseAddress[drv->num])); // RBR에서 데이터 읽기
        return 1; // 데이터 읽기 성공
    } 

    
    return 0; // 데이터가 준비되지 않음
}


/**
 * @brief
 * 처음에는 사용자가 요청한 타임아웃 만큼 지연준다.
 * 데이터가 빨리 도착하면 아직 타임아웃이 남아있다
 * 남아있는 타임아웃동안 계속 수신한다.
 * 그러다 원하는 데이터만큼 수신이 되면 타임아웃은 무시되고 리턴된다.
 * @retval 수신된 데이터 숫자
 */
uint16_t tls16c554_uart_recvs(driver_t *drv,uint8_t *pBuff,uint16_t buffSize,uint32_t timeOutMs)
{
#if STREAMBUFFER_USE // 레지스터 직접 접근
    uint32_t starTick;
    uint32_t stopTick;
    uint32_t elapseTick;
    uint32_t timeout;
    size_t xBytesAvailable;
    size_t xBytesRead;
    size_t remainBuffSize = buffSize;
    size_t cnt = 0;


    timeout = timeOutMs;

    while(1)
    {
        /* 스트림 버퍼에서 읽을 수 있는 데이터 크기 확인 */
        xBytesAvailable = xStreamBufferBytesAvailable( g_quad_xStreamBuffer[drv->num] );

        if(remainBuffSize < xBytesAvailable)
        {
          xBytesAvailable = remainBuffSize;// 버퍼 수만큼만 읽기
        }

        starTick = xTaskGetTickCount();
        if( xBytesAvailable > 0 )
        {
            /* 데이터를 읽을 수 있다면, 데이터를 수신 */
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[drv->num], ( void * ) &pBuff[cnt], xBytesAvailable, pdMS_TO_TICKS( timeout ) );
            
            if(xBytesRead >0)
            {
              cnt += xBytesRead;
            }
        }
        else
        {
            /*데이터를 기다려야 한다면 최소 1개가 수신될때까지 대기*/
            xBytesRead = xStreamBufferReceive( g_quad_xStreamBuffer[drv->num], ( void * ) &pBuff[cnt], 1, pdMS_TO_TICKS( timeout ) );
            if(xBytesRead ==1)
            {
              cnt += 1;
            }


        }
        stopTick = xTaskGetTickCount();
        elapseTick = stopTick-starTick;

        if(timeout <= elapseTick ||cnt >= buffSize)
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

    return 0;
}



driver_t *g_quad_uart_INTA_1;
driver_t *g_quad_uart_INTB_2;
driver_t *g_quad_uart_INTC_3;
driver_t *g_quad_uart_INTD_4;
driver_t *g_quad_uart_INTA_5;
driver_t *g_quad_uart_INTB_6;
driver_t *g_quad_uart_INTC_7;
driver_t *g_quad_uart_INTD_8;

void tls16c554_init(driver_t *tls16c554)
{
  tl16c554_api_t *api = (tl16c554_api_t *)tls16c554->api;

  api->init(tls16c554);

}



void irq_tl16c554(driver_t *drv)
{
  uint8_t iir;
  uint8_t interruptType ;
  uint8_t data; 
  uint8_t lineStatus;
  uint8_t modemStatus;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
  size_t xBytesSent;
  
  
  iir = read_register(IIR(exUartBaseAddress[drv->num]));

  if ((iir & 0x01) == 0)
  {
     interruptType = (iir >> 1) & 0x07;  // Extract interrupt type

    switch (interruptType) 
    {
        case 0x01:  // Transmitter Holding Register Empty
            // Handle TX Ready
            write_register(THR(exUartBaseAddress[drv->num]), '1');
            break;

        case 0x02://데이터 수신
             data = read_register(RBR(exUartBaseAddress[drv->num])); // RBR에서 데이터 읽기

            /* 데이터를 스트림 버퍼에 전송 */
            xBytesSent = xStreamBufferSendFromISR(g_quad_xStreamBuffer[drv->num],&data, 
                                        1, &xHigherPriorityTaskWoken);
            /* 높은 우선순위의 태스크가 깨어나야 하면 컨텍스트 스위칭 요청 */
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

            if(!(xBytesSent > 0))
            {
              __asm("BKPT #0"); 
            }
            break;

        case 0x03:  // Receiver Line Status
            // Handle Line Error
            lineStatus = read_register(LSR(exUartBaseAddress[drv->num])); // RBR에서 데이터 읽기
            // Check for specific errors
            break;

        case 0x04:  // Modem Status
            // Handle Modem Status
            modemStatus = read_register(MSR(exUartBaseAddress[drv->num])); // RBR에서 데이터 읽기
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


