#include "TCPClient.h"

#pragma comment(lib, "ws2_32.lib")

int main(void)
{
	TCPClient tcpClient;
	if (tcpClient.Init(9000))
		tcpClient.Run();

	tcpClient.Shutdown();
	return 0;
}