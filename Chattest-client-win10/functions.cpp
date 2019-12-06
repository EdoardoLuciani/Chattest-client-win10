#include "stdafx.h"
#include "functions.h"
#include "global_variables.h"

SOCKET connect_to_server(const char* ip_addr, const char* default_port) {
	struct addrinfo* result = NULL, * ptr = NULL, hints;
	int iResult;

	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed with error: " << iResult;
		return -1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(ip_addr, default_port, &hints, &result);
	if (iResult != 0) {
		std::cout << "getaddrinfo failed with error: " << WSAGetLastError();
		WSACleanup();
		return -1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ConnectSocket == INVALID_SOCKET) {
			std::cout << "socket failed with error: " << WSAGetLastError();
			WSACleanup();
			return -1;
		}

		// Connect to server.
		iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(result);
	if (ConnectSocket == INVALID_SOCKET) {
		printf("Unable to connect to server!\n");
		WSACleanup();
		return -1;
	}
	return ConnectSocket;
}

bool check_socket_error(SOCKET room_socket, int return_value) {
	if (return_value == SOCKET_ERROR) {
		printf("Function failed with error: %d\n", WSAGetLastError());
		closesocket(room_socket);
		WSACleanup();
		std::terminate();
	}
	return true;
}

int join_room(SOCKET room_socket, std::string name) {
	int result;

	uint8_t start = 1;
	result = send(room_socket, reinterpret_cast<char*>(&start), sizeof(uint8_t), 0);
	check_socket_error(room_socket, result);

	int lenght_name = htonl(name.length());
	result = send(room_socket, reinterpret_cast<char*>(&lenght_name), sizeof(int), 0);
	check_socket_error(room_socket, result);

	result = send(room_socket, name.c_str(), name.length(), 0);
	check_socket_error(room_socket, result);

	char welcome_code;
	result = recv(room_socket, &welcome_code, sizeof(char), 0);
	if (welcome_code == 5) {
		std::cout << "Successfully connected" << std::endl;
	}
	else if (welcome_code == 2) {
		return -1;
	}

	int receive_message_lenght;
	result = recv(room_socket, reinterpret_cast<char*>(&receive_message_lenght), sizeof(int), 0);
	check_socket_error(room_socket, result);
	receive_message_lenght = ntohl(receive_message_lenght);

	uint8_t* connect_info = new uint8_t[receive_message_lenght];
	result = recv(room_socket, reinterpret_cast<char*>(connect_info), receive_message_lenght, 0);
	check_socket_error(room_socket, result);

	int room_name_lenght = (connect_info[0] << 24) | (connect_info[1] << 16) | (connect_info[2] << 8) | connect_info[3];

	room_info chat_room;
	chat_room.room_name.assign(reinterpret_cast<char*>(connect_info + 4), room_name_lenght);
	chat_room.admin_name.assign(reinterpret_cast<char*>(connect_info + room_name_lenght + 4), (receive_message_lenght - room_name_lenght - 4));

	std::cout << "Room name: " << chat_room.room_name << std::endl;
	std::cout << "Admin name: " << chat_room.admin_name << std::endl;

	delete[] connect_info;
	return 0;
}

void send_message(SOCKET room_socket, std::string message) {
	int result;

	uint8_t message_start = 3;
	result = send(room_socket, reinterpret_cast<char*>(&message_start), sizeof(uint8_t), 0);
	check_socket_error(room_socket, result);

	int message_lenght = htonl(message.length());
	result = send(room_socket, reinterpret_cast<char*>(&message_lenght), sizeof(int), 0);
	check_socket_error(room_socket, result);

	result = send(room_socket, message.c_str(), message.length(), 0);
	check_socket_error(room_socket, result);
}

void receive_message(SOCKET room_socket) {
	int result;
	u_long bytes_to_read;

	while (proceed) {
		Sleep(10);
		bytes_to_read = 0;
		ioctlsocket(room_socket, FIONREAD, &bytes_to_read);
		if (bytes_to_read != 0) {
			char receive_code;
			result = recv(room_socket, &receive_code, sizeof(char), 0);
			if (receive_code == 3) {
				int receive_message_lenght;
				result = recv(room_socket, reinterpret_cast<char*>(&receive_message_lenght), sizeof(int), 0);
				check_socket_error(room_socket, result);
				receive_message_lenght = ntohl(receive_message_lenght);

				uint8_t* message = new uint8_t[receive_message_lenght + 1];
				result = recv(room_socket, reinterpret_cast<char*>(message), receive_message_lenght, 0);
				check_socket_error(room_socket, result);

				std::cout << std::string_view(reinterpret_cast<char*>(message), receive_message_lenght) << std::endl;
				delete[] message;
			}
			else if (receive_code == 4) {
				int receive_message_lenght;
				result = recv(room_socket, reinterpret_cast<char*>(&receive_message_lenght), sizeof(int), 0);
				check_socket_error(room_socket, result);
				receive_message_lenght = ntohl(receive_message_lenght);

				uint8_t* message = new uint8_t[receive_message_lenght + 1];
				message[receive_message_lenght] = '\0';
				result = recv(room_socket, reinterpret_cast<char*>(message), receive_message_lenght, 0);
				check_socket_error(room_socket, result);

				int sender_name_lenght = (message[0] << 24) | (message[1] << 16) | (message[2] << 8) | message[3];

				std::cout << std::string_view(reinterpret_cast<char*>(message + 4), sender_name_lenght) << ": "
					<< std::string_view(reinterpret_cast<char*>(message + 4 + sender_name_lenght), receive_message_lenght - sender_name_lenght - 4) << std::endl;

				delete[] message;
			}
		}
	}
}

void close_connection(SOCKET room_socket, std::string last_message) {
	int result;
	if (!last_message.empty()) {
		send_message(room_socket, last_message);
	}
	result = shutdown(room_socket, SD_BOTH);
	check_socket_error(room_socket, result);
}