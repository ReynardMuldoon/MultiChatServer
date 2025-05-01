#include <winsock2.h>
#include <iostream>
#include <thread>
#include <string>

#pragma comment(lib, "ws2_32.lib")

SOCKET clientSocket;
bool running = true;

void ReceiveMessages() {
    char buffer[1024];
    while (running) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << "\r" << buffer << "\n> ";
            std::cout.flush();
        }
        else {
            std::cout << "\n[서버 연결 종료됨]\n";
            running = false;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(9000);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "서버에 연결할 수 없습니다.\n";
        return 1;
    }

    std::cout << "[서버 연결 성공]\n";

    std::thread receiver(ReceiveMessages);

    while (running) {
        std::cout << "> ";
        std::string input;
        std::getline(std::cin, input);

        if (!input.empty()) {
            send(clientSocket, input.c_str(), input.size(), 0);
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    receiver.join();

    return 0;
}
