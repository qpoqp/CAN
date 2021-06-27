#ifndef __SDCARD__
#define __SDCARD__

#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_wifi_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "driver/twai.h"
#include "state.h"
#include "esp_vfs.h"
bool initSDCard();

bool getSettings(wifi_config_t* settings);
bool setSettings(wifi_config_t* settings);

bool openFileForCAN(const char* filename);
bool closeFileForCAN();
bool writePacket(twai_message_t* packet);
void sendFileList();
bool fileRemove(char* filename);


#endif
