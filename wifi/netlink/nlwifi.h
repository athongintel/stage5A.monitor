#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include "nl80211.h"

void mac_addr_n2a(char *mac_addr, unsigned char *arg);
void print_ssid(unsigned char *ie, int ielen);

void print_ssid_escaped(const uint8_t len, const uint8_t *data);
int dump_wiphy_list(struct nl_sock* socket, int netlinkID, void* handler);
