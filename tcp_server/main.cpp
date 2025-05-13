#pragma comment(lib, "ws2_32.lib")

#include <iostream>
#include "multithread/multithreadServer.h";
#include "select/selectServer.h";
#include "poll/pollServer.h"
#include "iocp/iocpServer.h";

const int port = 9000;

int selectVersion(void);

std::shared_ptr<TCPServer> server;

BOOL WINAPI ConsoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT ||
		signal == CTRL_LOGOFF_EVENT || signal == CTRL_SHUTDOWN_EVENT)
	{
		std::cout << "\n[서버 종료 신호 수신됨. 자원 해제 중...]\n";
		if (server) {
			server->Shutdown();  // 명시적 자원 해제
		}
		return TRUE; // 신호 처리 완료
	}
	return FALSE;
}

int main(void)
{
	// 종료 핸들러 등록
	if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
		std::cerr << "SetConsoleCtrlHandler 등록 실패" << std::endl;
		return 1;
	}

	int choice = selectVersion();
	std::shared_ptr<TCPServer> server;

	switch (choice)
	{
		case 1:
			server = std::make_shared<TCPServer>(new TCPMultithreadServer());
			break;
		case 2:
			server = std::make_shared<TCPServer>(new TCPSelectServer());
			break;
		case 3:
			server = std::make_shared<TCPServer>(new TCPPollServer());
			break;
		case 4:
			iocpServer();
			break;
	}

	if (server->Init(port))
		server->Run();

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