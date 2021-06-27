#include "SDCard.h"
#define MOUNT_POINT "/sdcard"

static bool isInit = false;
static sdmmc_card_t* card;//информаци€ о SD/MMC карте

static bool CANFileAllow = false;

static FILE* settingsFile;//открываетс€ и закрываетс€ непосредственно при получении необходимого событи€
static FILE* CANFile;



bool initSDCard(){
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,//не форматировать sd карту в FAT32
        .max_files = 3,//максимальное кол-во одновременно открытых файлов
        .allocation_unit_size = 16 * 1024//размер кластера если необходимо форматирование
    };
    const char mount_point[] = MOUNT_POINT;//монтировать карту в дирректории /sdcard
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();//sd карта в spi режиме
	spi_bus_config_t bus_cfg = { .mosi_io_num = 15, .miso_io_num = 2,//установка пинов spi
			.sclk_io_num = 14, .quadwp_io_num = -1, .quadhd_io_num = -1,
			.max_transfer_sz = 4000, };
	esp_err_t ret = spi_bus_initialize(host.slot, &bus_cfg, 1);//инициализаци€ spi
	if (ret != ESP_OK) {
		ESP_LOGE("SDCARD", "Failed to initialize SPI.");
		return false;
	}
	//инициализаци€ слота sd карты (без определени€ наличи€ карты в слоте и защиты от записи)
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = 13;
	slot_config.host_id = host.slot;//номер используемого spi
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);//подключает новое SD SPI устройство к шине определенной в host
	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {//ќшибка монтировани€ файловой системы - формат файловой системы sd карты не FAT32
			ESP_LOGE("SDCARD", "Failed to mount filesystem. " "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		} else {//ошибка подключени€ самой sd карты (карта впринципе отсутствует или же сбит smd резистор на плате микроконтроллера)
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
	if(!CANFileAllow){//≈сли дескриптор CAN файла закрыт
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
