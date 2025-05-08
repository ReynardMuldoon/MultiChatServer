// IOCP_Server.cpp
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

#define PORT 9000
#define MAX_BUFFER 1024
#define WORKER_COUNT 4

struct PER_IO_CONTEXT {
    OVERLAPPED overlapped = {};
    WSABUF wsaBuf;
    char buffer[MAX_BUFFER];
    SOCKET socket;
    DWORD bytesReceived;
    int operation; // 0: RECV, 1: SEND
};

HANDLE iocpHandle;

void WorkerThread() {
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    PER_IO_CONTEXT* context;

    while (true) {
        BOOL success = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred,
            &completionKey, (LPOVERLAPPED*)&context, INFINITE);

        if (!success || bytesTransferred == 0) {
            std::cout << "클라이언트 연결 종료됨\n";
            closesocket(context->socket);
            delete context;
            continue;
        }

        if (context->operation == 0) { // RECV 완료
            context->buffer[bytesTransferred] = '\0';
            std::cout << "수신: " << context->buffer << std::endl;

            // Echo back
            context->wsaBuf.len = bytesTransferred;
            context->operation = 1; // SEND
            DWORD sent;
            WSASend(context->socket, &context->wsaBuf, 1, &sent, 0, &context->overlapped, nullptr);
        }
        else { // SEND 완료
            // 다시 수신 준비
            context->operation = 0;
            DWORD flags = 0;
            context->wsaBuf.len = MAX_BUFFER;
            WSARecv(context->socket, &context->wsaBuf, 1, nullptr, &flags, &context->overlapped, nullptr);
        }
    }
}

int iocpServer() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(listenSocket, SOMAXCONN);

    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

    // 워커 쓰레드 생성
    for (int i = 0; i < WORKER_COUNT; ++i) {
        std::thread(WorkerThread).detach();
    }

    std::cout << "서버 실행 중...\n";

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        std::cout << "클라이언트 접속\n";

        // 클라이언트 소켓을 IOCP에 등록
        CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, 0, 0);

        // 비동기 수신 시작
        PER_IO_CONTEXT* context = new PER_IO_CONTEXT;
        context->socket = clientSocket;
        context->wsaBuf.buf = context->buffer;
        context->wsaBuf.len = MAX_BUFFER;
        context->operation = 0;

        DWORD flags = 0;
        WSARecv(clientSocket, &context->wsaBuf, 1, nullptr, &flags, &context->overlapped, nullptr);
    }

    closesocket(listenSocket);
    WSACleanup();
    return 0;
}
