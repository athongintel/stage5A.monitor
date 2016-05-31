#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include <netlink/errno.h>
#include <errno.h>

#include "nlutil.h"


#define ETH_ALEN 6

//ERROR CODE DEFINE
enum common_errors {
	INTERFACE_ALREADY_DISCONNECTED,
	CANNOT_ALLOCATE_NETLINK_MESSAGE,
	CANNOT_ALLOCATE_NETLINK_CALLBACK,
	CANNOT_ADD_MULTICAST_MEMBERSHIP,
	CANNOT_SEND_NETLINK_MESSAGE,
	CANNOT_RECEIVE_NETLINK_MESSAGE
};


enum link_state {
	ASSOCIATED,
	AUTHENTICATED,
	IBSS_JOINED,
	DISCONNECTED
};

struct access_point {
	char SSID[256];
	unsigned char mac_address[ETH_ALEN];
	int frequency;
	enum nl80211_auth_type authType = NL80211_AUTHTYPE_OPEN_SYSTEM;
	int signal;
};

struct wiphy {
	char name[20];
	int phyIndex;	
	unsigned char mac_addr[ETH_ALEN];
};

struct interface {
	struct wiphy wiphy;
	char name[20];	
	int ifIndex;	
};

struct wiphy_state {
	enum link_state state;
	struct access_point accessPoint;
};

//APIs
int add_network_interface(const struct wiphy* wiphy, const char* name, struct interface* interface);
int remove_network_interface(const struct interface* interface);
int dump_interface_list(struct nl_sock* nlSocket, int netlinkID, nl_recvmsg_msg_cb_t handler, void* args);
int get_interface_state(struct nl_sock* nlSocket, int netlinkID, struct interface* interface, struct wiphy_state* state);
int full_network_scan_trigger(struct nl_sock* nlSocket,  int netlinkID, struct interface* interface);
int get_network_scan_result(struct nl_sock* nlSocket, int netlinkID, struct interface* interface, nl_recvmsg_msg_cb_t handler, void* args);
int connect_to_access_point(struct nl_sock* nlSocket, int netlinkID, struct interface* interface, struct access_point* ap, void* args);
int disconnect_from_access_point(struct nl_sock* nlSocket, int netlinkID, struct interface* interface);
