add_executable(multithreadClient main.cpp "../client/ConsoleChatUI.cpp")
target_link_libraries(multithreadClient ws2_32)
