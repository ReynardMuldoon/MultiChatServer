#pragma once 
#include <winsock2.h>
#include <string>
#include <unordered_map>

class Server
{
private:
	std::unordered_map<SOCKET, std::string> clientNames;
public:

private:

};