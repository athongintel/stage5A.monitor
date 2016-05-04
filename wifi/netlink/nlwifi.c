#include "nlwifi.h"


int full_network_scan_trigger_callback(struct nl_msg *msg, void *arg) {
    // Called by the kernel when the scan is done or has been aborted.
    struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
    struct trigger_results *results = arg;

    //printf("Got something.\n");
    //printf("%d\n", arg);
    //nl_msg_dump(msg, stdout);

    if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED) {
        printf("Got NL80211_CMD_SCAN_ABORTED.\n");
        results->done = 1;
        results->aborted = 1;
    } else if (gnlh->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
        printf("Got NL80211_CMD_NEW_SCAN_RESULTS.\n");
        results->done = 1;
        results->aborted = 0;
    }  // else probably an uninteresting multicast message.

    return NL_SKIP;
}


/*API IMPLEMENTATION*/

int dump_wiphy_list(struct nl_sock* nlSocket, int nlID,  nl_recvmsg_msg_cb_t handler, void* args){
	//using this socket to send dump request to kernel and receive response
	struct nl_msg* nlMessage;
	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		//cannot allocate message
		fprintf(stderr, "Error: cannot allocate netlink message");
		return -1;
	}
	
	//set command and add attribute for a wiphy dump
	genlmsg_put(nlMessage, 0, 0, nlID, 0, NLM_F_DUMP, NL80211_CMD_GET_INTERFACE, 0);
	nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, handler, args);
	
	//send the message
	nl_send_auto(nlSocket, nlMessage);
	//block and wait for response packet. Returned after callback.
    	int ret = nl_recvmsgs_default(nlSocket);

	//free the message
   	nlmsg_free(nlMessage);
	
	return ret;
}

int get_network_scan_result(struct nl_sock* nlSocket, int netlinkID, int ifIndex, nl_recvmsg_msg_cb_t callback, void* args){
	
	struct nl_msg* nlMessage;

	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		fprintf(stderr, "Error: cannot allocate netlink message");
		return -1;	
	}

	genlmsg_put(nlMessage, 0, 0, netlinkID, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);  // Setup which command to run.
	nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, ifIndex);  // Add message attribute, which interface to use.
	nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, callback, args);  // Add the callback.
	
	nl_send_auto(nlSocket, nlMessage);  // Send the message.
	int ret = nl_recvmsgs_default(nlSocket);  // Retrieve the kernel's answer
	
	nlmsg_free(nlMessage);
	return ret;
}

int full_network_scan_trigger(struct nl_sock* socket, int if_index, int driver_id) {
	// Starts the scan and waits for it to finish. Does not return until the scan is done or has been aborted.
	struct trigger_results results = { .done = 0, .aborted = 0 };
	struct nl_msg *msg;
	struct nl_cb *cb;
	struct nl_msg *ssids_to_scan;
	int err;
	int ret;
	int mcid;

	//multicast id
	mcid = nl_get_multicast_id(socket, "nl80211", "scan");
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
	
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, full_network_scan_trigger_callback, &results);  // Add the callback.
	nl_cb_err(cb, NL_CB_CUSTOM, default_error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, default_finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, default_ack_handler, &err);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, default_noseqcheck_handler, NULL);  // No sequence checking for multicast messages.

	// Send NL80211_CMD_TRIGGER_SCAN to start the scan. The kernel may reply with NL80211_CMD_NEW_SCAN_RESULTS on
	// success or NL80211_CMD_SCAN_ABORTED if another scan was started by another process.
	err = 1;
	ret = nl_send_auto(socket, msg);  // Send the message.
	
	//First wait for ack_handler(). This helps with basic errors.
	while (err > 0)
		ret = nl_recvmsgs(socket, cb);  
	
	if (err < 0) {
		printf("WARNING: err has a value of %d.\n", err);
	}
	if (ret < 0) {
		printf("ERROR: nl_recvmsgs() returned %d (%s).\n", ret, nl_geterror(-ret));
		return ret;
	}
	
	while (!results.done)
		nl_recvmsgs(socket, cb);  // Now wait until the scan is done or aborted.
	
	if (results.aborted) {
		printf("ERROR: Kernel aborted scan.\n");
		return 1;
	}

	// Cleanup.
	nlmsg_free(msg);
	nl_cb_put(cb);
	nl_socket_drop_membership(socket, mcid);  // No longer need this.
	return 0;
}






























