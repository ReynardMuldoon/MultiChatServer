#pragma once
#include "../TCPServer.h"
#include <winsock2.h>
#include <vector>
#include <string>
#include <unordered_map>

class TCPSelectServer : public TCPServer {
public:
    virtual ~TCPSelectServer();

    virtual void Run();

private:
    fd_set masterSet;

    virtual void RemoveClient(SOCKET clientSocket);
    virtual void HandleNewConnection();
    virtual void HandleClientMessage(SOCKET clientSocket);
    virtual void Broadcast(const std::string& message, SOCKET sender);
};
