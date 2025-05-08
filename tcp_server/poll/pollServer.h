// WSAPollServer.cpp
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

#define PORT 9000
#define MAX_CLIENTS 1000

int pollServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    u_long mode = 1; // 논블로킹 모드
    ioctlsocket(listenSocket, FIONBIO, &mode);

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    std::vector<WSAPOLLFD> pollfds;
    WSAPOLLFD listenFd = { listenSocket, POLLRDNORM, 0 };
    pollfds.push_back(listenFd);

    std::cout << "서버가 시작되었습니다. 포트 " << PORT << "에서 대기 중입니다...\n";

    char buffer[1024];

    while (true) {
        int result = WSAPoll(pollfds.data(), pollfds.size(), -1);
        if (result == SOCKET_ERROR) {
            std::cerr << "WSAPoll 오류 발생\n";
            break;
        }

        for (size_t i = 0; i < pollfds.size(); ++i) {
            WSAPOLLFD& pfd = pollfds[i];

            if (pfd.revents & POLLRDNORM) {
                if (pfd.fd == listenSocket) {
                    // 새 클라이언트 연결 처리
                    SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
                    if (clientSocket != INVALID_SOCKET) {
                        u_long mode = 1;
                        ioctlsocket(clientSocket, FIONBIO, &mode);
                        pollfds.push_back({ clientSocket, POLLRDNORM, 0 });
                        std::cout << "클라이언트 접속\n";
                    }
                }
                else {
                    // 기존 클라이언트로부터 메시지 수신
                    int bytesReceived = recv(pfd.fd, buffer, sizeof(buffer) - 1, 0);
                    if (bytesReceived <= 0) {
                        std::cout << "클라이언트 연결 종료\n";
                        closesocket(pfd.fd);
                        pollfds.erase(pollfds.begin() + i);
                        --i; // 지웠으니 인덱스 보정
                    }
                    else {
                        buffer[bytesReceived] = '\0';
                        std::cout << "수신: " << buffer << "\n";

                        // 모든 클라이언트에게 브로드캐스트
                        for (auto& client : pollfds) {
                            if (client.fd != listenSocket && client.fd != pfd.fd) {
                                send(client.fd, buffer, bytesReceived, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    // 마무리
    for (auto& fd : pollfds) closesocket(fd.fd);
    WSACleanup();
    return 0;
}
