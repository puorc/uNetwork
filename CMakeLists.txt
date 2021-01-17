cmake_minimum_required(VERSION 3.17)
project(tcpip)
find_package (Threads)
set(CMAKE_CXX_STANDARD 20)

add_executable(tcpip main.cpp tuntap_if.cpp tuntap_if.h netdev.cpp netdev.h ethernet.h ipv4.h arp.h constants.h ipv4.cpp arp.cpp network.cpp network.h ipc.cpp ipc.h ethernet.cpp TCPConnection.cpp TCPConnection.h utils.cpp utils.h)
target_link_libraries (tcpip ${CMAKE_THREAD_LIBS_INIT})