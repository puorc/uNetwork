#include <thread>
#include "NetworkDevice.h"
#include "ARPController.h"
#include "Ethernet.h"
#include "IPController.h"
#include "TCPController.h"
#include "RouteTable.h"
#include "MessageListener.h"


int main() {
    NetworkDevice device;
    Ethernet eth(device);
    RouteTable rtable(device);
    ARPController arp(eth, device);
    IPController ip(eth, arp, device, rtable);
    TCPController tcp(ip, device);
    MessageListener listener(tcp);

    int fd = tcp.alloc();
    tcp.init(fd, 0x2ed93ad8, 80);
    bool doit = false;
    while (true) {
        tcp.recv();
    }
//    std::thread t1([&]() { while (1) { tcp.recv(); }});
//    std::thread t2([&]() { listener.start(); });
//    t1.join();
//    t2.join();
    return 0;
}
