#include "serverActions.h"
#define COMMAND_TO_INT(COMMAND, START) (((*COMMAND)[START])<<24 | ((*COMMAND)[START + 1])<<16 | ((*COMMAND)[START +2])<<8 | ((*COMMAND)[START + 3]))
#define INT_TO_COMMAND(VALUE, COMMAND, START)\
(*COMMAND)[START] = (VALUE >> 24) & 0xFF;\
(*COMMAND)[START + 1] = (VALUE >> 16) & 0xFF;\
(*COMMAND)[START + 2] = (VALUE >> 8) & 0xFF;\
(*COMMAND)[START + 3] =  VALUE & 0xFF;\

static void onCONNECT(action_t *command) {
	TCPSendStatus(initDriver((*command)[2] - '0'));//инициализирует скорость шины CAN
}

static void onCAN(action_t *command){//start / stop CAN emulation
	setDeviceCount((*command)[2]);// 1 - 127
	TCPSendStatus(
			(*command)[1] == '1' ?
					setCANState(CAN_start) : setCANState(CAN_stop));

}
static void onRECORD(action_t *command){//start / stop record on sdCard
	if((*command)[1] == '1'){
		openFileForCAN((char *)command);
		startRecord((*command)[2]);//параметр - максимальный размер файла
		TCPSendStatus(true);
	}else if((*command)[1] == '0'){
		stopRecord();
		closeFileForCAN();
		TCPSendStatus(true);
	} else
		TCPSendStatus(false);
}

static void onMONITOR(action_t *command) {//start / stop monitor
	(*command)[1] == '1' ? startMonitor() : stopMonitor();
	TCPSendStatus(true);
}

static void onFILE(action_t *command) {
	if ((*command)[1] == '0') { // 0 = get file list
		sendFileList();
	}
	if ((*command)[1] == '1') { // 1 = file remove
		fileRemove((char*) command);
	}
	TCPSendStatus(true);
}
static void onSETTINGS(action_t *command){
	ESP_LOGE("TCP", "onSETTINGS");
	wifi_config_t settings;
	bool s = false;
	for(uint8_t i = 1, j = 0; i<strlen((char*)command); i++){
		if((*command)[i] == '|'){
			s = true;
			settings.ap.ssid[i-1] = '\0';
			continue;
		}
		if(!s){
			settings.ap.ssid[i-1] = (*command)[i];
		}else{
			settings.ap.password[j] = (*command)[i];
			settings.ap.password[j+1] = '\0';
			j++;
		}
	}
	ESP_LOGE("TCP", "SET SETTINGS ssid:%s \t\npass:%s", settings.ap.ssid, settings.ap.password);
	if(setSettings(&settings)){
		TCPSendStatus(true);
		esp_restart();
	}
}
static serverActions_t actions = {
		.onCONNECT = onCONNECT,
		.onCAN = onCAN,
		.onRECORD = onRECORD,
		.onMONITOR = onMONITOR,
		.onFILE = onFILE,
		.onSETTINGS = onSETTINGS,
};
serverActions_t* getServerActionsDefault(){
	return &actions;
};
