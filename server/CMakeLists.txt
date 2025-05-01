add_executable(server "multithreadServer.h" "main.cpp" "selectServer.h" "iocpServer.h" "pollServer.h" "selectServer.cpp" "Server.h")
target_link_libraries(server ws2_32)