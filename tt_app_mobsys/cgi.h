#ifndef TT_CGI_H
#define TT_CGI_H 1

/* show errors if dependencies are not included */

#if !LWIP_HTTPD_CUSTOM_FILES
#error This needs LWIP_HTTPD_CUSTOM_FILES
#endif

#if !LWIP_HTTPD_DYNAMIC_HEADERS
#error This needs LWIP_HTTPD_DYNAMIC_HEADERS
#endif

#if !LWIP_HTTPD_CGI
#error This needs LWIP_HTTPD_CGI
#endif

/* endpoints */

/* setters */
#define SET_TIME_ENDPOINT_CGI "/set_time"

/* getters */
#define GET_WHOAMI_ENDPOINT "/whoami.json"
#define GET_DEVICE_TIME_ENDPOINT "/device_time.json"

#ifdef LIGHT_SENSOR
#define GET_LIGHT_SENSOR_ANALOGOUS_ENDPOINT "/light_analogous.json"
#endif

#ifdef LOUDNESS_SENSOR
#define GET_LOUDNESS_SENSOR_ANALOGOUS_ENDPOINT "/loudness_analogous.json"
#endif

#ifdef DHT22
#define GET_DHT22_SENSOR_HUMIDITY_ENDPOINT "/humidity_dht22.json"
#define GET_DHT22_SENSOR_TEMPERATURE_ENDPOINT "/temperature_dht22.json"
#endif

/* mapping of sensor type <-> int value used for communication with app
   only change in accordance with app!,
   negative numbers indicate interfaces which return data previously set
   by the app*/
   
#define SENSOR_TYPE_DEVICE_TIME -1

#ifdef LIGHT_SENSOR
#define SENSOR_TYPE_LIGHT_ANALOGOUS 2
#endif

#ifdef LOUDNESS_SENSOR
#define SENSOR_TYPE_LOUDNESS_ANALOGOUS 3
#endif

#ifdef DHT22
#define SENSOR_TYPE_DHT22_HUMIDITY 4
#define SENSOR_TYPE_DHT22_TEMPERATURE 5
#endif

/* initialization functions */
void custom_files_init(void);
void cgi_init(void);

#endif
