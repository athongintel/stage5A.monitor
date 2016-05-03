#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include "nl80211.h"


//utils functions
nl_sock* create_genlink_socket(int& nlID);
void mac_addr_n2a(char *mac_addr, unsigned char *arg);
void print_ssid(unsigned char *ie, int ielen);
int ieee80211_frequency_to_channel(int freq);
char *channel_width_name(enum nl80211_chan_width width);
char *channel_type_name(enum nl80211_channel_type channel_type);
const char *iftype_name(enum nl80211_iftype iftype);
void print_ssid_escaped(const uint8_t len, const uint8_t *data);

static const char *ifmodes[NL80211_IFTYPE_MAX + 1] = {
	"unspecified",
	"IBSS",
	"managed",
	"AP",
	"AP/VLAN",
	"WDS",
	"monitor",
	"mesh point",
	"P2P-client",
	"P2P-GO",
	"P2P-device",
	"outside context of a BSS",
};
static char modebuf[100];

//APIs

int dump_wiphy_list(struct nl_sock* socket, int netlinkID, nl_recvmsg_msg_cb_t handler, void* args);
int full_network_scan_trigger(struct nl_sock* socket, int if_index, int driver_id);
