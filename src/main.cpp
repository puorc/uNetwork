#include <thread>
#include "NetworkDevice.h"
#include "ARPController.h"
#include "Ethernet.h"
#include "IPController.h"
#include "TCPController.h"
#include "RouteTable.h"
#include "MessageListener.h"


int main() {
    char const *gateway_ip = "10.0.0.5";

    NetworkDevice device;
    Ethernet eth(device);
    RouteTable rtable(gateway_ip);

    ARPController arp(eth, device);
    IPController ip(eth, arp, device, rtable);
    TCPController tcp(ip, device);
    MessageListener listener(tcp);

    std::thread t1([&]() { while (1) { tcp.recv(); }});
    std::thread t2([&]() { listener.start(); });
    t1.join();
    t2.join();
    return 0;
}
