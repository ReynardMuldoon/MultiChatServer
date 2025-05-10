#include "multithreadServer.h"

TCPMultithreadServer::TCPMultithreadServer()  
{ 

}

TCPMultithreadServer::~TCPMultithreadServer() 
{ 
    Shutdown();
}

bool TCPMultithreadServer::Init(int port)
{
    TCPServer::Init(port);

    std::cout << "MULTITHREAD 서버 초기화 완료, 포트 " << port << "에서 대기 중..." << std::endl;
    return true;
}

void TCPMultithreadServer::Run()
{
    while (true) 
    {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        std::thread([this, clientSocket]() { HandleClient(clientSocket); }).detach();
    }
}

void TCPMultithreadServer::Shutdown()
{
    {
        std::lock_guard<std::mutex> lock(clients_mutex);

        for (int i = 0; i < clients.size(); i++)
        {
            closesocket(clients[i]);
        }
    }

    WSACleanup();
    std::cout << "서버 종료 완료" << std::endl;
}

void TCPMultithreadServer::HandleClient(SOCKET clientSocket)
{
    std::string nickname = GetNickname(clientSocket);

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(clientSocket);
        clientNames[clientSocket] = nickname;
    }

    std::string joinMsg = nickname + " has joined the chat.\n";
    Broadcast(joinMsg, clientSocket);
    std::cout << joinMsg;

    char buffer[1024];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) 
    {
        buffer[bytesReceived] = '\0';
        std::string msg = "[" + nickname + "]: " + buffer; 
        Broadcast(msg, clientSocket);
        std::cout << msg << std::endl;
    }

    RemoveClient(clientSocket);
}

void TCPMultithreadServer::RemoveClient(SOCKET clientSocket)
{
    std::string nickname;
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        nickname = clientNames[clientSocket];
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
        clientNames.erase(clientSocket);
    }

    std::string leaveMsg = nickname + " has left the chat.\n";
    Broadcast(leaveMsg, clientSocket);
    std::cout << leaveMsg;

    closesocket(clientSocket);
}

void TCPMultithreadServer::Broadcast(const std::string& message, SOCKET sender)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) 
    {
        if (client != sender) 
        {
            send(client, message.c_str(), static_cast<int>(message.size()), 0);
        }
    }
}
