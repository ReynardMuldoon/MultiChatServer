add_executable(client TCP_MultiThread_main.cpp "ConsoleChatUI.cpp")
target_link_libraries(client ws2_32)
