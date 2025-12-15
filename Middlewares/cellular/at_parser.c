#include "at_parser.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "FreeRTOS.h"
#include "cellular_api.h"
#include "dispatcher.h"
#include "cellular_hal.h"
#include "cmsis_os2.h"
#include "util_memory.h"
#include "system_err.h"


#define RING_QUEUE_DEPTH 4
#define SMS_QUEUE_DEPTH 4

static osThreadId_t s_task_id = NULL;
static at_parser_task_fn_t s_ring_task = NULL;
static at_parser_task_fn_t s_sms_task = NULL;
static osThreadId_t s_ring_task_id = NULL;
static osThreadId_t s_sms_task_id = NULL;
static osMessageQueueId_t s_ring_queue = NULL;
static osMessageQueueId_t s_sms_queue = NULL;

typedef struct {
  char payload[128];
} ring_event_t;

typedef struct {
  char payload[160];
} sms_event_t;

static void ring_task_worker(void *arg)
{
  (void)arg;
  while (1) {
    ring_event_t evt;
    if (osMessageQueueGet(s_ring_queue, &evt, NULL, osWaitForever) == osOK && s_ring_task) {
      s_ring_task(&evt);
    }
  }
}

static void sms_task_worker(void *arg)
{
  (void)arg;
  while (1) {
    sms_event_t evt;
    if (osMessageQueueGet(s_sms_queue, &evt, NULL, osWaitForever) == osOK && s_sms_task) {
      s_sms_task(&evt);
    }
  }
}

static void at_task(void *arg)
{
  uint8_t buffer[512];
  int32_t len;
  at_cmd_table_t* at_table;
  at_table = cellular_get_at_cmd_table();
  uint8_t find = 0;

  while (1)
  {
    len = cellular_recv_uart_at(buffer, sizeof(buffer), 1000);
    if (len <= 0 || at_table == NULL)
    {
      continue;
    }

    find = 0;

    for (size_t i = 0; i < at_table->count; i++) 
    {
      const at_comand_t *entry = &at_table->list[i];
  
      if (strncmp((char*)buffer, entry->cmd_string, strlen(entry->cmd_string)) == 0)
      {
        find = 1;
        switch (entry->cmd)
        {
        case AT_URC_RECV_TCP:
          cellular_tcp_recv_handler(buffer, len, sizeof(buffer));
          break;
        case AT_URC_RECV_RING:
          if (s_ring_queue)
          {
            ring_event_t evt = { 0 };
            size_t copy_len = (len < sizeof(evt.payload) - 1) ? (size_t)len : sizeof(evt.payload) - 1;
            memcpy(evt.payload, buffer, copy_len);
            (void)osMessageQueuePut(s_ring_queue, &evt, 0, 0);
            break;
          }
          break;
        case AT_URC_RECV_SMS:
          if (s_sms_queue)
          {
            sms_event_t evt = { 0 };
            size_t copy_len = (len < sizeof(evt.payload) - 1) ? (size_t)len : sizeof(evt.payload) - 1;
            memcpy(evt.payload, buffer, copy_len);
            (void)osMessageQueuePut(s_sms_queue, &evt, 0, 0);
            break;
          }
        default:
          find = 0;


          break;
        }
      }
      

    }
    if (find == 0)
    {
       DEBUG_PRINTF_LEVEL(LOG_LEVEL_DEBUG,"RAW:%s\r\n", buffer);
      dispatcher_handle_frame(buffer, len);
    }

  }
}

int at_parser_start(void)
{
  if (s_task_id) return 0;

  const osThreadAttr_t attr = { .name = "at_parser", .stack_size = TASK_STACK(TASK_CDMA_AT_DEF), .priority = (osPriority_t)TASK_PRIO(TASK_CDMA_AT_DEF) };
  s_task_id = osThreadNew(at_task, NULL, &attr);
  return (s_task_id != NULL) ? 0 : -1;
}

void at_parser_stop(void)
{
  if (s_task_id) {
    osThreadTerminate(s_task_id);
    s_task_id = NULL;
  }
  if (s_ring_task_id) {
    osThreadTerminate(s_ring_task_id);
    s_ring_task_id = NULL;
  }
  if (s_sms_task_id) {
    osThreadTerminate(s_sms_task_id);
    s_sms_task_id = NULL;
  }
  if (s_ring_queue) {
    osMessageQueueDelete(s_ring_queue);
    s_ring_queue = NULL;
  }
  if (s_sms_queue) {
    osMessageQueueDelete(s_sms_queue);
    s_sms_queue = NULL;
  }
}

void at_parser_set_ring_task(at_parser_task_fn_t fn)
{
  s_ring_task = fn;

  if (fn != NULL) {
    if (s_ring_queue == NULL) {
      const osMessageQueueAttr_t attr = { .name = "ring_evt" };
      s_ring_queue = osMessageQueueNew(RING_QUEUE_DEPTH, sizeof(ring_event_t), &attr);
    }
    if (s_ring_queue && s_ring_task_id == NULL) {
      const osThreadAttr_t attr = { .name = "ring_cb", .stack_size = 1024, .priority = osPriorityNormal };
      s_ring_task_id = osThreadNew(ring_task_worker, NULL, &attr);
    }
  }
}

void at_parser_set_sms_task(at_parser_task_fn_t fn)
{
  s_sms_task = fn;

  if (fn != NULL) {
    if (s_sms_queue == NULL) {
      const osMessageQueueAttr_t attr = { .name = "sms_evt" };
      s_sms_queue = osMessageQueueNew(SMS_QUEUE_DEPTH, sizeof(sms_event_t), &attr);
    }
    if (s_sms_queue && s_sms_task_id == NULL) {
      const osThreadAttr_t attr = { .name = "sms_cb", .stack_size = 1024, .priority = osPriorityNormal };
      s_sms_task_id = osThreadNew(sms_task_worker, NULL, &attr);
    }
  }
}

