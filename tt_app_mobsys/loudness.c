#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>

#include <bl602_adc.h>  //  For BL602 ADC Standard Driver
#include <bl_adc.h>     //  For BL602 ADC Hardware Abstraction Layer
#include <bl_dma.h>     //  For BL602 DMA Hardware Abstraction Layer

#ifdef LOUDNESS_SENSOR

#ifndef WITH_ADC
#error WITH_ADC required!
#endif

#include "adc.h"

volatile int loudness_value = -1;

void read_adc_loudness() {
  // array which will store the samples
  static uint32_t adc_data[ADC_SAMPLES];
  
  // get the DMA context for the ADC channel
  adc_ctx_t *ctx = bl_dma_find_ctx_by_channel(ADC_DMA_CHANNEL);
  
  // return if sampling did not finish
  if (ctx->channel_data == NULL) {
    return;
  }
  
  // copy read samples to static array
  memcpy(
    (uint8_t*) adc_data, //dest
    (uint8_t*) (ctx->channel_data), //src
    sizeof(adc_data) // size
  );
  
  // calc avg value
  uint32_t sum = 0;
  for (int i = 0; i < ADC_SAMPLES; i++) {
    // scale ADC sample
    uint32_t scaled = ((adc_data[i] & 0xFFFF) * 3200) >> 16;
    sum += scaled;
  }
  
  loudness_value = sum/ADC_SAMPLES;
}


void task_loudness(void *pvParameters) {
  // initialize ADC channel first
  init_adc();
  vTaskDelay(1500);
  
  while(1) {
    read_adc_loudness();
    vTaskDelay(500);
  }
  vTaskDelete(NULL);
}
#endif
