// Server.cpp
#include "selectServer.h"
#include <iostream>
#include <algorithm>

TCPSelectServer::~TCPSelectServer() { }


void TCPSelectServer::Run() {
    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);

    std::cout << "TCP Select Server listening on port " << port << "...\n";

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

    std::string nickname = GetNickname(clientSocket);
    clientNames[clientSocket] = nickname;

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
    TCPServer::RemoveClient(clientSocket);
    clientNames.erase(clientSocket);

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
