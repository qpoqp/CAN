#ifndef __SERVER_ACTIONS__
#define __SERVER_ACTIONS__
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include <stdio.h>
#include "driver/gpio.h"
#include "CAN.h"

typedef enum {
	_CONNECT= 0, _CAN, _RECORD, _MONITOR, _FILE, _SETTINGS
}some;
typedef unsigned char action_t[32];
typedef void (*action)(action_t *command);
typedef struct {
	action onCONNECT;
	action onCAN;
	action onRECORD;
	action onMONITOR;
	action onFILE;
	action onSETTINGS;
} serverActions_t;
bool init_server(serverActions_t* serverActions);
int serverStart();
void closeClientConnection();

bool TCPSendStatus(bool state);
bool TCPSendCanPacket(twai_message_t* packet);
void TCPSendCurrentFilename(char *filename);

serverActions_t* getServerActionsDefault();
#endif
