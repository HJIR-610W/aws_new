
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "io.h"
#include "main.h"
#include "task.h"


void PrintTaskList(void)
{
    char *buffer = NULL;
    
    
    buffer = aws_malloc(512);
    
    
    
    // 모든 태스크의 상태를 문자열로 가져옵니다.
    vTaskList(buffer);

    // 태스크의 상태를 출력합니다.
    debug_printf("Task Name\tState\tPriority\tStack\tTask Number\n");
    debug_printf("-------------------------------------------------\n");
    debug_printf("%s", buffer);
    
    
    aws_free(buffer);
}




void PrintRunTimeStats(void)
{
    char *buffer = NULL;
    
    
    buffer = aws_malloc(512);

    // 런타임 통계를 버퍼에 저장
    vTaskGetRunTimeStats(buffer);

    // 런타임 통계 출력
    debug_printf("Task Name\tExecution Time\tCPU Usage\n");
    debug_printf("-------------------------------------------------\n");
    debug_printf("%s", buffer);
    
        aws_free(buffer);
}


void PrintTaskDetails(const char *taskName) {
    TaskHandle_t taskHandle = xTaskGetHandle(taskName);

    if (taskHandle != NULL) {
        UBaseType_t stackSize = uxTaskGetStackHighWaterMark(taskHandle);
        debug_printf("Task Name: %10s ", taskName);
        debug_printf("  Remaining Stack Size: %u words\n", (unsigned int)stackSize);
    } else {
        debug_printf("Task Name: %10s - Handle not found\n", taskName);
    }
}

void ParseAndPrintTaskList(void) {
    char taskListBuffer[512]; // 태스크 리스트를 저장할 버퍼
    char *line;
    char taskName[configMAX_TASK_NAME_LEN + 1];

    // 태스크 정보를 가져옵니다.
    vTaskList(taskListBuffer);

    // 한 줄씩 파싱합니다.

    line = strtok(taskListBuffer, "\n"); // 첫 번째 줄 읽기
    while (line != NULL) {
        // 태스크 이름 파싱 (첫 번째 필드)
        sscanf(line, "%s", taskName);

        // 태스크 상세 정보 출력
        PrintTaskDetails(taskName);

        // 다음 줄로 이동
        line = strtok(NULL, "\n");
    }
}
void YourTimerInitFunction(void) {
    // Cortex-M의 DWT를 활성화

    if(CoreDebug->DEMCR &CoreDebug_DEMCR_TRCENA_Msk ==0)
    {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // DWT Enable
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // Cycle Counter Enable
    DWT->CYCCNT = 0;                                // Counter 초기화
    }
}

uint32_t YourTimerGetCounterValue(void) {
    return DWT->CYCCNT;  // Cycle Counter 값 반환
}