cmake_minimum_required(VERSION 3.17)
project(tcpip)
find_package (Threads)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(tcpip src/main.cpp src/ipc.cpp include/ipc.h src/TCPConnection.cpp include/TCPConnection.h src/utils.cpp include/utils.h src/ARPController.cpp include/ARPController.h include/NetworkDevice.h src/NetworkDevice.cpp src/Ethernet.cpp include/Ethernet.h src/IPController.cpp include/IPController.h src/RouteTable.cpp include/RouteTable.h src/TCPController.cpp include/TCPController.h)
target_link_libraries(tcpip ${CMAKE_THREAD_LIBS_INIT})