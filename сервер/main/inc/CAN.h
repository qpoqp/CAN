#ifndef __CAN__
#define __CAN__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/twai.h"
#include "freertos/event_groups.h"
//#include "state.h"
//#include "state.h"
#include "serverActions.h"
#include "SDCard.h"

typedef enum {
	CAN_start = 1 << 0, CAN_stop = 1 << 1, CAN_uninstall = 1 << 2,
}CANState;

bool initDriver(uint8_t speed);
bool setCANState(CANState state);
bool startRecord(uint8_t maxSize);//начать запись шины, остановиться, когда размер файла = maxSize
bool stopRecord();//остановить запись вручную
void startMonitor();
void stopMonitor();
void setDeviceCount(uint8_t count);

#endif
