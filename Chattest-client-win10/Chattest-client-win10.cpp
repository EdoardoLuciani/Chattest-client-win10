// Chattest-client-win10.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"
#include "functions.h"
#include "global_variables.h"


int __cdecl main(int argc, char** argv) {

	std::string ip_string,port_string;
	std::cout << "Enter Ip of the server: ";
	std::cin >> ip_string;

	std::cout << "Select port (empty for default: 7357): ";
	std::cin.ignore();
	std::getline(std::cin,port_string);

	if (port_string.empty()) { port_string.assign("7357"); }
	
	//151.15.88.61

	SOCKET socket = connect_to_server(ip_string.c_str(), port_string.c_str());
	if (socket == -1) {
		std::cout << "An error during connection occurred" << std::endl;
		return -1;
	}

	int join_successful;
	do {
		std::cout << "Insert your name: ";
		std::string name;
		std::getline(std::cin, name);
		join_successful = join_room(socket, name);
	} while (join_successful != 0);

	std::thread receive_thread(receive_message, socket);
	
	while (true) {
		std::string message;
		std::getline(std::cin, message);
		if (message.empty()) {
			std::cout << "Are you sure you want to quit? (Y/n): ";
			char yon;
			std::cin.get(yon);
			if (yon == 'y') { break; }
			std::cin.ignore();
		} 
		else {
			send_message(socket, message);
		}
	}
	
	close_connection(socket, "Goodbye!");
	proceed = false;
	receive_thread.join();

	closesocket(socket);
	WSACleanup();

	return 0;
}