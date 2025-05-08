#pragma once 
#include <winsock2.h>
#include <string>
#include <unordered_map>

class TCPServer {
public:
	virtual ~TCPServer() { }

	virtual bool Init(int port);
	virtual void Run() = 0;
	virtual void Shutdown();

protected:
	SOCKET serverSocket;
	std::unordered_map<SOCKET, std::string> clientNames;
	int port;

	std::string GetNickname(SOCKET clientSocket);
	virtual void Broadcast(const std::string& message, SOCKET sender) = 0;
	virtual void RemoveClient(SOCKET clientSocket) = 0;
};