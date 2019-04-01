#ifndef SAMPLER_FREERTOS_TASKS_H
#define SAMPLER_FREERTOS_TASKS_H

#include "queue.h"
#include "sampler.h"

#define cliNEW_LINE "\n\r"

#define LOAD_INSTRUMENT_TASK_NAME "load_instrument"
#define MAX_PATH_LEN               100

typedef struct {
    char file_path[MAX_PATH_LEN];
    xQueueHandle return_handle;
} file_path_t;

void notification_test_task( void *pvParameters );
void load_instrument_task( void *pvParameters );

#endif