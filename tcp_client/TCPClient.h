#pragma once 
#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ConsoleChatUI.h"

class TCPClient {
public:
	~TCPClient();

	bool Init(int port);
	void Run();
	void Shutdown();

private:
	static const int DEFAULT_BUFFER_LEN = 1024;

	SOCKET			clientSocket;
	ConsoleChatUI	ui;
	std::string		nickname;
	bool			running;
	int				port;

	void ReceiveMessages();
	void SetNickname();
};