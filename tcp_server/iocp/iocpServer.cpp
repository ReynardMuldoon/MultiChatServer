#include "iocpServer.h"

TCPIOCPServer::TCPIOCPServer() 
{
    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
}

TCPIOCPServer::~TCPIOCPServer() 
{
    Shutdown();
}


bool TCPIOCPServer::Init(int port) 
{
    if (!TCPServer::Init(port)) return false;

    for (int i = 0; i < WORKER_COUNT; ++i) 
    {
        std::thread(&TCPIOCPServer::WorkerLoop, this).detach();
    }

    std::cout << "IOCP 서버 초기화 완료, 포트 " << port << "에서 대기 중..." << std::endl;
    return true;
}

void TCPIOCPServer::Run()
{
    while (true) 
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) continue;

        auto* context = new PerIOContext;
        context->socket = clientSocket;
        context->operation = 0;

        CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)clientSocket, 0);

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients[clientSocket] = context;
        }

        DWORD flags = 0;
        WSARecv(clientSocket, &context->wsaBuf, 1, nullptr, &flags, &context->overlapped, nullptr);
    }
}

void TCPIOCPServer::Shutdown()
{
    for (auto& [sock, ctx] : clients) 
    {
        delete ctx;
    }
    if (iocpHandle) CloseHandle(iocpHandle);
    for (auto& [sock, ctx] : clients) 
    {
        delete ctx;
    }
    if (iocpHandle) CloseHandle(iocpHandle);

    TCPServer::Shutdown();
}

void TCPIOCPServer::WorkerLoop()
{
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    PerIOContext* context;

    while (true) 
    {
        BOOL success = GetQueuedCompletionStatus(iocpHandle, &bytesTransferred, &completionKey, (LPOVERLAPPED*)&context, INFINITE);

        if (!success || bytesTransferred == 0) 
        {
            SOCKET sock = context->socket;
            std::cout << "[IOCP] Client disconnected: " << sock << "\n";
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(sock);
                clientNames.erase(sock);
            }
            closesocket(sock);
            delete context;
            continue;
        }

        context->buffer[bytesTransferred] = '\0';
        std::string msg(context->buffer);
        
        std::string finalMsg = "A: " + msg;
        Broadcast(finalMsg, context->socket);
        std::cout << finalMsg << std::endl;

        context->operation = 0;
        DWORD flags = 0;
        context->wsaBuf.len = MAX_BUFFER;
        WSARecv(context->socket, &context->wsaBuf, 1, nullptr, &flags, &context->overlapped, nullptr);
    }
}

bool TCPIOCPServer::CreateServerSocket(int port)
{
    serverSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "소켓 생성 실패" << std::endl;
        return false;
    }

    this->port = port;

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

void TCPIOCPServer::RemoveClient(SOCKET clientSocket)
{
    std::string leaveMsg = "A has left the chat.\n";
    Broadcast(leaveMsg, clientSocket);
    std::cout << leaveMsg;

    closesocket(clientSocket);
}

void TCPIOCPServer::Broadcast(const std::string& message, SOCKET sender)
{
    std::lock_guard<std::mutex> lock(clients_mutex);

    for (auto& [sock, ctx] : clients) 
    {
        if (sock == sender) continue;

        // 메시지를 PerIOContext 내에 보관하여 WSASend가 끝날 때까지 안전하게 유지
        std::shared_ptr<std::string> pendingMessage = std::make_shared<std::string>(message);

        // 비동기 송신 준비
        ctx->wsaBuf.buf = const_cast<char*>(pendingMessage->c_str());
        ctx->wsaBuf.len = static_cast<ULONG>(pendingMessage->size());

        memset(&ctx->overlapped, 0, sizeof(OVERLAPPED));
        ctx->operation = 1; // SEND

        // WSASend 요청 - 비동기 방식이므로 버퍼가 유효해야 함
        int result = WSASend(sock, &ctx->wsaBuf, 1, nullptr, 0, &ctx->overlapped, nullptr);
        if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) 
        {
            std::cerr << "[Broadcast] WSASend error on socket " << sock << ": " << WSAGetLastError() << "\n";
            RemoveClient(sock);
        }
    }
}
