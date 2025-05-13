#include "pollServer.h"

TCPPollServer::TCPPollServer()
{
}

TCPPollServer::~TCPPollServer()
{
    Shutdown();
}

bool TCPPollServer::Init(int port)
{
    TCPServer::Init(port);

    u_long mode = 1; // 논블로킹 모드
    ioctlsocket(serverSocket, FIONBIO, &mode);

    WSAPOLLFD serverFd = { serverSocket, POLLRDNORM, 0 };
    pollfds.push_back(serverFd);

    std::cout << "POLL 서버 초기화 완료, 포트 " << port << "에서 대기 중..." << std::endl;
    return true;
}

void TCPPollServer::Run()
{
    char buffer[1024];

    while (true) {
        int result = WSAPoll(pollfds.data(), pollfds.size(), -1);
        if (result == SOCKET_ERROR) {
            std::cerr << "WSAPoll 오류 발생\n";
            break;
        }

        for (size_t i = 0; i < pollfds.size(); ++i) {
            WSAPOLLFD& pfd = pollfds[i];

            if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL))
            {
                TCPServer::RemoveClient(pfd.fd);
                hasNickname.erase(pfd.fd);
                pollfds.erase(pollfds.begin() + i);
                --i; // 지웠으니 인덱스 보정
            }

            else if (pfd.revents & POLLRDNORM) {
                if (pfd.fd == serverSocket) {
                    HandleNewConnection();
                }
                else {
                    HandleClientMessage(pfd.fd, i);
                }
            }
        }
    }
}

void TCPPollServer::Shutdown()
{
    for (auto& fd : pollfds) closesocket(fd.fd);
    WSACleanup();
}

void TCPPollServer::HandleNewConnection()
{
    SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
    
    u_long mode = 1;
    ioctlsocket(clientSocket, FIONBIO, &mode);
    pollfds.push_back({ clientSocket, POLLRDNORM, 0 });

    hasNickname[clientSocket] = false;
    
}

void TCPPollServer::HandleClientMessage(SOCKET clientSocket, size_t& index)
{
    char buffer[1024];

    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived <= 0) {
        TCPServer::RemoveClient(clientSocket);
        hasNickname.erase(clientSocket);
        pollfds.erase(pollfds.begin() + index);
        --index; // 지웠으니 인덱스 보정
    }
    else {
        // 아직 닉네임 설정을 안했으면 (해당 클라이언트로부터 첫 메시지)
        if (!hasNickname[clientSocket])
        {
            buffer[bytesReceived] = '\0';
            std::string nickname = buffer;
            clientNames[clientSocket] = nickname;
            hasNickname[clientSocket] = true;

            // 해당 클라이언트 접속 알리기 
            std::string joinMsg = clientNames[clientSocket] + " has joined the chat.\n";
            Broadcast(joinMsg, clientSocket);
            std::cout << joinMsg;
        }

        // 닉네임 설정 완료 이후 
        else
        {
            buffer[bytesReceived] = '\0';
            std::string msg = "[" + clientNames[clientSocket] + "]: " + buffer;
            std::cout << msg << std::endl;

            // 모든 클라이언트에게 브로드캐스트
            Broadcast(msg, clientSocket);
        }
    }
}

void TCPPollServer::Broadcast(const std::string& message, SOCKET sender)
{
    for (auto& client : pollfds) 
    {
        if (client.fd != serverSocket && client.fd != sender) 
        {
            send(client.fd, message.c_str(), message.length(), 0);
        }
    }
}
