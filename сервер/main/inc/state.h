#ifndef __STATE__
#define __STATE__
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
#include "SDCard.h"
#include "serverActions.h"
typedef void(*event)();//события wifi EventGroupHandle_t * state
void wifi_init_hot(wifi_config_t* __wifi, event _onConnect, event _onDisconnect);


#endif
