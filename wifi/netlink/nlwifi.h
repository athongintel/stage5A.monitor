#include <netlink/netlink.h>
#include <netlink/socket.h>
#include <netlink/msg.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <net/if.h>
#include <netlink/errno.h>
#include <errno.h>

#include "nlutil.h"


//APIs

int dump_wiphy_list(struct nl_sock* socket, int netlinkID, nl_recvmsg_msg_cb_t handler, void* args);
int full_network_scan_trigger(struct nl_sock* socket, int if_index, int driver_id);
int get_network_scan_result();
