#include "nlwifi.h"


/*UTILS IMPLEMENTATION*/
void mac_addr_n2a(char *mac_addr, unsigned char *arg) {
    // From http://git.kernel.org/cgit/linux/kernel/git/jberg/iw.git/tree/util.c.
    int i, l;

    l = 0;
    for (i = 0; i < 6; i++) {
        if (i == 0) {
            sprintf(mac_addr+l, "%02x", arg[i]);
            l += 2;
        } else {
            sprintf(mac_addr+l, ":%02x", arg[i]);
            l += 3;
        }
    }
}

void print_ssid_escaped(const uint8_t len, const uint8_t *data)
{
	int i;

	for (i = 0; i < len; i++) {
		/*if (isprint(data[i]) && data[i] != ' ' && data[i] != '\\')
			printf("%c", data[i]);
		else*/if (data[i] == ' ' &&
			 (i != 0 && i != len -1))
			printf(" ");
		else
			printf("\\x%.2x", data[i]);
	}
}


void print_ssid(unsigned char *ie, int ielen) {
    uint8_t len;
    uint8_t *data;
    int i;

    while (ielen >= 2 && ielen >= ie[1]) {
        if (ie[0] == 0 && ie[1] >= 0 && ie[1] <= 32) {
            len = ie[1];
            data = ie + 2;
            for (i = 0; i < len; i++) {
                /*if (isprint(data[i]) && data[i] != ' ' && data[i] != '\\') printf("%c", data[i]);
                else*/if (data[i] == ' ' && (i != 0 && i != len -1)) printf(" ");
                else printf("\\x%.2x", data[i]);
            }
            break;
        }
        ielen -= ie[1] + 2;
        ie += ie[1] + 2;
    }
}

const char *iftype_name(enum nl80211_iftype iftype)
{
	if (iftype <= NL80211_IFTYPE_MAX && ifmodes[iftype])
		return ifmodes[iftype];
	sprintf(modebuf, "Unknown mode (%d)", iftype);
	return modebuf;
}

char *channel_type_name(enum nl80211_channel_type channel_type)
{
	switch (channel_type) {
	case NL80211_CHAN_NO_HT:
		return "NO HT";
	case NL80211_CHAN_HT20:
		return "HT20";
	case NL80211_CHAN_HT40MINUS:
		return "HT40-";
	case NL80211_CHAN_HT40PLUS:
		return "HT40+";
	default:
		return "unknown";
	}
}

int ieee80211_frequency_to_channel(int freq)
{
	/* see 802.11-2007 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq >= 4910 && freq <= 4980)
		return (freq - 4000) / 5;
	else if (freq <= 45000) /* DMG band lower limit */
		return (freq - 5000) / 5;
	else if (freq >= 58320 && freq <= 64800)
		return (freq - 56160) / 2160;
	else
		return 0;
}

char *channel_width_name(enum nl80211_chan_width width)
{
	switch (width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		return "20 MHz (no HT)";
	case NL80211_CHAN_WIDTH_20:
		return "20 MHz";
	case NL80211_CHAN_WIDTH_40:
		return "40 MHz";
	case NL80211_CHAN_WIDTH_80:
		return "80 MHz";
	case NL80211_CHAN_WIDTH_80P80:
		return "80+80 MHz";
	case NL80211_CHAN_WIDTH_160:
		return "160 MHz";
	default:
		return "unknown";
	}
}

nl_sock* create_genlink_socket(int& nlID){
	
	nl_sock* nlSocket = nl_socket_alloc();
	if (!nlSocket){
		throw "Error: cannot allocate netlink socket";
	}

	//set read and write buffer
	//nl_socket_set_buffer_size(nlSocket, 8192, 8192);

	//connect the socket to netlink generic family
	if (nl_connect(nlSocket, NETLINK_GENERIC)){
		//error, non-zero value returned
		nl_socket_free(nlSocket);
		throw "Error: cannot connect to generic netlink";
	}

	nlID = genl_ctrl_resolve(nlSocket, "nl80211");
	if (nlID < 0) {
		//nl80211 not found on kernel
		nl_socket_free(nlSocket);
		throw "Error: 80211 not found on this system";	
	}
	return nlSocket;	
}

/*API IMPLEMENTATION*/

int dump_wiphy_list(struct nl_sock* nlSocket, int nlID,  nl_recvmsg_msg_cb_t handler, void* args){
	//using this socket to send dump request to kernel and receive response
	
	
	//alloc a message
	struct nl_msg* nlMessage;
	struct nl_cb* cbGetInterface;

	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		//cannot allocate message
		fprintf(stderr, "Error: cannot allocate netlink message");
		return -1;
	}
	cbGetInterface = nl_cb_alloc(NL_CB_CUSTOM);
	if (!cbGetInterface){
		fprintf(stderr, "Error: cannot allocate netlink callback");
		return -1;	
	}
	
	//set command and add attribute for a wiphy dump
	genlmsg_put(nlMessage, 0, 0, nlID, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);
	nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, handler, args);
	
	//send the message
	int ret = nl_send_auto(nlSocket, nlMessage);
	//block and wait for response packet. Returned after callback.
    	ret = nl_recvmsgs_default(nlSocket);

	//free the message
   	nlmsg_free(nlMessage);
	
	return 0;
}

int full_network_scan_trigger(struct nl_sock* socket, int if_index, int driver_id) {
    // Starts the scan and waits for it to finish. Does not return until the scan is done or has been aborted.
    struct trigger_results results = { .done = 0, .aborted = 0 };
    struct nl_msg *msg;
    struct nl_cb *cb;
    struct nl_msg *ssids_to_scan;
    int err;
    int ret;
    int mcid = nl_get_multicast_id(socket, "nl80211", "scan");
    nl_socket_add_membership(socket, mcid);  // Without this, callback_trigger() won't be called.

    // Allocate the messages and callback handler.
    msg = nlmsg_alloc();
    if (!msg) {
        printf("ERROR: Failed to allocate netlink message for msg.\n");
        return -ENOMEM;
    }
    ssids_to_scan = nlmsg_alloc();
    if (!ssids_to_scan) {
        printf("ERROR: Failed to allocate netlink message for ssids_to_scan.\n");
        nlmsg_free(msg);
        return -ENOMEM;
    }
    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        printf("ERROR: Failed to allocate netlink callbacks.\n");
        nlmsg_free(msg);
        nlmsg_free(ssids_to_scan);
        return -ENOMEM;
    }

    // Setup the messages and callback handler.
    genlmsg_put(msg, 0, 0, driver_id, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);  // Setup which command to run.
    nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_index);  // Add message attribute, which interface to use.
    nla_put(ssids_to_scan, 1, 0, "");  // Scan all SSIDs.
    nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids_to_scan);  // Add message attribute, which SSIDs to scan for.
    nlmsg_free(ssids_to_scan);  // Copied to `msg` above, no longer need this.
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, callback_trigger, &results);  // Add the callback.
    nl_cb_err(cb, NL_CB_CUSTOM, error_handler, &err);
    nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, finish_handler, &err);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, ack_handler, &err);
    nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, no_seq_check, NULL);  // No sequence checking for multicast messages.

    // Send NL80211_CMD_TRIGGER_SCAN to start the scan. The kernel may reply with NL80211_CMD_NEW_SCAN_RESULTS on
    // success or NL80211_CMD_SCAN_ABORTED if another scan was started by another process.
    err = 1;
    ret = nl_send_auto(socket, msg);  // Send the message.
    printf("NL80211_CMD_TRIGGER_SCAN sent %d bytes to the kernel.\n", ret);
    printf("Waiting for scan to complete...\n");
    while (err > 0) ret = nl_recvmsgs(socket, cb);  // First wait for ack_handler(). This helps with basic errors.
    if (err < 0) {
        printf("WARNING: err has a value of %d.\n", err);
    }
    if (ret < 0) {
        printf("ERROR: nl_recvmsgs() returned %d (%s).\n", ret, nl_geterror(-ret));
        return ret;
    }
    while (!results.done) nl_recvmsgs(socket, cb);  // Now wait until the scan is done or aborted.
    if (results.aborted) {
        printf("ERROR: Kernel aborted scan.\n");
        return 1;
    }
    printf("Scan is done.\n");

    // Cleanup.
    nlmsg_free(msg);
    nl_cb_put(cb);
    nl_socket_drop_membership(socket, mcid);  // No longer need this.
    return 0;
}




























