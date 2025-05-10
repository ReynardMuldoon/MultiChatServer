#include "TCPServer.h"
#include <iostream>

bool TCPServer::Init(int port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) 
    {
        std::cerr << "WSAStartup 실패" << std::endl;
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "소켓 생성 실패" << std::endl;
        return false;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "bind 실패" << std::endl;
        closesocket(serverSocket);
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr << "listen 실패" << std::endl;
        closesocket(serverSocket);
        return false;
    }

    return true;
}

void TCPServer::Shutdown()
{
    closesocket(serverSocket);
    WSACleanup();
}

std::string TCPServer::GetNickname(SOCKET clientSocket)
{
    char buffer[1024];
    int bytesReceived;

    // 첫 번째 메시지는 닉네임 
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) 
    {
        closesocket(clientSocket);
        return "";
    }

    buffer[bytesReceived] = '\0';
    std::string nickname = buffer;

    return nickname;
}


void TCPServer::RemoveClient(SOCKET clientSocket)
{
    // 해당 클라이언트 접속 종료 알리기 
    std::string leaveMsg = clientNames[clientSocket] + " has left the chat.\n";
    Broadcast(leaveMsg, clientSocket);
    std::cout << leaveMsg;

    clientNames.erase(clientSocket);
    closesocket(clientSocket);
}
