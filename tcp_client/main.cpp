#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <conio.h>
#include "TCPClient.h"
#include "ConsoleChatUI.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_PORT "9000"
#define DEFAULT_BUFFER_LEN 1024

ConsoleChatUI ui;
bool running = true;

void ReceiveMessages(SOCKET sock) {
    char buffer[DEFAULT_BUFFER_LEN];
    while (running) {
        int bytesReceived = recv(sock, buffer, DEFAULT_BUFFER_LEN - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            ui.PrintMessage(std::string(buffer));
        }
        else if (bytesReceived == 0) {
            ui.PrintMessage("서버와의 연결이 종료되었습니다.");
            running = false;
            break;
        }
        else {
            ui.PrintMessage("recv 실패");
            running = false;
            break;
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패" << std::endl;
        return 1;
    }

    addrinfo hints = {}, * result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int ret = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (ret != 0) {
        std::cerr << "getaddrinfo 실패: " << ret << std::endl;
        WSACleanup();
        return 1;
    }

    SOCKET sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sock == INVALID_SOCKET) {
        std::cerr << "소켓 생성 실패" << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "서버에 연결 실패" << std::endl;
        closesocket(sock);
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
    ui.Init();

    std::string nickname;
    std::cout << "닉네임을 입력하세요: ";
    std::getline(std::cin, nickname);
    send(sock, nickname.c_str(), nickname.length(), 0);

    std::thread recvThread(ReceiveMessages, sock);

    std::cout << "> ";

    while (running) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == '\r') { // 엔터
                std::string message = ui.ConsumeInput();
                if (!message.empty()) {
                    send(sock, message.c_str(), message.size(), 0);
                    ui.PrintMessage("[나]: " + message);
                }
            }
            else {
                ui.ProcessInputChar(ch);
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    recvThread.join();
    return 0;
}
