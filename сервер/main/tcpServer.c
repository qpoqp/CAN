#include "serverActions.h"
static serverActions_t s_a;
static int32_t serverSocket = -1;
static int32_t clientSocket = -1;
static TaskHandle_t server_desc; //дескриптор задачи сервера
bool init_server(serverActions_t* serverActions) {
	if(serverActions == (void*)0){
		ESP_LOGE("SERVER", "SERVER ACTIONS ARE (void*)0");
		return false;
	}
	s_a = *serverActions;
	struct sockaddr_in dest_addr = {				//Настройка сервера
				.sin_family = AF_INET,				//Тип протокола сетевого уровня ipv4
				.sin_port = htons(5555),			//Порт сервера
				.sin_addr.s_addr = htonl(INADDR_ANY)//Сервер доступен для всех устройств, подключенных к точке доступа
		};
	//	socket 		- создает объект сокета с параметрами используемых протоколов:
	//	AF_INET     - Использовать IPv4 на сетевом уровне
	//	SOCK_STREAM - Использовать TCP  на транспортном уровне
	//	IPPROTO_IP  - Создать соект для семества адресов IPv4
	serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (serverSocket < 0) {
		ESP_LOGE("SERVER", "serverSocket < 0");
		return false;
	}
	//	bind - присоединяет объект сокета к определенному ip адресу и порту
	if (bind(serverSocket, (struct sockaddr*) &dest_addr, sizeof(dest_addr)) != 0) {
		close(serverSocket);
		ESP_LOGE("SERVER", "bind < 0 ");
		return false;
	}
	//	listen - Порт готов принимать соединения, 1 - размер очереди соединений
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
	while (1) { //пользователь закрыл предыдущее соединение, но все еще не отключился от wifi
		struct sockaddr_storage source_addr; //ин-ия о клиенте
		uint addr_len = sizeof(source_addr);
		//accept - сервер переходит в режим ожидания подключения клиента
		clientSocket = accept(serverSocket, (struct sockaddr*) &source_addr, &addr_len); //Сокет клиента
		if (clientSocket < 0) {
			ESP_LOGE("SERVER", "CAN'T ACCEPT NEW CONNECTION: %d", errno);
			break;
		}
		ESP_LOGE("SERVER", "RECIEVED NEW CONNECTION");
		while (1) {
			memset(command, '\0', sizeof(command));
			if (recv(clientSocket, command, sizeof(command), 0) > 0) { //пустой пакет (len = 0) -> закрыть соединение
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
		//Пользователь отключился
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
	//1)метод таска 2)название задачи
	//3)размер стека для задачи 4)параметры для метода
	//5) приоритет потока 6) объект для хранения дескриптора
	return xTaskCreate(server_task, "tcp_server", 4096, NULL, 5, &server_desc); //Создает задачу TCP сервера
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
