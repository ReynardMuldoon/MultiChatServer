#include <memory.h>
#include "TCPClient.h"
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

std::shared_ptr<TCPClient> client = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT ||
        signal == CTRL_LOGOFF_EVENT || signal == CTRL_SHUTDOWN_EVENT)
    {
        std::cout << "\n[서버 종료 신호 수신됨. 자원 해제 중...]\n";
        if (client) {
            client->Shutdown();  // 명시적 자원 해제
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

    client = std::make_shared<TCPClient>();
	if (client->Init(9000))
        client->Run();
	
	return 0;
}