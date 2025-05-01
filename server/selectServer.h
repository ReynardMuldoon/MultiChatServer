#pragma once
#include <winsock2.h>
#include <vector>
#include <string>
#include <unordered_map>

#pragma comment(lib, "ws2_32.lib")

class TCPSelectServer {
public:
    TCPSelectServer(int port);
    ~TCPSelectServer();

    void Run();

private:
    SOCKET serverSocket;
    fd_set masterSet;
    std::unordered_map<SOCKET, std::string> clientNames;
    int port;

    void InitWinsock();
    void SetupServerSocket();
    void HandleNewConnection();
    void HandleClientMessage(SOCKET clientSocket);
    void RemoveClient(SOCKET clientSocket);
    void Broadcast(const std::string& message, SOCKET sender);
};
