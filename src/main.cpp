#include "NetworkDevice.h"
#include "ARPController.h"
#include "Ethernet.h"
#include "IPController.h"
#include "TCPController.h"
#include "RouteTable.h"


int main() {
    NetworkDevice device;
    Ethernet eth(device);
    RouteTable rtable(device);
    ARPController arp(eth, device);
    IPController ip(eth, arp, device, rtable);
    TCPController tcp(ip);

    bool doit = false;
    while (true) {
        tcp.recv();
        const char *tosend = "GET / HTTP/1.1\r\nHost: google.com:80\r\nConnection: close\r\n\r\n";
        if (!doit) {
            tcp.send(0, (uint8_t *) tosend, 58);
            doit = true;
        }
    }


//    std::thread t1(read_loop);
//    std::thread t2(rx_loop);
//    t1.join();
//    t2.join();
    return 0;
}
