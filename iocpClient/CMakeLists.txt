add_executable(iocpClient main.cpp "../client/ConsoleChatUI.cpp")
target_link_libraries(iocpClient ws2_32)
