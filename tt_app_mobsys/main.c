#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include <bl_dma.h>
#include <bl_irq.h>
#include <bl_sec.h>
#include <bl_sys_time.h>
#include <bl_uart.h>
#include <hal_boot2.h>
#include <hal_board.h>
#include <hal_hwtimer.h>

#include <blog.h>
#include <lwip/tcpip.h>

#include "main.h"

/* Define heap regions */
extern uint8_t _heap_start;
extern uint8_t _heap_size;
extern uint8_t _heap_wifi_start;
extern uint8_t _heap_wifi_size;

static HeapRegion_t xHeapRegions[] =
{
  { &_heap_start, (unsigned int) &_heap_size},
  { &_heap_wifi_start, (unsigned int) &_heap_wifi_size },
  { NULL, 0},
  { NULL, 0}
};

/* main function, execution starts here */
void bfl_main(void)
{
  /* Define information containers for tasks */
  static StackType_t wifi_stack[1024];
  static StaticTask_t wifi_task;
  
  static StackType_t httpd_stack[512];
  static StaticTask_t httpd_task;
  
  static StackType_t clock_stack[128];
  static StaticTask_t clock_task;
  
#ifdef LIGHT_SENSOR
  static StackType_t light_stack[LIGHT_STACK_SIZE];
  static StaticTask_t light_task;
#endif

#ifdef LOUDNESS_SENSOR
  static StackType_t loudness_stack[LOUDNESS_STACK_SIZE];
  static StaticTask_t loudness_task;
#endif

#ifdef DHT22
  static StackType_t dht22_stack[DHT22_STACK_SIZE];
  static StaticTask_t dht22_task;
#endif
  /* Initialize UART
   * Ports: 16+7 (TX+RX)
   * Baudrate: 2 million
   */
  bl_uart_init(0, 16, 7, 255, 255, 2 * 1000 * 1000);
  printf("[SYSTEM] Starting up!\r\n");
  
  /* (Re)define Heap */
  vPortDefineHeapRegions(xHeapRegions);
  
  /* Initialize system */
  blog_init();
  bl_irq_init();
  bl_sec_init();
  bl_dma_init();
  hal_boot2_init();
#if DHT22
    bl_sys_time_cli_init();
    hal_hwtimer_init();
#endif
  hal_board_cfg(0);
  
  /* Start tasks */

  printf("[SYSTEM] Starting httpd task\r\n");
  extern void task_httpd(void *pvParameters);
  xTaskCreateStatic(task_httpd, (char*)"httpd", 512, NULL, 10, httpd_stack, &httpd_task);

  printf("[SYSTEM] Starting WiFi task\r\n");
  extern void task_wifi(void *pvParameters);
  xTaskCreateStatic(task_wifi, (char*)"wifi", 1024, NULL, 10, wifi_stack, &wifi_task);
  
  printf("[SYSTEM] Starting clock task\r\n");
  extern void task_clock(void *pvParameters);
  xTaskCreateStatic(task_clock, (char*)"software clock", 128, NULL, 16, clock_stack, &clock_task);

#ifdef LIGHT_SENSOR
  printf("[SYSTEM] Starting light sensor measurement task\r\n");
  extern void task_light(void *pvParameters);
  xTaskCreateStatic(task_light, (char*)"light sensor", LIGHT_STACK_SIZE, NULL, 15, light_stack, &light_task);
#endif

#ifdef LOUDNESS_SENSOR
  printf("[SYSTEM] Starting loudness sensor measurement task\r\n");
  extern void task_loudness(void *pvParameters);
  xTaskCreateStatic(task_loudness, (char*)"loudness sensor", LOUDNESS_STACK_SIZE, NULL, 15, loudness_stack, &loudness_task);
#endif
  
#ifdef DHT22
  printf("[SYSTEM] Starting dht22 sensor measurement task\r\n");
  extern void task_dht22(void *pvParameters);

  // Must be set to max priority in order to fulfill the real-time requirements of the sensor
  xTaskCreateStatic(task_dht22, (char*)"DHT22 sensor", DHT22_STACK_SIZE, NULL, TASK_MAX_PRIORITY, dht22_stack, &dht22_task);
#endif
  /* Start TCP/IP stack */
  printf("[SYSTEM] Starting TCP/IP stack\r\n");
  tcpip_init(NULL, NULL);
  
  /* Start scheduler */
  printf("[SYSTEM] Starting scheduler\r\n");
  vTaskStartScheduler();
}
