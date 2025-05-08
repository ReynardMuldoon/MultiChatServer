#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include "multithread/multithreadServer.h";
#include "select/selectServer.h";
#include "poll/pollServer.h"
#include "iocp/iocpServer.h";

const int port = 9000;

int selectVersion(void);

int main(void)
{
	int choice = selectVersion();
	std::shared_ptr<TCPServer> server;

	switch (choice)
	{
		case 1:
			server = std::shared_ptr<TCPServer>(new TCPMultithreadServer(9000));
			break;
		case 2:
			server = std::shared_ptr<TCPServer>(new TCPSelectServer());
			break;
		case 3:
			pollServer();
		case 4:
			iocpServer();
			break;
	}

	if (server->Init(port))
		server->Run();
	server->Shutdown();

	return 0;
}

int selectVersion()
{
	std::cout << "-----------------------------------" << std::endl;
	std::cout << "어떤 버전의 통신을 사용하시겠습니까?" << std::endl;
	std::cout << "1. MultiThreading Version" << std::endl;
	std::cout << "2. Select Version" << std::endl;
	std::cout << "3. Poll Version" << std::endl;
	std::cout << "4: IOCP with Overlapped I/O Version" << std::endl;
	std::cout << "-----------------------------------" << std::endl;
	std::cout << "선 택: ";

	int choice;
	std::cin >> choice;

	if (choice < 1 || choice > 4)
	{
		std::cout << "Wrong Choice! Please choose one more time!" << std::endl << std::endl;
		return selectVersion();
	}

	return choice;
}