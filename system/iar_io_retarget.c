#include "iar_io_retarget.h"

#include <stddef.h>
#include <stdint.h>
#include <LowLevelIOInterface.h>  // 저수준 I/O 선언을 위한 IAR DLIB 헤더
#include <errno.h>


#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

#include "system_err.h"
#include "debug_io.h"
#include "drv_rs232.h"
#include "stm32f4xx_hal.h"

// off_t 타입을 정의 (일반적으로 파일 오프셋에 사용되며, 여기서는 int로 사용)
typedef int off_t;

// errno 변수 정의
 //int errno;



// isatty 함수 재정의: 표준 입출력 파일 디스크립터인지 확인
int __isatty(int file) {

    if(file == STDOUT_FILENO || file == STDERR_FILENO || file == STDIN_FILENO)
    {
        return 1;
    }
    errno = EBADF;  // 잘못된 파일 디스크립터
    return 0;
}


 int remove(const char *out)
 {
   return -1;
 }





 /* 표준 파일 핸들 */
#define STDIN 0
#define STDOUT 1
#define STDERR 2

 /* 최대 파일 핸들 수 (필요시 조정, 예: 파일 지원 추가 시) */
#define MAX_HANDLES 3  // stdin, stdout, stderr


 void low_level_put_char(unsigned char c)
{
   int32_t uart_handle = get_debug_uart_handle();  //
   if (uart_handle != -1)
   {
     drv_uart_send(uart_handle, &c, 1);  //
   }
}


  unsigned char low_level_get_char(void)
 {
   unsigned char c = 0;
   int32_t uart_handle = get_debug_uart_handle();  //
   if (uart_handle !=-1)
   {
     drv_uart_recv(uart_handle, &c, 1, 0xFFFFFFFF);  
   }
   return c;
 }

 /**
  * @brief 파일을 엽니다.
  * @param filename 파일명 문자열.
  * @param mode 열기 모드 플래그 (_LLIO_...).
  * @return 파일 핸들 (표준 스트림은 0, 1, 2, 에러 시 -1).
  * @note 기본 구현은 표준 스트림만 지원합니다.
  */
 __ATTRIBUTES int __open(const char *filename, int mode)
 {
#if 0 
   // 최소 구현: 표준 핸들만 지원합니다.
   if (filename == _LLIO_STDOUT || filename == _LLIO_STDERR || filename == _LLIO_STDIN)
   {
     if (filename == _LLIO_STDIN && (mode & _LLIO_RDONLY))
       return STDIN;
     if (filename == _LLIO_STDOUT && (mode & _LLIO_WRONLY))
       return STDOUT;
     if (filename == _LLIO_STDERR && (mode & _LLIO_WRONLY))
       return STDERR;
   }
#endif
   return -1;  // 에러: 이 기본 재정의에서는 파일 연산을 지원하지 않습니다.
 }

 /**
  * @brief 파일 핸들에 데이터를 씁니다.
  * @param handle 파일 핸들 (STDOUT=1, STDERR=2).
  * @param buffer 데이터 버퍼 포인터.
  * @param size 쓸 바이트 수.
  * @return 쓰여진 바이트 수, 실패 시 _LLIO_ERROR.
  */
 __ATTRIBUTES size_t __write(int handle, const unsigned char *buffer, size_t size)
 {
   int32_t uart_handle = get_debug_uart_handle();  //

   if (buffer == NULL )
   {
     return _LLIO_ERROR;
   }

   // stdout과 stderr만 처리하고, 둘 다 디버그 UART로 리디렉션합니다.
   if (handle != STDOUT && handle != STDERR)
   {
     return _LLIO_ERROR;  // 파일 쓰기는 지원하지 않습니다.
   }


   int32_t bytes_sent = drv_uart_send(uart_handle, (uint8_t *)buffer, size);  //

   // driver_uart_send가 전송된 바이트 수를 반환하거나 에러 시 음수 값을 반환한다고 가정합니다.
   if (bytes_sent < 0)
   {
     return _LLIO_ERROR;
   }

   return (size_t)bytes_sent;  // 실제로 쓰여진 바이트 수를 반환합니다.
                               // 또는 드라이버 함수가 성공 시 모든 바이트 전송을 보장한다면
                               // 'size'를 반환합니다.
 }

 /**
  * @brief 파일 핸들에서 데이터를 읽습니다.
  * @param handle 파일 핸들 (STDIN=0).
  * @param buffer 대상 버퍼 포인터.
  * @param size 읽을 최대 바이트 수.
  * @return 읽은 바이트 수, EOF 시 0, 실패 시 _LLIO_ERROR.
  */
 __ATTRIBUTES size_t __read(int handle, unsigned char *buffer, size_t size)
 {
   int32_t uart_handle = get_debug_uart_handle();  //

   if (buffer == NULL )
   {
     return _LLIO_ERROR;
   }

   // stdin만 처리합니다.
   if (handle != STDIN)
   {
     return _LLIO_ERROR;  // 파일 읽기는 지원하지 않습니다.
   }

   // dev_io.c의 UART 수신 함수를 사용합니다.
   // 이 기본 예제는 블로킹 방식의 문자 읽기 함수를 가정하여 한 번에 한 바이트씩 읽습니다.
   // 더 효율적인 구현은 타임아웃이 있는 driver_uart_recv를 사용할 수 있습니다.
   size_t bytes_read = 0;
   for (size_t i = 0; i < size; ++i)
   {
     // driver_uart_get_char가 하나의 문자를 가져오는 블로킹 호출이라고 가정합니다.
     // 성공 시 1, 에러/타임아웃 시 0 또는 음수를 반환한다고 가정합니다.
     int32_t result = drv_uart_recv(uart_handle, &buffer[i],1,0xFFFFFFFF);  

     if (result > 0)  // 문자 읽기 성공
     {
       bytes_read++;
       // 선택 사항: 터미널로 문자 에코백
       // __write(STDOUT, &buffer[i], 1);

       // 선택 사항: 줄 끝 처리 (예: '\r' 또는 '\n'에서 반환)
       if (buffer[i] == '\r' || buffer[i] == '\n')
       {
         // 선택 사항: CR/LF를 '\n'과 같은 표준 줄 끝 문자로 대체
         // buffer[i] = '\n';
         break;  // 개행 문자 후 읽기 중단
       }
     }
     else  // 에러 또는 타임아웃 발생
     {
       if (bytes_read == 0)  // 아무것도 읽기 전에 에러 발생
       {
         return _LLIO_ERROR;
       }
       break;  // 일부 문자를 읽은 후 에러 발생 시 읽기 중단
     }
   }

   return bytes_read;
 }

 /**
  * @brief 파일 핸들을 닫습니다.
  * @param handle 파일 핸들.
  * @return 성공 시 0, 에러 시 -1.
  * @note 기본 구현에서는 표준 스트림에 대해 아무 작업도 하지 않습니다.
  */
 __ATTRIBUTES int __close(int handle)
 {
   if (handle < 0 || handle >= MAX_HANDLES)
   {
     return -1;  // 유효하지 않은 핸들
   }
   // 이 기본 구현에서는 표준 스트림에 대해 별도 조치가 필요하지 않습니다.
   return 0;
 }

 /**
  * @brief 파일 내 위치를 탐색합니다.
  * @param handle 파일 핸들.
  * @param offset 오프셋 값.
  * @param whence 탐색 모드 (SEEK_SET, SEEK_CUR, SEEK_END).
  * @return 현재 파일 위치, 에러 시 -1L.
  * @note 표준 스트림에서는 지원되지 않습니다.
  */
 __ATTRIBUTES long __lseek(int handle, long offset, int whence)
 {
   // 표준 스트림은 탐색할 수 없습니다.
   if (handle == STDIN || handle == STDOUT || handle == STDERR)
   {
     return -1L;  // 에러 또는 지원 안 됨을 나타냅니다.
   }
   // 필요한 경우 여기에 파일 시스템 지원을 추가합니다.
   return -1L;  // 지원되지 않음
 }

 /**
  * @brief 디버거/세미호스팅을 사용하여 데이터를 씁니다 (사용 가능/구성된 경우).
  * @param handle 파일 핸들 (보통 무시되거나 특정 디버그 채널용).
  * @param buffer 데이터 버퍼 포인터.
  * @param size 쓸 바이트 수.
  * @return 쓰여진 바이트 수, 실패 시 _LLIO_ERROR.
  * @note 종종 __write 또는 특정 디버그 채널로 리디렉션됩니다.
  */
 __ATTRIBUTES size_t __dwrite(int handle, const unsigned char *buffer, size_t size)
 {
   // 일반적으로 디버그 쓰기를 표준 출력/에러와 동일한 UART로 리디렉션합니다.
   return __write(handle, buffer, size);
 }


 __ATTRIBUTES const char *__getzone(void)
 {
   static const char *timezone_str = "KST";
   return timezone_str;
 }

 /**
  * @brief 버퍼링된 데이터를 씁니다 (DLIB 특정).
  * @param handle 파일 핸들.
  * @param buffer 데이터 버퍼 포인터.
  * @param size 쓸 바이트 수.
  * @return 쓰여진 바이트 수, 실패 시 _LLIO_ERROR.
  * @note 종종 __write로 직접 리디렉션될 수 있습니다.
  */
 __ATTRIBUTES size_t __write_buffered(int handle, const unsigned char *buffer, size_t size)
 {
   return __write(handle, buffer, size);
 }



 /**
  * @brief 열려있는 모든 파일 핸들을 닫습니다.
  * @note 프로그램 종료 중에 호출됩니다 (링크된 경우).
  */
 void _Close_all(void)
 {
   // 표준 스트림 닫기 (여기서 __close는 아무 작업도 하지 않음)
   for (int i = 0; i < MAX_HANDLES; ++i)
   {
     __close(i);
   }
   // 파일 지원이 구현된 경우 실제 파일 닫기 로직 추가
 }



void __exit (int status)
{
  vTaskSuspendAll();
  __disable_irq();
  reset_system("__exit");
  while (1)
  {
    
  }
}
