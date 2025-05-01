add_executable(selectClient main.cpp "../client/ConsoleChatUI.cpp")
target_link_libraries(selectClient ws2_32)
