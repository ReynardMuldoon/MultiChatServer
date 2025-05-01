#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include "multithreadServer.h";
#include "selectServer.h";
#include "iocpServer.h";

int selectVersion(void);

int main(void)
{
	int choice = selectVersion();
	TCPSelectServer server(9000);

	switch (choice)
	{
		case 1:
			multithreadServer();
			break;
		case 2:
			server.Run();
			break;
		case 3:
			iocpServer();
			break;
	}

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