 /*
 * Implementation of CGI handlers and adoption of generated JSON to sensors use case.
 */

#include "lwip/apps/httpd.h"
#include "lwip/opt.h"

#include "lwip/apps/fs.h"
#include "lwip/def.h"
#include "lwip/mem.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"

#include "cgi.h"
#include "clock.h"


static const char *
cgi_handler_time(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
  if (iNumParams == 1) {
    if (!strcmp(pcParam[0], "time")) {
      current_time = strtol(pcValue[0], NULL, 10);
      return GET_DEVICE_TIME_ENDPOINT;
    }
  }
  return "/404.html";
}

static const tCGI cgi_handlers[] = {
  {
    SET_TIME_ENDPOINT_CGI,
    cgi_handler_time
  }
};


cJSON* create_sensor(int type, char* endpoint)
{
  cJSON *sensor = cJSON_CreateObject();
  cJSON_AddNumberToObject(sensor, "sensorType", type);
  cJSON_AddStringToObject(sensor, "endpoint", endpoint);
  return sensor;
}

cJSON* prepare_response(double data)
{
  /* create main response object */
  cJSON* response = cJSON_CreateObject();
  
  /* create data object */
  cJSON* response_data = cJSON_CreateObject();
  cJSON_AddNumberToObject(response_data, "sensor_data", data);
  
  /* annotate sensor data with time if available */
  if (current_time != 0) {
    cJSON_AddNumberToObject(response_data, "time", current_time);
  }
  
  cJSON_AddItemToObject(response, "data", response_data);
  cJSON_AddNumberToObject(response, "device_id", DEVICE_ID);
  return response;
}

/* opening (creating) the in real-time created file (page) */

int
fs_open_custom(struct fs_file *file, const char *name)
{
  cJSON *response;
  if (!strcmp(name, GET_WHOAMI_ENDPOINT)) {
    /* create basic JSON structure */
    response = cJSON_CreateObject();
  
    /* create data array and add sensor information */    
    cJSON *data = cJSON_CreateArray();
    
    cJSON *device_time = create_sensor(SENSOR_TYPE_DEVICE_TIME, GET_DEVICE_TIME_ENDPOINT);
    cJSON_AddItemToArray(data, device_time);

#ifdef LIGHT_SENSOR
    cJSON *light_sensor_analogous = create_sensor(SENSOR_TYPE_LIGHT_ANALOGOUS, GET_LIGHT_SENSOR_ANALOGOUS_ENDPOINT);
    cJSON_AddItemToArray(data, light_sensor_analogous);
#endif

#ifdef LOUDNESS_SENSOR
    cJSON *loudness_sensor_analogous = create_sensor(SENSOR_TYPE_LOUDNESS_ANALOGOUS, GET_LOUDNESS_SENSOR_ANALOGOUS_ENDPOINT);
    cJSON_AddItemToArray(data, loudness_sensor_analogous);
#endif

#ifdef DHT22
    cJSON *humidity_sensor_dht22 = create_sensor(SENSOR_TYPE_DHT22_HUMIDITY, GET_DHT22_SENSOR_HUMIDITY_ENDPOINT);
    cJSON_AddItemToArray(data, humidity_sensor_dht22);
    
    cJSON *temperature_sensor_dht22 = create_sensor(SENSOR_TYPE_DHT22_TEMPERATURE, GET_DHT22_SENSOR_TEMPERATURE_ENDPOINT);
    cJSON_AddItemToArray(data, temperature_sensor_dht22);
#endif

    /* put everything together */
    cJSON_AddItemToObject(response, "data", data);
    cJSON_AddNumberToObject(response, "device_id", DEVICE_ID);
  } else if (!strcmp(name, GET_DEVICE_TIME_ENDPOINT)) {
    response = prepare_response(current_time);
#ifdef LIGHT_SENSOR
  } else if (!strcmp(name, GET_LIGHT_SENSOR_ANALOGOUS_ENDPOINT)) {
    extern volatile int light_value;
    response = prepare_response(light_value);
#endif

#ifdef LOUDNESS_SENSOR
  } else if (!strcmp(name, GET_LOUDNESS_SENSOR_ANALOGOUS_ENDPOINT)) {
    extern volatile int loudness_value;
    response = prepare_response(loudness_value);
#endif

#ifdef DHT22
  } else if (!strcmp(name, GET_DHT22_SENSOR_HUMIDITY_ENDPOINT)) {
    extern volatile double dht22_rh;
    response = prepare_response(dht22_rh);
  } else if (!strcmp(name, GET_DHT22_SENSOR_TEMPERATURE_ENDPOINT)) {
    extern volatile double dht22_tmp;
    response = prepare_response(dht22_tmp);
#endif

  } else {
    /* send null if unknown URI */
    return 0;
  }
  
  /* send JSON response */
  char *encoded_response = cJSON_PrintUnformatted(response);
  int response_size = strlen(encoded_response);
  
  /* allocate memory */
  memset(file, 0, sizeof(struct fs_file));
  file->pextension = mem_malloc(response_size);
  if (file->pextension != NULL)
  {
    /* copy json to file handler */
    memcpy(file->pextension, encoded_response, response_size);
    file->data = (const char *)file->pextension;
    file->len = response_size;
    file->index = file->len;
    /* allow persisting connections */
    file->flags = FS_FILE_FLAGS_HEADER_PERSISTENT;
  }
  
  /* free no longer needed memory */
  cJSON_Delete(response);
  free(encoded_response);
  encoded_response = NULL;
  
  /* return whether data was sent */
  if (file->pextension != NULL) {
    return 1;
  } else {
    return 0;
  }
} 

/* closing the custom file (free the memory) */
void
fs_close_custom(struct fs_file *file)
{
  if (file && file->pextension) {
    mem_free(file->pextension);
    file->pextension = NULL;
  }
}

/* reading the custom file (nothing has to be done here, but function must be defined */
int
fs_read_custom(struct fs_file *file, char *buffer, int count)
{
  LWIP_UNUSED_ARG(file);
  LWIP_UNUSED_ARG(buffer);
  LWIP_UNUSED_ARG(count);
  return FS_READ_EOF;
}

/* initialization functions */
void
custom_files_init(void)
{
  printf("Initializing module for generating JSON output\r\n");
  /* Nothing to do as of now, should be initialized automatically */
}

void
cgi_init(void)
{
  printf("Initializing module for CGI\r\n");
  http_set_cgi_handlers(cgi_handlers, LWIP_ARRAYSIZE(cgi_handlers));
}
