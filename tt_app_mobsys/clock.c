#include <FreeRTOS.h>
#include <task.h>

#include "clock.h"

/* software clock task */
void task_clock(void *pvParameters)
{
  current_time = 0;
  while(1) {
    if (current_time != 0) {
      current_time++;
    }
    vTaskDelay(1000);
  }
  
  /* will never happen */
  vTaskDelete(NULL);
}
