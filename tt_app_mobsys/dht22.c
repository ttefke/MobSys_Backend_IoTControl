#ifdef DHT22

#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>
#include <string.h>

#include <bl_gpio.h>
#include <bl_timer.h>

#ifndef DHT22_DATA_PIN
#define DHT22_DATA_PIN 14
#endif

#define VOLTAGE_HIGH 1
#define VOLTAGE_LOW 0

#define ENABLE_PULLUP 1
#define DISABLE_PULLUP 0

#define ENABLE_PULLDOWN 1
#define DISABLE_PULLDOWN 0

volatile double dht22_rh = 0;
volatile double dht22_tmp = 0;

void task_dht22(void *pvParameters) {
  while (1) {
    uint8_t measurement_data[40];
    uint8_t data[5];
    uint8_t sensor_value;
    short sensor_readings = 0;
    
    memset(measurement_data, 0, 40);
    memset(data, 0, 5);
    
    // set data pin as output
    bl_gpio_enable_output(DHT22_DATA_PIN, DISABLE_PULLUP, DISABLE_PULLDOWN);
    bl_gpio_output_set(DHT22_DATA_PIN, VOLTAGE_HIGH);
    bl_timer_delay_us(10 * 1000);
    
    // send start signal: set low
    bl_gpio_output_set(DHT22_DATA_PIN, VOLTAGE_LOW);
    
    // wait for at least 1ms
    bl_timer_delay_us(1250);
    
    // set high
    bl_gpio_output_set(DHT22_DATA_PIN, VOLTAGE_HIGH);
    
    // wait 40 us
    bl_timer_delay_us(40);
    
    // set data pin as input for reading the sensor's response
    bl_gpio_enable_input(DHT22_DATA_PIN, DISABLE_PULLUP, DISABLE_PULLDOWN);
    
    // read sensor's response
    do {
      sensor_readings++;
      bl_timer_delay_us(1);
      bl_gpio_input_get(DHT22_DATA_PIN, &sensor_value);
    } while (sensor_value == VOLTAGE_LOW);

    // sensor pulls high for about 80 us
    sensor_readings = 0;
    do {
      sensor_readings++;
      bl_timer_delay_us(1);
      bl_gpio_input_get(DHT22_DATA_PIN, &sensor_value);
    } while (sensor_value == VOLTAGE_HIGH);
    
    // now we enter a process happening 40 times: we receive low for 50 ms and then high for either ~28 ms (0) or ~70ms (1)
    for (short i = 0; i < 40; i++) {
      // 50us low-voltage level indicating new measurement
      sensor_readings = 0;
      do {
        sensor_readings++;
        bl_timer_delay_us(1);
        bl_gpio_input_get(DHT22_DATA_PIN, &sensor_value);
      } while (sensor_value == VOLTAGE_LOW);
    
      // either 26-28 us -> 0 or 70 us -> 1
      sensor_readings = 0;
      do {
        sensor_readings++;
        bl_timer_delay_us(1);
        bl_gpio_input_get(DHT22_DATA_PIN, &sensor_value);
      } while (sensor_value == VOLTAGE_HIGH);
      
      if (sensor_readings <= 28) {
        measurement_data[i] = 0;
      } else {
        measurement_data[i] = 1;
      }
    }
    
    // put all measurements into one array
    for (short i = 0; i < 40; i++) {
      uint8_t data_nr = i/8;
      uint8_t data_pos = i % 8;
      uint8_t shifts = 7 - data_pos;
      uint8_t current_data = measurement_data[i]; 
      current_data = (current_data << shifts);
      uint8_t stored_data = data[data_nr];
      stored_data = (stored_data | current_data);
      data[data_nr] = stored_data;
    }
    
    // update values if measurements correct
    if (((data[0] + data[1] + data[2] + data[3]) &0xFF) == data[4]) {    
      uint16_t rh_tmp = (data[0] << 8);
      rh_tmp += data[1];
      dht22_rh = rh_tmp / 10.0;
      
      uint16_t t_tmp = (data[2] << 8);
      t_tmp += data[3];
      dht22_tmp = t_tmp / 10.0;
  
      printf("RH: %f\r\n", dht22_rh); 
      printf("TMP: %f\r\n", dht22_tmp);
    } else {
      printf("data is incorrect\r\n");
      printf("%d, %d, %d, %d, %d\r\n", data[0], data[1], data[2] ,data[3], data[4]);
    }
    vTaskDelay(30*1000);
  }
  
  vTaskDelete(NULL);
}
#endif
