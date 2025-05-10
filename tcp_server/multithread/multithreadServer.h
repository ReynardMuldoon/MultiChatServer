#pragma once 
#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <string>
#include <unordered_map>
#include "../TCPServer.h"

class TCPMultithreadServer : public TCPServer
{
public: 
    TCPMultithreadServer();
    virtual ~TCPMultithreadServer();

    // 추상 클래스에서 요구한 순수 가상 함수 구현
    bool Init(int port) override;       // 소켓 초기화 + 바인딩
    void Run() override;                // select 루프
    void Shutdown() override;          // 연결 종료 및 자원 정리

private:
    std::vector<SOCKET> clients;
    std::mutex clients_mutex;

    void HandleClient(SOCKET clientSocket);

    virtual void RemoveClient(SOCKET clientSocket);
    virtual void Broadcast(const std::string& message, SOCKET sender);
};
