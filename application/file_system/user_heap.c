
#include "user_heap.h"

#include "os_user_def.h"
#include "tlsf.h"
#include "bsp_board_mem.h"
static osSemaphoreId_t g_heap_sem;

// 확장 SRAM 없으면 사용 POOL_SIZE는 MCU SRAM 사이즈 맞게 조정필요
// static char memory_pool[POOL_SIZE];

char *g_ext_sram = (char *)TLSF_MEM_BASE;

tlsf_t tlsf_handle = NULL;

void user_tlsf_init(size_t size)
{
  OS_CREATE_BINARY_SEM(g_heap_sem);

  tlsf_handle = tlsf_create_with_pool(g_ext_sram, size);

  if (tlsf_handle == NULL)
  {
        // 초기화 실패 처리
        while (1);
  }
}


void *user_malloc(size_t size)
{
  void *mem=0;

  OS_PEND_SEM(g_heap_sem,osWaitForever);

  mem = (void *)tlsf_malloc(tlsf_handle, size);

  OS_POST_SEM(g_heap_sem);
  return mem;
}


void user_free(void *ptr)
{
  OS_PEND_SEM(g_heap_sem, osWaitForever);
  tlsf_free(tlsf_handle, ptr);
  OS_POST_SEM(g_heap_sem);
}