// IOCP_Server.cpp
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <thread>
#include <mutex>
#include "../TCPServer.h"

#pragma comment(lib, "ws2_32.lib")

class TCPIOCPServer : public TCPServer {
public:
    TCPIOCPServer();
    ~TCPIOCPServer();

    bool Init(int port) override;
    void Run() override;
    void Shutdown() override;

protected:
    virtual bool CreateServerSocket(int port);
    void RemoveClient(SOCKET clientSocket);
    void Broadcast(const std::string& message, SOCKET sender) override;

private:
    static constexpr int MAX_BUFFER = 1024;
    static constexpr int WORKER_COUNT = 4;

    enum OperationType { OP_RECV, OP_SEND };

    struct PerIOContext {
        OVERLAPPED overlapped;
        WSABUF wsaBuf;
        char buffer[MAX_BUFFER];
        OperationType opType;
        SOCKET clientSocket = INVALID_SOCKET;

        PerIOContext() {
            wsaBuf.buf = buffer;
            wsaBuf.len = MAX_BUFFER;
        }
    };

    struct ClientInfo {
        SOCKET socket;
        std::string nickname;
        bool isNicknameSet;
    };

    HANDLE iocpHandle = nullptr;

    std::vector<ClientInfo> clients;
    std::mutex clientsMutex;

    void WorkThread();
};