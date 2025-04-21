#include <winsock2.h>
#include <windows.h>
#include <thread>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

void ReceiveMessages(SOCKET sock) {
    char buffer[512];
    int len;
    while ((len = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        std::string msg(buffer, len);
        std::cout << "[Server]: " << msg << std::endl;
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

    std::thread(ReceiveMessages, sock).detach();

    std::string input;
    while (std::getline(std::cin, input)) {
        send(sock, input.c_str(), input.size(), 0);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
