#include "SDCard.h"
#define MOUNT_POINT "/sdcard"

static bool isInit = false;
static sdmmc_card_t* card;

static bool CANFileAllow = false;

static FILE* settingsFile;
static FILE* CANFile;



bool initSDCard(){
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 3,
        .allocation_unit_size = 16 * 1024
    };
    const char mount_point[] = MOUNT_POINT;
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	spi_bus_config_t bus_cfg = { .mosi_io_num = 15, .miso_io_num = 2,
			.sclk_io_num = 14, .quadwp_io_num = -1, .quadhd_io_num = -1,
			.max_transfer_sz = 4000, };
	esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, 1);
	if (ret != ESP_OK) {
		ESP_LOGE("SDCARD", "Failed to initialize SPI.");
		return false;
	}
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = 13;
	slot_config.host_id = host.slot;
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			ESP_LOGE("SDCARD", "Failed to mount filesystem. " "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {
			ESP_LOGE("SDCARD", "Failed to initialize the card (%s). " "Make sure SD card lines have pull-up resistors in place.",
					esp_err_to_name(ret));
		}
		return false;
	}
	return isInit = true;
}

bool getSettings(wifi_config_t *settings) {
	return isInit
			&& (settingsFile = fopen(MOUNT_POINT"/settings.txt", "r")) != NULL
			&& fgets((char*)settings->ap.ssid, sizeof(settings->ap.ssid), settingsFile) != NULL
			&& fgets((char*)settings->ap.password, sizeof(settings->ap.password), settingsFile) != NULL
			&& fclose(settingsFile) == 0;
}
bool setSettings(wifi_config_t* settings){
	return isInit
			&& (settingsFile = fopen(MOUNT_POINT"/settings.txt", "w")) != NULL
			&& fprintf(settingsFile, "%s\0%s", settings->ap.ssid, settings->ap.password) > 0 //"%s\r\n%s"
			&& fclose(settingsFile) == 0;
}
bool writePacket(twai_message_t* packet) {
	if (isInit && CANFileAllow) {
		fprintf(CANFile, "%X\r\n", packet[0].identifier);
	    for(int i = 0; i<TWAI_FRAME_MAX_DLC; i++)
	    	fprintf(CANFile, "%X ", packet[0].data[i]);
	    fprintf(CANFile, "\r\n\r\n");
	    return true;
	}
	return false;
}

bool openFileForCAN(const char *filename) {
	char file[32];
	if(!CANFileAllow){
		snprintf(file, sizeof(file), "%s/%s", MOUNT_POINT, filename + 3);
		CANFile = fopen(file, "w");
		return CANFileAllow = (CANFile == NULL) ? false : true;
	}
	return false;
}

bool closeFileForCAN(){
	if(CANFileAllow && fclose(CANFile) == 0){
		CANFileAllow = false;
		return true;
	}
	return false;
}

void sendFileList() {
	DIR *dir = opendir(MOUNT_POINT"/");
	struct dirent *dp;
	while ((dp = readdir(dir)) != NULL) {
		TCPSendCurrentFilename(dp->d_name);
	}
	(void) closedir(dir);
	TCPSendCurrentFilename(NULL);
}

bool fileRemove(char *filename) {
	char file[32];
	uint8_t _size;
	while (filename[(_size = strlen(filename) - 1)] == ' ') {
		filename[_size] = '\0';
	}
	snprintf(file, sizeof(file), "%s/%s", MOUNT_POINT, filename + 2);
	struct stat st;
	if (stat(file, &st) == 0) {
		unlink(file);
		return true;
	}
	return false;
}
