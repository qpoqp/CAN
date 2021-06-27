#include "CAN.h"
//static SemaphoreHandle_t txMutex;//xSemaphoreCreateMutex(); Mutex �� ����� ������ � CAN ����
static EventGroupHandle_t canEvents;//���������� ������� ����
static twai_timing_config_t t_config = { 0, 0, 0, 3, false };//������� ����
static const twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL(); //�� ����������� �����
static const twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(21, 22, TWAI_MODE_NO_ACK);//���� ���������������� �� ������� ��������� ������-���������� � ��� �����

static bool isWriting;
static bool isMonitor;
static int availableBytes = 0;//kbytes
static uint8_t deviceCount = 1;
static void CANtx(void *arg) { //TODO: �������� ������������ ���������� ��� ����������� �������� ���� ���������
	twai_message_t tx_msg = { .data_length_code = TWAI_FRAME_MAX_DLC };
	memset(tx_msg.data, 0, TWAI_FRAME_MAX_DLC);
	twai_status_info_t status_info;
	while (twai_get_status_info(&status_info) == ESP_OK) {//����������� ��������� �������� CAN. ESP_OK - ������� ���������������
		if (status_info.state == TWAI_STATE_RUNNING) {
			memset(tx_msg.data, 0, sizeof(tx_msg.data));
			tx_msg.identifier = esp_random() % deviceCount; //%2047 �.�. ������������� 11bit    esp_random() % 2047
			tx_msg.self = 1; //��� ������������� ��������
			for (int i = 0; i < TWAI_FRAME_MAX_DLC; i++) {
				tx_msg.data[i] = (esp_random() % 2) ? (esp_random() % 256) : 0; // � ������������ 50/50 �������������� i ���� �� 0 �� FF
			}
			ESP_ERROR_CHECK(twai_transmit(&tx_msg, portMAX_DELAY));
		}
		vTaskDelay(pdMS_TO_TICKS(100));
	}
	vTaskDelete(NULL);
}

static void CANrx(void *arg){
	twai_status_info_t status_info; //twai_get_status_info ��� (void*)0 ���������� ������ ESP_ERR_INVALID_ARG
	twai_message_t rx_message;
	while (twai_get_status_info(&status_info) == ESP_OK) { //����������� ��������� �������� CAN. ESP_OK - ������� ���������������
		ESP_ERROR_CHECK(twai_receive(&rx_message, portMAX_DELAY)); //���� ��������� �� CAN ����
		ESP_LOGI("CAN", "ID:%X", rx_message.identifier); //��������� ������
		for (uint8_t i = 0; i < rx_message.data_length_code; i++)
			printf("%X  ", rx_message.data[i]);
		printf("\n");
		if (isWriting && availableBytes > 0){//���� �������� ������� ���������� � ����
			writePacket(&rx_message);//���� ����� ������ ��� ������ ������ ����������� ���������� �� ����������� ������
			availableBytes -= 30;
		}else{
			stopRecord();
		}
		if(isMonitor)
			isMonitor = TCPSendCanPacket(&rx_message);//���� �������� ������� ���������� ������ ��������� �������
	}
	vTaskDelete(NULL); //������� CAN ������
}
static void CANController(void *arg){
	EventBits_t bits;
	while (1) {
		bits = xEventGroupWaitBits(canEvents, CAN_start | CAN_stop | CAN_uninstall, pdFALSE, pdFALSE, portMAX_DELAY);
		if ((bits & CAN_start) == CAN_start) {
			ESP_LOGE("CAN", "twai_start()");
			ESP_ERROR_CHECK(twai_start());//��������� twai_status_info_t.state � TWAI_STATE_RUNNING
		}
		if ((bits & CAN_stop) == CAN_stop) {
			ESP_LOGE("CAN", "twai_stop()");
			twai_stop();
		}
		if((bits & CAN_uninstall) == CAN_uninstall){
			twai_stop();
			isMonitor = false;
			isWriting = false;
			availableBytes = 0;
			closeFileForCAN();
			ESP_ERROR_CHECK(twai_driver_uninstall());
			xEventGroupClearBits(canEvents, CAN_start | CAN_stop | CAN_uninstall);
			vTaskDelete(NULL);
		}
		xEventGroupClearBits(canEvents, CAN_start | CAN_stop | CAN_uninstall);
	}
}

static bool setCANSpeed(int speed) {
	t_config.tseg_1 = 15;
	t_config.tseg_2 = 4;
	if (speed == 1)//50KBIT/S
		t_config.brp = 80;
	if (speed == 2)//100KBIT/S
		t_config.brp = 40;
	if (speed == 3)//125KBIT/S
		t_config.brp = 32;
	if (speed == 4)//250KBIT/S
		t_config.brp = 16;
	if (speed == 5)//500KBIT/S
		t_config.brp = 8;
	if (speed == 7)//1000KBIT/S
		t_config.brp = 4;
	if (speed == 6) {//800KBIT/S
		t_config.brp = 4;
		t_config.tseg_1 = 16;
		t_config.tseg_2 = 8;
	}
	if (speed == 0) {//25KBIT/S
		t_config.brp = 128;
		t_config.tseg_1 = 16;
		t_config.tseg_2 = 8;
	}
	if (t_config.brp)
		return true;
	return false;
}

bool initDriver(uint8_t speed){
	//��� ����� �������� ��� ������� ��������� �� twai_status_info_t
	//�.�. twai_get_status_info ������������� ��������� ������������� ��������
	//ESP_ERR_INVALID_STATE �������� �� �������� �� ����������
	if (twai_get_status_info(NULL) == ESP_ERR_INVALID_STATE && setCANSpeed(speed)) {
		canEvents = xEventGroupCreate();
		ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
		ESP_LOGI("CAN", "DRIVER INSTALLED");
		xTaskCreate(CANController, "can_controller", 4096, NULL, 6, NULL);
		xTaskCreate(CANrx, "can_rx", 4096, NULL, 5, NULL);
		xTaskCreate(CANtx, "can_tx", 4096, NULL, 5, NULL);
		return true;
	}
	return false;
}

bool setCANState(CANState state) {
	twai_status_info_t status_info;	//twai_get_status_info ��� (void*)0 ���������� ������ ESP_ERR_INVALID_ARG
	if (twai_get_status_info(&status_info) == ESP_OK) {//CAN ��������������� ?
		xEventGroupSetBits(canEvents, state);
		return true;
	}
	return false;
}

bool startRecord(uint8_t maxSize){//N kbytes
	twai_status_info_t status_info;	//twai_get_status_info ��� (void*)0 ���������� ������ ESP_ERR_INVALID_ARG
	ESP_LOGE("RECORD", "fileSize: %d", maxSize);
	if (twai_get_status_info(&status_info) == ESP_OK) {	//CAN ��������������� ?
		availableBytes = maxSize * 1024;//max 8738 ������� � 1 �����
		return isWriting = true;
	}
	return false;
}

bool stopRecord() {
	isWriting = false;
	availableBytes = 0;
	closeFileForCAN();
	return true;
}
void startMonitor(){
	isMonitor = true;
}
void stopMonitor(){
	isMonitor = false;
}
void setDeviceCount(uint8_t count){
	deviceCount = count;
}
