#pragma once
#include "../TCPServer.h"
#include <winsock2.h>
#include <vector>
#include <string>
#include <unordered_map>

class TCPSelectServer : public TCPServer {
public:
    TCPSelectServer();
    virtual ~TCPSelectServer();

    // 추상 클래스에서 요구한 순수 가상 함수 구현
    bool Init(int port) override;       // 소켓 초기화 + 바인딩
    void Run() override;                // select 루프
    void Shutdown() override;          // 연결 종료 및 자원 정리

private:
    fd_set masterSet;

    virtual void RemoveClient(SOCKET clientSocket);
    virtual void HandleNewConnection();
    virtual void HandleClientMessage(SOCKET clientSocket);
    virtual void Broadcast(const std::string& message, SOCKET sender);
};
