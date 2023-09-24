#ifndef TT_ADC_H
#define TT_ADC_H

#ifndef ADC_GAIN1
#define ADC_GAIN1 ADC_PGA_GAIN_1
#endif

#ifndef ADC_GAIN2
#define ADC_GAIN2 ADC_PGA_GAIN_1
#endif

#ifndef ADC_FREQUENCY
// must be between 500 and 16,000
#define ADC_FREQUENCY 500
#endif

#ifndef ADC_SAMPLES
#define ADC_SAMPLES 10
#endif

#ifndef ADC_PIN_GPIO
// supported GPIOS:  4, 5, 6, 9, 10, 11, 12, 13, 14, 15
#define ADC_PIN_GPIO 14
#endif

void init_adc();

#endif
