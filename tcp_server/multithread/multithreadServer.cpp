#include "multithreadServer.h"

TCPMultithreadServer::TCPMultithreadServer(int port) : TCPServer(port) { }

TCPMultithreadServer::~TCPMultithreadServer() { }

void TCPMultithreadServer::Run()
{
    InitWinsock();
    SetupServerSocket();

    std::cout << "TCP Multithread Server listening on port " << port << "...\n";

    while (true) {
        SOCKET clientSock = accept(serverSocket, nullptr, nullptr);
        std::thread(TCPMultithreadServer::HandleClient, clientSock).detach();
    }
}

void TCPMultithreadServer::HandleClient(SOCKET clientSocket)
{
    std::string nickname = GetNickname(clientSocket);

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(clientSocket);
        clientNames[clientSocket] = nickname;
    }

    SendJoinMsg(clientSocket);

    HandleClientMessage(clientSocket);

    RemoveClient(clientSocket);
}

void TCPMultithreadServer::RemoveClient(SOCKET clientSocket)
{
    TCPServer::RemoveClient(clientSocket);

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), clientSocket), clients.end());
        clientNames.erase(clientSocket);
    }
}

void TCPMultithreadServer::HandleNewConnection()
{
    std::string nickname = GetNickname(clientSocket);

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(clientSocket);
        clientNames[clientSocket] = nickname;
    }

    SendJoinMsg(clientSocket);
}

void TCPMultithreadServer::HandleClientMessage(SOCKET clientSocket)
{
    int bytesReceived;
    char buffer[1024];

    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        std::string msg = "[" + clientNames[clientSocket] + "]: " + buffer;
        Broadcast(msg, clientSocket);
        std::cout << msg << std::endl;
    }
}

void TCPMultithreadServer::Broadcast(const std::string& message, SOCKET sender)
{
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client : clients) {
        if (client != sender) {
            send(client, message.c_str(), static_cast<int>(message.size()), 0);
        }
    }
}
