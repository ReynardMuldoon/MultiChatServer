#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>
#include <string>
#include <unordered_map>

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;
std::mutex clients_mutex;
std::unordered_map<SOCKET, std::string> clientNames;

void Broadcast(const std::string& message, SOCKET sender) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), static_cast<int>(message.size()), 0);
        }
    }
}

void HandleClient(SOCKET clientSock) {
    char buffer[1024];
    int bytesReceived;

    // 첫 번째 메시지는 닉네임 
    bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        closesocket(clientSock);
        return;
    }

    buffer[bytesReceived] = '\0';
    std::string nickname = buffer;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(clientSock);
        clientNames[clientSock] = nickname;
    }

    std::string joinMsg = nickname + " has joined the chat.\n";
    Broadcast(joinMsg, clientSock);
    std::cout << joinMsg;

    while ((bytesReceived = recv(clientSock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::string msg = "[" + nickname + "]: " + buffer; 
        Broadcast(msg, clientSock);
        std::cout << msg << std::endl;
    }

    // 연결 종료 시 
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSock), clients.end());
        clientNames.erase(clientSock);
    }

    std::string leaveMsg = nickname + " has left the chat.\n";
    Broadcast(leaveMsg, clientSock);
    std::cout << leaveMsg;

    closesocket(clientSock);
}

void multithreadServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSock, SOMAXCONN);
    std::cout << "Server listening on port 9000..." << std::endl;

    while (true) {
        SOCKET clientSock = accept(listenSock, nullptr, nullptr);
        std::thread(HandleClient, clientSock).detach();
    }

    closesocket(listenSock);
    WSACleanup();
}