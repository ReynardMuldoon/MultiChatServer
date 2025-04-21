#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

std::vector<SOCKET> clients;
std::mutex clients_mutex;

void Broadcast(const std::string& message, SOCKET sender) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), message.size(), 0);
        }
    }
}

void HandleClient(SOCKET clientSock) {
    char buffer[512];
    int len;
    while ((len = recv(clientSock, buffer, sizeof(buffer), 0)) > 0) {
        std::string msg(buffer, len);
        std::cout << "Received: " << msg << std::endl;
        Broadcast(msg, clientSock);
    }
    closesocket(clientSock);
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), clientSock), clients.end());
}

int main() {
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
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(clientSock);
        }
        std::thread(HandleClient, clientSock).detach();
    }

    closesocket(listenSock);
    WSACleanup();
    return 0;
}