#pragma once
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../TCPServer.h"

class TCPPollServer : public TCPServer
{
public:
    TCPPollServer();
    virtual ~TCPPollServer();

    // 추상 클래스에서 요구한 순수 가상 함수 구현
    bool Init(int port) override;       // 소켓 초기화 + 바인딩
    void Run() override;                // select 루프
    void Shutdown() override;          // 연결 종료 및 자원 정리

private:
    std::unordered_map<SOCKET, bool> hasNickname;
    std::vector<WSAPOLLFD> pollfds;

    void HandleNewConnection();                     // 새 클라이언트 연결 처리
    void HandleClientMessage(SOCKET clientSocket, size_t& index);  // 기존 클라이언트로부터 메시지 수신

    virtual void Broadcast(const std::string& message, SOCKET sender);
};