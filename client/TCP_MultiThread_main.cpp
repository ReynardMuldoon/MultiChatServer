#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

void ReceiveMessages(SOCKET sock) {
    char buffer[1024];
    int bytesReceived;
    while ((bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        // std::string msg(buffer, bytesReceived);
        std::cout << buffer << std::endl;
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to server." << std::endl;
        return 1;
    }

    std::string nickname;
    std::cout << "Enter your nickname: ";
    std::getline(std::cin, nickname);
    send(sock, nickname.c_str(), nickname.size(), 0);

    std::thread(ReceiveMessages, sock).detach();

    std::string input;
    while (std::getline(std::cin, input)) {
        send(sock, input.c_str(), input.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
