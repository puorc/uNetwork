#include "arp.h"

void arp_recv(uint8_t *data) {
    struct arp_t *arp = reinterpret_cast<arp_t *>(data);
    struct arp_t *resp = new arp_t();
    resp->opcode = arp->opcode;
    resp->hw_addr_len = arp->hw_addr_len;
    resp->hw_type = arp->hw_type;
    resp->protocol_addr_len = arp->protocol_addr_len;
    resp->protocol_type = arp->protocol_type;
    resp->target_protocol_addr = arp->sender_protocol_addr;
    memcpy(resp->target_hw_addr, arp->sender_hw_addr, 6);
    struct netdev *ndev = get_net_dev();
    memcpy(resp->sender_hw_addr, ndev->hwaddr, 6);
    resp->sender_protocol_addr = htonl(ndev->addr);
    ethernet_send(ndev->hwaddr, arp->sender_hw_addr, ethernet_protocol::ARP, reinterpret_cast<uint8_t *>(resp),
                  sizeof(struct arp_t));
}
