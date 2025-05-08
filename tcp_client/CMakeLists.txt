add_executable(tcp_client main.cpp "TCPClient.cpp" "ConsoleChatUI.cpp")
target_link_libraries(tcp_client ws2_32)
