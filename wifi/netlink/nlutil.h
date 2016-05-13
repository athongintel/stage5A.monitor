#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include <netlink/errno.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>

#include "nl80211.h"

struct nl_sock* create_genlink_socket(int& nlID);
void mac_addr_n2a(char *mac_addr, unsigned char *arg);
void mac_addr_a2n(unsigned char* raw, char* mac_addr);
char* get_ssid_string(unsigned char *ie, int ielen);
char* channel_width_name(enum nl80211_chan_width width);
char* channel_type_name(enum nl80211_channel_type channel_type);
char* iftype_name(enum nl80211_iftype iftype);
int nl_get_multicast_id(struct nl_sock *sock, const char *family, const char *group);
int ieee80211_frequency_to_channel(int freq);

//default handlers
int default_ack_handler(struct nl_msg *msg, void *arg);
int default_finish_handler(struct nl_msg *msg, void *arg);
int default_error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg);
int default_noseqcheck_handler(struct nl_msg *msg, void *arg);

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

struct trigger_results {
    
    int done; //indicate that the current operation is completed
    int extra; //extra information on the current operation
};


struct handler_args {  // For family_handler() and nl_get_multicast_id().
    const char *group;
    int id;
};
