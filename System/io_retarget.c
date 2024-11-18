
#include "stm32f4xx_hal.h"



UART_HandleTypeDef huart2;

// POSIX 관련 상수 정의
#define EBADF           9   // Bad file descriptor
#define ENOENT          2   // No such file or directory
#define STDIN_FILENO    0   // Standard input
#define STDOUT_FILENO   1   // Standard output
#define STDERR_FILENO   2   // Standard error output

// off_t 타입을 정의 (일반적으로 파일 오프셋에 사용되며, 여기서는 int로 사용)
typedef int off_t;

// errno 변수 정의
static int errno;

extern UART_HandleTypeDef huart1;

#if 1 
// write 함수 재정의: STDOUT 및 STDERR에 대해 UART로 출력
int __write(int file, char *data, int len) {
    if (file != STDOUT_FILENO && file != STDERR_FILENO) {
        errno = EBADF; // 잘못된 파일 디스크립터
        return -1;
    }
    
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
    return (status == HAL_OK ? len : 0); // 성공적으로 전송한 바이트 수 반환
}
#endif
// lseek 함수 재정의: UART에서는 의미가 없으므로 0 반환
off_t __lseek(int file, off_t offset, int whence) {
    (void)file;
    (void)offset;
    (void)whence;
    return 0;
}

// remove 함수 재정의: 파일 시스템이 없으므로 항상 오류 반환
int remove(const char *name) {
    (void)name;
    errno = ENOENT; // 파일이 존재하지 않음을 나타냄
    return -1;
}

// close 함수 재정의: UART에는 파일 닫기 기능이 필요하지 않으므로 -1 반환
int __close(int file) {
    (void)file;
    return -1;
}

// read 함수 재정의: STDIN에 대해 UART에서 읽기
int __read(int file, char *data, int len) {
    if (file != STDIN_FILENO) {
        errno = EBADF;
        return -1;
    }

    HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, (uint8_t*)data, len, HAL_MAX_DELAY);
    return (status == HAL_OK ? len : 0); // 성공적으로 읽은 바이트 수 반환
}

// isatty 함수 재정의: 표준 입출력 파일 디스크립터인지 확인
int __isatty(int file) {
    return (file == STDOUT_FILENO || file == STDERR_FILENO || file == STDIN_FILENO) ? 1 : 0;
}



