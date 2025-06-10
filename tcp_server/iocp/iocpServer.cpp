#include "iocpServer.h"

TCPIOCPServer::TCPIOCPServer() 
{
    iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (iocpHandle == nullptr)
    {
        std::cerr << "CreateIoCompletionPort failed\n";
    }
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
        std::thread(&TCPIOCPServer::WorkThread, this).detach();
    }

    std::cout << "IOCP 서버 초기화 완료, 포트 " << port << "에서 대기 중..." << std::endl;
    return true;
}

void TCPIOCPServer::Run()
{
    while (true) 
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Accept Failed\n";
            continue;
        }

        // 클라이언트 소켓을 IOCP에 연결 
        if (CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (ULONG_PTR)clientSocket, 0) == nullptr)
        {
            std::cerr << "CreateIoCompletionPrt for client failed\n";
            continue;
        }

        // 클라이언트 정보 추가 
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clients.push_back({ clientSocket, "", false });
        }

        PerIOContext* ioData = new PerIOContext();
        ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));
        ioData->clientSocket = clientSocket;
        ioData->opType = OP_RECV;

        DWORD flags = 0;
        if (WSARecv(clientSocket, &ioData->wsaBuf, 1, nullptr, &flags, &ioData->overlapped, nullptr) == SOCKET_ERROR)
        {
            if (WSAGetLastError() != WSA_IO_PENDING)
            {
                RemoveClient(clientSocket);
                delete ioData;
            }
        }
    }
}

void TCPIOCPServer::Shutdown()
{
    if (iocpHandle) CloseHandle(iocpHandle);
    TCPServer::Shutdown();
}

void TCPIOCPServer::WorkThread()
{
    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    LPOVERLAPPED overlapped;

    while (true) 
    {
        BOOL result = GetQueuedCompletionStatus(
            iocpHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE
        );

        if (!overlapped) {
            std::cerr << "GetQueuedCompletionStatus returned null overlapped\n";
            continue;
        }

        PerIOContext* ioData = (PerIOContext*)overlapped;
        if (!result || bytesTransferred == 0) 
        {
            std::cout << "[IOCP] Client disconnected: " << ioData->clientSocket << "\n";
            if (ioData->clientSocket != INVALID_SOCKET)
            {
                // 소켓 유효성 확인 
                int optVal;
                int optLen = sizeof(optVal);
                if (getsockopt(ioData->clientSocket, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) == SOCKET_ERROR)
                {
                    std::cerr << "Invalid socket detected in WorkThread: " << ioData->clientSocket << "\n";
                }
                else
                {
                    RemoveClient(ioData->clientSocket);
                }
            }
            // OP_SEND의 경우 동적 메모리 해제 
            if (ioData->opType == OP_SEND)
            {
                delete[] ioData->wsaBuf.buf;
            }
            delete ioData;
            continue;
        }

        if (ioData->opType == OP_RECV)
        {
            if (bytesTransferred >= MAX_BUFFER) 
            {
                std::cerr << "Error: Received data size (" << bytesTransferred << ") exceeds MAX_BUFFER\n";
                RemoveClient(ioData->clientSocket);
                delete ioData;
                continue;
            }
            ioData->buffer[bytesTransferred] = '\0';
            bool isValidString = true;
            for (DWORD i = 0; i < bytesTransferred; ++i) 
            {
                if (!std::isprint(static_cast<unsigned char>(ioData->buffer[i])) && ioData->buffer[i] != '\n' && ioData->buffer[i] != '\r') {
                    isValidString = false;
                    break;
                }
            }
            if (!isValidString) 
            {
                std::cerr << "Error: Received invalid string data\n";
                RemoveClient(ioData->clientSocket);
                delete ioData;
                continue;
            }

            std::string message(ioData->buffer);
            std::cout << "[DEBUG] Received message: " << message << " from socket: " << ioData->clientSocket << "\n";

            // currentClient 찾기
            std::string broadcastMsg;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                ClientInfo* currentClient = nullptr;
                for (auto& client : clients) {
                    if (client.socket == ioData->clientSocket) {
                        currentClient = &client;
                        break;
                    }
                }

                if (currentClient) {
                    if (!currentClient->isNicknameSet) {
                        currentClient->nickname = message;
                        currentClient->isNicknameSet = true;
                        broadcastMsg = "Welcome, " + message + "!\n";
                    }
                    else {
                        broadcastMsg = currentClient->nickname + ": " + message + "\n";
                    }
                }
                else {
                    std::cerr << "Client not found for socket: " << ioData->clientSocket << "\n";
                    delete ioData;
                    continue;
                }
            }

            // Broadcast 호출
            if (!broadcastMsg.empty()) 
            {
                Broadcast(broadcastMsg, ioData->clientSocket);
                std::cout << broadcastMsg;
            }

            // 다음 수신 작업 준비 
            ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));
            ioData->wsaBuf.buf = ioData->buffer;
            ioData->wsaBuf.len = MAX_BUFFER;
            ioData->opType = OP_RECV;
            DWORD flags = 0;
            if (ioData->clientSocket != INVALID_SOCKET) 
            {
                // 소켓 유효성 확인
                int optVal;
                int optLen = sizeof(optVal);
                if (getsockopt(ioData->clientSocket, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) == SOCKET_ERROR) 
                {
                    std::cerr << "Invalid socket detected for WSARecv: " << ioData->clientSocket << "\n";
                    RemoveClient(ioData->clientSocket);
                    delete ioData;
                    continue;
                }

                int result = WSARecv(ioData->clientSocket, &ioData->wsaBuf, 1, nullptr, &flags, &ioData->overlapped, nullptr);
                if (result == SOCKET_ERROR) {

                    int error = WSAGetLastError();
                    if (error != WSA_IO_PENDING) 
                    {
                        std::cerr << "WSARecv failed with error: " << error << "\n";
                        RemoveClient(ioData->clientSocket);
                        delete ioData;
                        continue;
                    }
                }
            }
            else 
            {
                std::cerr << "Invalid client socket detected\n";
                delete ioData;
                continue;
            }
        }

        else if (ioData->opType == OP_SEND)
        {
            std::cout << "[DEBUG] Sent message on socket: " << ioData->clientSocket << "\n";
            delete[] ioData->wsaBuf.buf;
            delete ioData;
        }    
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
    std::string leaveMsg;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) 
        {
            if (it->socket == clientSocket) 
            {
                leaveMsg = it->nickname.empty() ? "A client" : it->nickname;
                leaveMsg += " has left the chat.\n";
                if (clientSocket != INVALID_SOCKET)
                {
                    closesocket(clientSocket);
                    it->socket = INVALID_SOCKET;
                }
                clients.erase(it);
                break;
            }
        }
    }
    if (!leaveMsg.empty()) {
        Broadcast(leaveMsg, INVALID_SOCKET); // sender를 INVALID_SOCKET으로 설정하여 자신에게 보내지 않도록
        std::cout << leaveMsg;
    }
}

void TCPIOCPServer::Broadcast(const std::string& message, SOCKET sender) {
    // clients 복사
    std::vector<ClientInfo> clientsCopy;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clientsCopy = clients;
    }

    // 복사본을 사용하여 브로드캐스트
    for (const auto& client : clientsCopy) {
        if (client.socket == sender || client.socket == INVALID_SOCKET) continue;

        // 소켓 유효성 확인
        int optVal;
        int optLen = sizeof(optVal);
        if (getsockopt(client.socket, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) == SOCKET_ERROR) {
            std::cerr << "Invalid socket detected: " << client.socket << "\n";
            RemoveClient(client.socket);
            continue;
        }

        PerIOContext* ioData = new PerIOContext();
        ZeroMemory(&ioData->overlapped, sizeof(OVERLAPPED));
        ioData->wsaBuf.buf = new char[message.size() + 1];
        ioData->wsaBuf.len = message.size() + 1;
        strcpy_s(ioData->wsaBuf.buf, message.size() + 1, message.c_str());
        ioData->opType = OP_SEND;
        ioData->clientSocket = client.socket;

        DWORD flags = 0;
        if (WSASend(client.socket, &ioData->wsaBuf, 1, nullptr, flags, &ioData->overlapped, nullptr) == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error != WSA_IO_PENDING) {
                std::cerr << "WSASend failed with error: " << error << "\n";
                delete[] ioData->wsaBuf.buf;
                delete ioData;
                RemoveClient(client.socket);
            }
        }
    }
}