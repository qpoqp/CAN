#include "state.h"
static void on_connect() {
}
static void on_disconnect() {
	ESP_LOGE("WIFI", "station disconnected");
	setCANState(CAN_uninstall);
	closeClientConnection();
}
void app_main(void) {
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	wifi_config_t __wifi = { .ap = { .channel = 1, .max_connection = 1,
			.authmode = WIFI_AUTH_WPA_WPA2_PSK } };
	if(initSDCard() && getSettings(&__wifi)){
		ESP_LOGE("MAIN", "INIT HOTSPOT FROM FILE\n SSID:%s\nPASS:%s", __wifi.ap.ssid, __wifi.ap.password);
	}else{
		ESP_LOGI("MAIN", "CAN'T INIT SD CARD");
		strcpy((char*) __wifi.ap.ssid, "CAR13");
		__wifi.ap.ssid_len = strlen((char*)__wifi.ap.ssid);
		strcpy((char*) __wifi.ap.password, "123456789");
	}
	wifi_init_hot(&__wifi, on_connect, on_disconnect);
	if(!init_server(getServerActionsDefault()) || serverStart() != pdPASS){
		ESP_LOGE("MAIN", "CAN'T INIT WIFI OR STATION");
		vTaskDelay(portMAX_DELAY);
	}
}
