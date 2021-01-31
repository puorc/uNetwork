#include "RouteTable.h"

RouteTable::RouteTable(const char *gateway_ip) {
    table.push_back(Entry{.dst = 0, .gateway=ip_parse(gateway_ip), .genmask=0, .flags=Entry::Type::Gateway});
}

uint32_t RouteTable::lookup(uint32_t target) const {
    for (auto &e : table) {
        if ((target & e.genmask) == e.dst && e.flags == Entry::Type::Gateway) {
            return e.gateway;
        }
    }
    return end;
}