#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "dev_io.h"
#define printf io_printf
// 테스트할 메모리 크기 (4MB)
#define MEM_SIZE (4 * 1024 * 1024)
// 테스트할 메모리 시작 주소 (★★★★★ 실제 하드웨어 주소로 변경 필수 ★★★★★)
#define MEM_ADDRESS 0x64000000 // 예시 주소 (STM32 FSMC 등)

// 메모리 테스트 함수 (volatile 포인터 사용)
// ptr: 테스트할 메모리 시작 주소 (volatile)
// size: 테스트할 메모리 크기 (바이트)
// 반환값: 0 성공, -1 실패
int memory_test_mapped(volatile unsigned char *ptr, size_t size) { // volatile 추가
    size_t i;
    unsigned char pattern;
    unsigned char read_val;

    // 포인터 유효성 검사는 이 시나리오에서는 생략 (하드웨어 주소 가정이므로)
    printf("Starting memory test for %zu bytes at mapped address 0x%lX...\n\r",
           size, (unsigned long)ptr); // 주소값 직접 출력

    // --- 테스트 패턴 1: 주소 기반 패턴 쓰기 (i % 256) ---
    printf("Phase 1: Writing address-based pattern (i %% 256)...\n\r");
    for (i = 0; i < size; ++i) {
        pattern = (unsigned char)(i % 256);
        ptr[i] = pattern; // Volatile write
        if ((i & 0xFFFFF) == 0xFFFFF) {
             printf("  Wrote up to address 0x%lX\n\r", (unsigned long)(&ptr[i]));
        }
    }
    printf("  Write phase complete.\n\r");

    // --- 테스트 패턴 1: 검증 ---
    printf("Phase 2: Verifying address-based pattern...\n\r");
    for (i = 0; i < size; ++i) {
        pattern = (unsigned char)(i % 256);
        read_val = ptr[i]; // Volatile read

        if (read_val != pattern) {
            fprintf(stderr, "\n\rERROR: Memory mismatch at address 0x%lX (offset %zu)!\n\r",
                    (unsigned long)(&ptr[i]), i);
            fprintf(stderr, "  Expected: 0x%02X\n\r", pattern);
            fprintf(stderr, "  Actual:   0x%02X\n\r", read_val);
            return -1;
        }
        if ((i & 0xFFFFF) == 0xFFFFF) {
            printf("  Verified up to address 0x%lX\n\r", (unsigned long)(&ptr[i]));
        }
    }
    printf("  Verification complete.\n\r");
    printf("Phase 1 & 2 (Address-based pattern) PASSED.\n\r\n\r");


    // --- 테스트 패턴 2: 0xAA 패턴 쓰기 및 검증 ---
    printf("Phase 3: Writing 0xAA pattern...\n\r");
    // memset은 volatile 포인터에 직접 사용하기 어려울 수 있음 -> 루프 사용 권장
    for (i = 0; i < size; ++i) ptr[i] = 0xAA;
    // 또는 volatile을 고려한 memset 구현 필요
    printf("  Write phase complete.\n\r");

    printf("Phase 4: Verifying 0xAA pattern...\n\r");
    for (i = 0; i < size; ++i) {
        read_val = ptr[i];
        if (read_val != 0xAA) {
             fprintf(stderr, "\n\rERROR: Memory mismatch at address 0x%lX (offset %zu)!\n\r",
                    (unsigned long)(&ptr[i]), i);
            fprintf(stderr, "  Expected: 0xAA\n\r");
            fprintf(stderr, "  Actual:   0x%02X\n\r", read_val);
            return -1;
        }
         if ((i & 0xFFFFF) == 0xFFFFF) {
            printf("  Verified up to address 0x%lX\n\r", (unsigned long)(&ptr[i]));
        }
    }
    printf("  Verification complete.\n\r");
    printf("Phase 3 & 4 (0xAA pattern) PASSED.\n\r\n\r");

    // --- 테스트 패턴 3: 0x55 패턴 쓰기 및 검증 ---
    printf("Phase 5: Writing 0x55 pattern...\n\r");
    for (i = 0; i < size; ++i) ptr[i] = 0x55;
    printf("  Write phase complete.\n\r");

    printf("Phase 6: Verifying 0x55 pattern...\n\r");
    for (i = 0; i < size; ++i) {
        read_val = ptr[i];
        if (read_val != 0x55) {
             fprintf(stderr, "\n\rERROR: Memory mismatch at address 0x%lX (offset %zu)!\n\r",
                    (unsigned long)(&ptr[i]), i);
            fprintf(stderr, "  Expected: 0x55\n\r");
            fprintf(stderr, "  Actual:   0x%02X\n\r", read_val);
            return -1;
        }
        if ((i & 0xFFFFF) == 0xFFFFF) {
             printf("  Verified up to address 0x%lX\n\r", (unsigned long)(&ptr[i]));
        }
    }
    printf("  Verification complete.\n\r");
    printf("Phase 5 & 6 (0x55 pattern) PASSED.\n\r\n\r");

    printf("All memory test phases PASSED!\n\r");
    return 0; // 성공
}


int test_sram()

{
    // 메모리 주소로 직접 포인터 생성 (volatile 키워드 필수!)
    volatile unsigned char *memory_region = (volatile unsigned char *)MEM_ADDRESS;
    int result = -1;

    printf("Memory test target: Mapped region at 0x%lX, Size: %zu bytes\n\r",
           (unsigned long)memory_region, (size_t)MEM_SIZE);


    // 메모리 테스트 실행
    result = memory_test_mapped(memory_region, MEM_SIZE);

    // 메모리 매핑된 하드웨어 영역은 free() 하지 않습니다.

    if (result == 0) {
        printf("Overall Memory Test Result: SUCCESS\n\r");
        return 0;
    } else {
        printf("Overall Memory Test Result: FAILURE\n\r");
        return 1;
    }
}