#ifndef TCPIP_ROUTETABLE_H
#define TCPIP_ROUTETABLE_H

// https://www.thegeekstuff.com/2012/04/route-examples/

#include <cstdint>
#include <vector>
#include "NetworkDevice.h"

class RouteTable {
private:
    class Entry {
    public:
        enum class Type : uint32_t {
            Gateway,
            Reject,
            Up,
        };

        const uint32_t dst;
        const uint32_t gateway;
        const uint32_t genmask;
        const Type flags;
    };

    std::vector<Entry> table;
public:
    explicit RouteTable(const NetworkDevice &device);

    uint32_t lookup(uint32_t target) const;

    static const uint32_t end = 0;
};


#endif //TCPIP_ROUTETABLE_H
