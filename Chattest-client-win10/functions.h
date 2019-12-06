#pragma once

typedef struct room_info {
	std::string admin_name;
	std::string room_name;
}room_info;

SOCKET connect_to_server(const char* ip_addr, const char* default_port);

bool check_socket_error(SOCKET room_socket, int return_value);

int join_room(SOCKET room_socket, std::string name);

void send_message(SOCKET room_socket, std::string message);

void receive_message(SOCKET room_socket);

void close_connection(SOCKET room_socket, std::string last_message = std::string());
