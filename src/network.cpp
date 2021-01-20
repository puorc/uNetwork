#include "network.h"

void read_loop() {
    tun_init();
    uint8_t buf[8092];

    TCPConnection s(0x2ed93ad8, 80);
    s.init();

    bool doit = false;
    ssize_t n;
    ethernet_protocol protocol;
    while ((n = ethernet_recv(buf, &protocol)) > 0) {
        switch (protocol) {
            case ethernet_protocol::IPv4: {
                auto [data, protocol, size] = ipv4_recv(buf, n);
                s.receive(data, size);
                const char *tosend = "GET / HTTP/1.1\r\nHost: google.com:80\r\nConnection: close\r\n\r\n";
                if (!doit) {
                    s.send((uint8_t *) tosend, 58);
                    doit = true;
                }
                break;
            }
            case ethernet_protocol::ARP:
                std::cout << "arp packet" << std::endl;
                arp_recv(buf);
                break;
            case ethernet_protocol::IPv6:
                std::cout << "Currently IP v6 not supported." << std::endl;
                break;
        }
        std::cout.flush();
    }
}