cmake_minimum_required(VERSION 3.16)
project(server)

set(CMAKE_CXX_STANDARD 14)

add_executable(server src/main.cpp src/include/net.h src/net.cpp src/include/logger.h src/logger.cpp src/include/task.h src/include/cTask.h src/cTask.cpp src/task.cpp src/include/threadPool.h src/threadPool.cpp src/include/tank.h src/tank.cpp)
target_link_libraries(server pthread mysqlclient)

include_directories(src/include)