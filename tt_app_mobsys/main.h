#ifdef LIGHT_SENSOR
#define LIGHT_STACK_SIZE 256
#endif

#ifdef LOUDNESS_SENSOR
#define LOUDNESS_STACK_SIZE 256
#endif

#ifdef DHT22
#define DHT22_STACK_SIZE 384
#endif

#define TASK_MAX_PRIORITY configMAX_PRIORITIES - 1
