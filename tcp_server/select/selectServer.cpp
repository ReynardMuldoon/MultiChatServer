// Server.cpp
#include "selectServer.h"
#include <iostream>
#include <algorithm>

TCPSelectServer::TCPSelectServer()
{
    FD_ZERO(&masterSet);
}

TCPSelectServer::~TCPSelectServer() 
{ 
    Shutdown();
}


bool TCPSelectServer::Init(int port) 
{
    TCPServer::Init(port);

    FD_SET(serverSocket, &masterSet);

    std::cout << "SELECT 서버 초기화 완료, 포트 " << port << "에서 대기 중..." << std::endl;
    return true;
}


void TCPSelectServer::Run() 
{
    while (true) 
    {
        fd_set copySet = masterSet;

        int socketCount = select(0, &copySet, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; ++i) 
        {
            SOCKET sock = copySet.fd_array[i];

            if (sock == serverSocket) {
                HandleNewConnection();
            }
            else {
                HandleClientMessage(sock);
            }
        }
    }
}

void TCPSelectServer::Shutdown() 
{
    for (int i = 0; i <= masterSet.fd_count; ++i) 
    {
        if (FD_ISSET(i, &masterSet)) 
        {
            closesocket(i);
        }
    }
    WSACleanup();
    std::cout << "서버 종료 완료" << std::endl;
}

void TCPSelectServer::HandleNewConnection() 
{
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);

    std::string nickname = GetNickname(clientSocket);
    clientNames[clientSocket] = nickname;

    // 해당 클라이언트 접속 알리기 
    std::string joinMsg = clientNames[clientSocket] + " has joined the chat.\n";
    Broadcast(joinMsg, clientSocket);
    std::cout << joinMsg;

    FD_SET(clientSocket, &masterSet);
}

void TCPSelectServer::HandleClientMessage(SOCKET clientSocket) 
{
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived <= 0) 
    {
        RemoveClient(clientSocket);
    }
    else {
        buffer[bytesReceived] = '\0';
        std::string msg = "[" + clientNames[clientSocket] + "]: " + buffer;
        std::cout << msg << std::endl;

        // 모든 클라이언트에게 메시지 전송
        Broadcast(msg, clientSocket);
    }
}

void TCPSelectServer::RemoveClient(SOCKET clientSocket) 
{
    TCPServer::RemoveClient(clientSocket);

    FD_CLR(clientSocket, &masterSet);
}

// 모든 클라이언트에게 메시지 전송
void TCPSelectServer::Broadcast(const std::string& message, SOCKET sender)
{
    for (u_int i = 0; i < masterSet.fd_count; ++i) 
    {
        SOCKET outSock = masterSet.fd_array[i];
        if (outSock != serverSocket && outSock != sender) 
        {
            send(outSock, message.c_str(), message.length(), 0);
        }
    }
}
