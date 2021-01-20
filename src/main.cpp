#include <thread>
#include "network.h"
#include "ipc.h"

int main() {
    std::thread t1(read_loop);
    std::thread t2(rx_loop);
    t1.join();
    t2.join();
    return 0;
}
