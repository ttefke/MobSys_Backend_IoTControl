#ifdef WITH_ADC

#include "adc.h"

#include <bl602_adc.h>  //  For BL602 ADC Standard Driver
#include <bl_adc.h>     //  For BL602 ADC Hardware Abstraction Layer
#include <bl_dma.h>     //  For BL602 DMA Hardware Abstraction Layer

static int set_adc_gain(uint32_t gain1, uint32_t gain2) {
  // read configuration hardware register
  uint32_t reg = BL_RD_REG(AON_BASE, AON_GPADC_REG_CONFIG2);
  
  // set ADC gain
  reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_PGA1_GAIN, gain1);
  reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_PGA2_GAIN, gain2);
  
  // set chop mode
  if (gain1 != ADC_PGA_GAIN_NONE || gain2 != ADC_PGA_GAIN_NONE) {
    reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_CHOP_MODE, 2);
  } else {
    reg = BL_SET_REG_BITS_VAL(reg, AON_GPADC_CHOP_MODE, 1);
  }
  
  // enable ADC PGA
  reg = BL_CLR_REG_BIT(reg, AON_GPADC_PGA_VCMI_EN);
  if (gain1 != ADC_PGA_GAIN_NONE || gain2 != ADC_PGA_GAIN_NONE) {
    reg = BL_SET_REG_BIT(reg, AON_GPADC_PGA_EN);
  } else {
    reg = BL_CLR_REG_BIT(reg, AON_GPADC_PGA_EN);
  }
  
  // update ADC configuration hardware register
  BL_WR_REG(AON_BASE, AON_GPADC_REG_CONFIG2, reg);
  return 0;
}

void init_adc() {
  // init frequency for single-channel-conversion mode
  bl_adc_freq_init(1, ADC_FREQUENCY);
  
  // init ADC GPIO for single channel conversion mode
  bl_adc_init(1, ADC_PIN_GPIO);
  
  // enable ADC gain
  set_adc_gain(ADC_GAIN1, ADC_GAIN2);
  
  // init DMA for the ADC channel for single channel conversion mode
  bl_adc_dma_init(1, ADC_SAMPLES);
  
  // configure the GPIO pin as ADC input, no pullup, no pulldown
  bl_adc_gpio_init(ADC_PIN_GPIO);
  
  // get the ADC channel number for the GPIO pin
  int channel = bl_adc_get_channel_by_gpio(ADC_PIN_GPIO);
  
  // get the DMA context for the ADC channel
  adc_ctx_t *ctx = bl_dma_find_ctx_by_channel(ADC_DMA_CHANNEL);
  
  // indicate that the GPIO has been configured for ADC
  ctx->chan_init_table |= (1 << channel);
  
  // start reading the ADC via DMA
  bl_adc_start();
}

#endif
