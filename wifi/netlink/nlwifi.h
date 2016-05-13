#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include <netlink/errno.h>
#include <errno.h>

#include "nlutil.h"

//ERROR CODE DEFINE
enum common_errors {
	INTERFACE_ALREADY_DISCONNECTED,
	CANNOT_ALLOCATE_NETLINK_MESSAGE,
	CANNOT_ALLOCATE_NETLINK_CALLBACK,
	CANNOT_ADD_MULTICAST_MEMBERSHIP,
	CANNOT_SEND_NETLINK_MESSAGE,
	CANNOT_RECEIVE_NETLINK_MESSAGE
};


typedef struct access_point {
	char SSID[255];
	unsigned char mac_address[6];
	int frequency;
};

//APIs

int get_wiphy_state(struct nl_sock* nlSocket, int netlinkID, int ifIndex, struct access_point* accessPoint);
int dump_wiphy_list(struct nl_sock* nlSocket, int netlinkID, nl_recvmsg_msg_cb_t handler, void* args);
int full_network_scan_trigger(struct nl_sock* nlSocket,  int netlinkID, int ifIndex);
int get_scan_result(struct nl_sock* nlSocket, int netlinkID, int ifIndex, nl_recvmsg_msg_cb_t handler, void* args);
int connect_to_access_point(struct nl_sock* nlSocket, int netlinkID, int ifIndex, struct access_point* ap, void* args);
int disconnect_from_access_point(struct nl_sock* nlSocket, int netlinkID, int ifIndex);
