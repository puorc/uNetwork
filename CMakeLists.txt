cmake_minimum_required(VERSION 3.17)
project(tcpip)
find_package (Threads)
set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/include)
add_executable(tcpip src/main.cpp src/tuntap_if.cpp include/tuntap_if.h src/netdev.cpp include/netdev.h include/ethernet.h include/ipv4.h include/arp.h src/ipv4.cpp src/arp.cpp src/network.cpp include/network.h src/ipc.cpp include/ipc.h src/ethernet.cpp src/TCPConnection.cpp include/TCPConnection.h src/utils.cpp include/utils.h)
target_link_libraries (tcpip ${CMAKE_THREAD_LIBS_INIT})