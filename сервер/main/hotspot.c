#include "state.h"

static event onConnect;
static event onDisconnect;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
	if (event_id == WIFI_EVENT_AP_STACONNECTED && onConnect) {
		onConnect();
	} else if (event_id == WIFI_EVENT_AP_STADISCONNECTED && onDisconnect) {
		onDisconnect();
	}
}
void wifi_init_hot(wifi_config_t* __wifi, event _onConnect, event _onDisconnect)
{
	onConnect = _onConnect;
	onDisconnect = _onDisconnect;

	ESP_ERROR_CHECK(esp_netif_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());
	esp_netif_t *wifiAP = esp_netif_create_default_wifi_ap();

	esp_netif_ip_info_t ipInfo;
	IP4_ADDR(&ipInfo.ip, 192, 168, 0, 1);
	IP4_ADDR(&ipInfo.gw, 192, 168, 0, 1);
	IP4_ADDR(&ipInfo.netmask, 255, 255, 255, 0);
	esp_netif_dhcps_stop(wifiAP);
	esp_netif_set_ip_info(wifiAP, &ipInfo);
	esp_netif_dhcps_start(wifiAP);

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));

	ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, __wifi));
	ESP_ERROR_CHECK(esp_wifi_start());
}
