#include "serverActions.h"
static serverActions_t s_a;
static int32_t serverSocket = -1;
static int32_t clientSocket = -1;
static TaskHandle_t server_desc;
bool init_server(serverActions_t* serverActions) {
	if(serverActions == (void*)0){
		ESP_LOGE("SERVER", "SERVER ACTIONS ARE (void*)0");
		return false;
	}
	s_a = *serverActions;
	struct sockaddr_in dest_addr = {
				.sin_family = AF_INET,
				.sin_port = htons(5555),
				.sin_addr.s_addr = htonl(INADDR_ANY)
		};
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (serverSocket < 0) {
		ESP_LOGE("SERVER", "serverSocket < 0");
		return false;
	}
	if (bind(serverSocket, (struct sockaddr*) &dest_addr, sizeof(dest_addr)) != 0) {
		close(serverSocket);
		ESP_LOGE("SERVER", "bind < 0 ");
		return false;
	}
	if (listen(serverSocket, 1) != 0) {
		close(serverSocket);
		ESP_LOGE("SERVER", "listen < 0");
		return false;
	}
	ESP_LOGE("SERVER", "SUCCESS: %d", serverSocket);
	return true;
}
static void server_task(void *pvParameters) {
	action_t command;
	while (1) {
		struct sockaddr_storage source_addr;
		uint addr_len = sizeof(source_addr);
		clientSocket = accept(serverSocket, (struct sockaddr*) &source_addr, &addr_len);
		if (clientSocket < 0) {
			ESP_LOGE("SERVER", "CAN'T ACCEPT NEW CONNECTION: %d", errno);
			break;
		}
		ESP_LOGE("SERVER", "RECIEVED NEW CONNECTION");
		while (1) {
			memset(command, '\0', sizeof(command));
			if (recv(clientSocket, command, sizeof(command), 0) > 0) {
				ESP_LOGE("SERVER", "NEW COMMAND:%s", command);
				switch (command[0] - '0') {
				case _CONNECT: {
					s_a.onCONNECT(&command);
					break;
				}
				case _CAN: {
					s_a.onCAN(&command);
					break;
				}
				case _RECORD: {
					ESP_LOGE("TCP", "_RECORD");
					s_a.onRECORD(&command);
					break;
				}
				case _MONITOR: {
					ESP_LOGE("TCP", "_MONITOR");
					s_a.onMONITOR(&command);
					break;
				}
				case _FILE: {
					ESP_LOGE("TCP", "_FILE");
					s_a.onFILE(&command);
					break;
				}
				case _SETTINGS:{
					ESP_LOGE("TCP", "_SETTINGS");
					s_a.onSETTINGS(&command);
					break;
				}
				default:
					break;
				}
			} else {
				ESP_LOGE("SERVER", "RECIEVED DISCONNECT");
				break;
			}
		}
		closeClientConnection();
	}
	close(serverSocket);
	vTaskDelete(NULL);
}
void closeClientConnection(){
	shutdown(clientSocket, 0);
	close(clientSocket);
}

int serverStart(){
	return xTaskCreate(server_task, "tcp_server", 4096, NULL, 5, &server_desc);
}
bool TCPSendStatus(bool state){
	return (clientSocket && send(clientSocket, state ? "S:1" : "S:0", 3, 0)) ?
			true : false;
}
bool TCPSendCanPacket(twai_message_t* packet){
	uint8_t out[9];
	out[0] = packet->identifier;
	for(int i = 0; i< 8; i++){
		out[i + 1] = packet->data[i];
	}
	return (clientSocket && send(clientSocket, &out, sizeof(out), 0)) ? true : false;
}

void TCPSendCurrentFilename(char *filename) {
	if (clientSocket) {
		if (filename == (void*) 0) {
			TCPSendStatus(true);
		} else {
			uint8_t _size = strlen(filename);
			while (_size < 10) {
				filename[_size] = ' ';
				_size++;
			}
			filename[_size+1] = '\0';
			send(clientSocket, filename, strlen(filename), 0);
			vTaskDelay(pdMS_TO_TICKS(100));
		}
	}
}
