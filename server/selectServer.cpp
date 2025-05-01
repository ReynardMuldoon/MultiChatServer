// Server.cpp
#include "selectServer.h"
#include <iostream>
#include <algorithm>

TCPSelectServer::TCPSelectServer(int port) : port(port), serverSocket(INVALID_SOCKET) {}

TCPSelectServer::~TCPSelectServer() {
    closesocket(serverSocket);
    WSACleanup();
}

void TCPSelectServer::InitWinsock() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void TCPSelectServer::SetupServerSocket() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, SOMAXCONN);
}

void TCPSelectServer::Run() {
    InitWinsock();
    SetupServerSocket();

    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);

    std::cout << "Server listening on port " << port << "...\n";

    while (true) {
        fd_set copySet = masterSet;

        int socketCount = select(0, &copySet, nullptr, nullptr, nullptr);

        for (int i = 0; i < socketCount; ++i) {
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

void TCPSelectServer::HandleNewConnection() {
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);

    char buffer[1024];
    int bytesReceived;

    // 첫 번째 메시지는 닉네임 
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSocket);
        return;
    }

    buffer[bytesReceived] = '\0';
    std::string nickname = buffer;
    
    clientNames[clientSocket] = nickname;

    std::string joinMsg = nickname + " has joined the chat.\n";
    Broadcast(joinMsg, clientSocket);
    std::cout << joinMsg;

    FD_SET(clientSocket, &masterSet);
}

void TCPSelectServer::HandleClientMessage(SOCKET clientSocket) {
    char buffer[1024];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

    if (bytesReceived <= 0) {
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

void TCPSelectServer::RemoveClient(SOCKET clientSocket) {
    // 해당 클라이언트 접속 종료 알리기 
    std::string leaveMsg = clientNames[clientSocket] + " has left the chat.\n";
    Broadcast(leaveMsg, clientSocket);
    std::cout << leaveMsg;

    clientNames.erase(clientSocket);

    closesocket(clientSocket);
    FD_CLR(clientSocket, &masterSet);
}

// 모든 클라이언트에게 메시지 전송
void TCPSelectServer::Broadcast(const std::string& message, SOCKET sender)
{
    for (u_int i = 0; i < masterSet.fd_count; ++i) {
        SOCKET outSock = masterSet.fd_array[i];
        if (outSock != serverSocket && outSock != sender) {
            send(outSock, message.c_str(), message.length(), 0);
        }
    }
}
