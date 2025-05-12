#include "TCPClient.h"
#include <iostream>
#include <conio.h>

TCPClient::~TCPClient()
{
    Shutdown();
}

bool TCPClient::Init(int port)
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup 실패" << std::endl;
        return false;
    }

    this->port = port;

    addrinfo hints = {}, * result = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int ret = getaddrinfo("127.0.0.1", std::to_string(port).c_str(), &hints, &result);
    if (ret != 0) {
        std::cerr << "getaddrinfo 실패: " << ret << std::endl;
        WSACleanup();
        return 1;
    }

    clientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "소켓 생성 실패" << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    if (connect(clientSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "서버에 연결 실패" << std::endl;
        closesocket(clientSocket);
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
    ui.Init();
}

void TCPClient::Run()
{
    SetNickname();


    std::thread recvThread([this]() { this->ReceiveMessages(); });

    while (running) {
        if (_kbhit()) {
            char ch = _getch();
            if (ch == '\r') { // 엔터
                std::string message = ui.ConsumeInput();
                if (!message.empty()) {
                    send(clientSocket, message.c_str(), message.size(), 0);
                    ui.PrintMessage("[" + nickname + "]: " + message);
                }
            }
            else {
                ui.ProcessInputChar(ch);
            }
        }
    }

    recvThread.join();
}

void TCPClient::Shutdown()
{
    closesocket(clientSocket);
    WSACleanup();
}

void TCPClient::ReceiveMessages()
{
    char buffer[DEFAULT_BUFFER_LEN];
    while (running) {
        int bytesReceived = recv(clientSocket, buffer, DEFAULT_BUFFER_LEN - 1, 0);
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

void TCPClient::SetNickname()
{
    std::cout << "닉네임을 입력하세요: ";
    std::getline(std::cin, nickname);
    send(clientSocket, nickname.c_str(), nickname.length(), 0);
    ui.PrintMessage("You (" + nickname + ") have joined the chat.\n");   // 접속 알림 
}
