#include "nlwifi.h"

int get_wiphy_state_calback(struct nl_msg* msg, void* args){

	struct nlattr* tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* bss[NL80211_BSS_MAX + 1];
	
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {		
		[__NL80211_BSS_INVALID] = {},
		[NL80211_BSS_BSSID] = {},
		[NL80211_BSS_FREQUENCY] = { type : NLA_U32 },
		[NL80211_BSS_TSF] = { type : NLA_U64 },				
		[NL80211_BSS_BEACON_INTERVAL] = { type : NLA_U16 },
		[NL80211_BSS_CAPABILITY] = { type : NLA_U16 },
		[NL80211_BSS_INFORMATION_ELEMENTS] = {},
		[NL80211_BSS_SIGNAL_MBM] = { type : NLA_U32 },
		[NL80211_BSS_SIGNAL_UNSPEC] = { type : NLA_U8 },
		[NL80211_BSS_STATUS] = { type : NLA_U32 },
		[NL80211_BSS_SEEN_MS_AGO] = { type : NLA_U32 },
		[NL80211_BSS_BEACON_IES] = {},
	};
	
	struct wiphy_state* state = (struct wiphy_state*)args;
	
	char mac_addr[20];


	fprintf(stdout, "get state callback called");
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_BSS]) {
		//fprintf(stderr, "bss info missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy)) {
		//fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	if (!bss[NL80211_BSS_BSSID])
		return NL_SKIP;

	if (!bss[NL80211_BSS_STATUS])
		return NL_SKIP;	

	switch (nla_get_u32(bss[NL80211_BSS_STATUS])) {
	case NL80211_BSS_STATUS_ASSOCIATED:
		state->state=ASSOCIATED;
		break;
	case NL80211_BSS_STATUS_AUTHENTICATED:
		state->state=AUTHENTICATED;
		return NL_SKIP;
	case NL80211_BSS_STATUS_IBSS_JOINED:
		state->state=IBSS_JOINED;
		break;
	default:
		state->state=UNKNOWN;
		return NL_SKIP;
	}

	//get access point informations
	memcpy(state->accessPoint.mac_address, (unsigned char*)nla_data(bss[NL80211_BSS_BSSID]), nla_len(bss[NL80211_BSS_BSSID]));
	strcpy(state->accessPoint.SSID, get_ssid_string((unsigned char*)nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS])));
	if (bss[NL80211_BSS_FREQUENCY]){
		state->accessPoint.frequency = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	}

	return NL_SKIP;
	
}

int full_network_scan_trigger_multicast_callback(struct nl_msg* msg, void* arg) {
    // Called by the kernel when the scan is done or has been aborted.
    struct genlmsghdr *gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
    struct trigger_results *results = (trigger_results*)arg;

    if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED) {
        //printf("Got NL80211_CMD_SCAN_ABORTED.\n");
        results->done = 1;
        results->extra = 1;
    } else if (gnlh->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
        //printf("Got NL80211_CMD_NEW_SCAN_RESULTS.\n");
        results->done = 1;
        results->extra = 0;
    }  // else probably an uninteresting multicast message.

    return NL_SKIP;
}

int network_connect_multicast_callback(struct nl_msg* msg, void* args){
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	struct trigger_results* results = (trigger_results*)args;
	
	if (gnlh->cmd == NL80211_CMD_CONNECT){
		nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
		results->done = 1;
		if (!tb_msg[NL80211_ATTR_STATUS_CODE]){
			results->extra = 1; 
		}
		else if (nla_get_u16(tb_msg[NL80211_ATTR_STATUS_CODE]) == 0){
			results->extra = 0;		
		}
		else{
			results->extra = 1;
		}
	}
	
	return NL_SKIP;
}

int network_disconnect_multicast_callback(struct nl_msg* msg, void* args){
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	struct trigger_results* results = (trigger_results*)args;
	
	//fprintf(stdout, "disconnect multicast callback called\n");
	if (gnlh->cmd == NL80211_CMD_DISCONNECT){
		fprintf(stdout, "disconnect multicast callback got DISCONNECT RETURN\n");
		
		//parse message data and check for IFINDEX
		nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
		if (tb_msg[NL80211_ATTR_IFINDEX]){
			fprintf(stdout,  "OK, get disconnect event from interface %d, required %d\n", nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]), results->extra);
			if (nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]) == results->extra){
				results->done = 1;
				//OK, we got what we want
				return NL_STOP;
			}
		}
	}
	
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

int full_network_scan_trigger(struct nl_sock* nlSocket, int nlID, int ifIndex) {
	// Starts the scan and waits for it to finish. Does not return until the scan is done or has been aborted (extra).
	struct trigger_results results = { .done = 0, .extra = 0 };
	struct nl_msg* msg;
	struct nl_cb* cb;
	struct nl_msg* ssids_to_scan;
	int err;
	int ret;
	int mcid;

	//multicast id
	mcid = nl_get_multicast_id(nlSocket, NL80211_GENL_NAME, NL80211_MULTICAST_GROUP_SCAN);
	nl_socket_add_membership(nlSocket, mcid); 

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
	genlmsg_put(msg, 0, 0, nlID, 0, 0, NL80211_CMD_TRIGGER_SCAN, 0);  // Setup which command to run.
	nla_put_u32(msg, NL80211_ATTR_IFINDEX, ifIndex);  // Add message attribute, which interface to use.
	nla_put(ssids_to_scan, NLA_UNSPEC, 0, "");  // Scan all SSIDs, passing SSID as binary - NLA_UNSPEC, 0 as length of empty string
	nla_put_nested(msg, NL80211_ATTR_SCAN_SSIDS, ssids_to_scan);  // Add message attribute, which SSIDs to scan for.
	nlmsg_free(ssids_to_scan);  // Copied to `msg` above, no longer need this.
	
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, full_network_scan_trigger_multicast_callback, &results);  // Add the callback.
	nl_cb_err(cb, NL_CB_CUSTOM, default_error_handler, &err);
	nl_cb_set(cb, NL_CB_FINISH, NL_CB_CUSTOM, default_finish_handler, &err);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, default_ack_handler, &err);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, default_noseqcheck_handler, NULL);  // No sequence checking for multicast messages.

	// Send NL80211_CMD_TRIGGER_SCAN to start the scan. The kernel may reply with NL80211_CMD_NEW_SCAN_RESULTS on
	// success or NL80211_CMD_SCAN_ABORTED if another scan was started by another process.
	err = 1;
	ret = nl_send_auto(nlSocket, msg);  // Send the message.
	
	//First wait for ack_handler(). This helps with basic errors.
	while (err > 0)
		ret = nl_recvmsgs(nlSocket, cb);  
	
	if (err < 0) {
		printf("WARNING: err has a value of %d.\n", err);
	}
	if (ret < 0) {
		printf("ERROR: nl_recvmsgs() returned %d (%s).\n", ret, nl_geterror(-ret));
		return ret;
	}
	
	while (!results.done)
		nl_recvmsgs(nlSocket, cb);  // Now wait until the scan is done or aborted.
	
	if (results.extra) {
		printf("ERROR: Kernel aborted scan.\n");
		return 1;
	}

	// Cleanup.
	nlmsg_free(msg);
	nl_cb_put(cb);
	nl_socket_drop_membership(nlSocket, mcid);  // No longer need this.
	return 0;
}

int get_scan_result(struct nl_sock* nlSocket, int nlID, int ifIndex, nl_recvmsg_msg_cb_t handler, void* args){
	struct nl_msg* nlMessage;
	nlMessage = nlmsg_alloc();  // Allocate a message.
	if (!nlMessage){
		fprintf(stderr,"Error: cannot allocate netlink message");
		return -1;
	}
	else{
		genlmsg_put(nlMessage, 0, 0, nlID, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);  // Setup which command to run.
		nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, ifIndex);  // Add message attribute, which interface to use.
		nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, handler, args);  // Add the callback.
		int ret = nl_send_auto(nlSocket, nlMessage);  // Send the message.

		ret = nl_recvmsgs_default(nlSocket);
		nlmsg_free(nlMessage);
		if (ret < 0) {
			fprintf(stderr,"ERROR: nl_recvmsgs_default returned %d",ret);
		}
		return ret;
	}	
}

int connect_to_access_point(struct nl_sock* nlSocket, int netlinkID, int ifIndex, struct access_point* ap, void* args){
			
	struct nl_msg* nlMessage;
	struct nl_cb* nlCallback;
	int mcid;
	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		fprintf(stderr,"Error: cannot allocate netlink message\n");
		return -1;
	}
	else{
		genlmsg_put(nlMessage, 0, 0, netlinkID, 0, 0, NL80211_CMD_CONNECT, 0);
		nla_put(nlMessage, NL80211_ATTR_SSID, strlen((*ap).SSID), (*ap).SSID);
		if ((*ap).frequency>0){
			nla_put_u32(nlMessage, NL80211_ATTR_WIPHY_FREQ, (*ap).frequency);			
		}
		if (strlen((const char*)((*ap).mac_address))>0){
			nla_put(nlMessage, NL80211_ATTR_MAC, 6, (*ap).mac_address);
		}
		
		//listen to multicast event
		mcid = nl_get_multicast_id(nlSocket, "nl80211", "mlme");
		int ret;		
		if (mcid >= 0) {
			ret = nl_socket_add_membership(nlSocket, mcid);
			if (ret){
				fprintf(stderr,"Cannot listen to multicast connect event");
				return ret;
			}
			//adding callback
			struct trigger_results results;
			//alloc and set the callback
			nlCallback = nl_cb_alloc(NL_CB_DEFAULT);
			if (!nlCallback) {
				fprintf(stderr,"Error: Failed to allocate netlink callbacks.\n");
				nlmsg_free(nlMessage);
				return -ENOMEM;
			}
			nl_cb_set(nlCallback, NL_CB_VALID, NL_CB_CUSTOM, network_connect_multicast_callback, &results);		
			
			//send the message and wait for response
			nl_send_auto(nlSocket, nlMessage);
			fprintf(stdout, "Connect request sent to %s - %s\n", (*ap).SSID, (*ap).mac_address);
			
			//need to loop because nl_recvmsgs only block once message
			while (!results.done)
				nl_recvmsgs(nlSocket, nlCallback);
			
			//no longer listen to multicast event
			nl_socket_drop_membership(nlSocket, mcid);
			nlmsg_free(nlMessage);
			
			return !results.extra;
		}		
	}		
}

/* @NL80211_CMD_DISCONNECT: drop a given connection; also used to notify
 *	userspace that a connection was dropped by the AP or due to other
 *	reasons, for this the %NL80211_ATTR_DISCONNECTED_BY_AP and
 *	%NL80211_ATTR_REASON_CODE attributes are used.
 */

int disconnect_from_access_point(struct nl_sock* nlSocket, int netlinkID, int ifIndex){
	
	struct nl_msg* nlMessage;
	struct nl_cb* nlCallback;
	int mcid;
	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		return -CANNOT_ALLOCATE_NETLINK_MESSAGE;
	}
	else{
		//check if current interface is in disconnected state
		//if yes, return error
		genlmsg_put(nlMessage, 0, 0, netlinkID, 0, 0, NL80211_CMD_DISCONNECT, 0);
		//fprintf(stdout, "current if index: %d\n", ifIndex);
		nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, ifIndex);				
		//listen to multicast event
		mcid = nl_get_multicast_id(nlSocket, NL80211_GENL_NAME, NL80211_MULTICAST_GROUP_MLME);
		//fprintf(stdout, "Disconnect muticast id %d\n", mcid);
		int ret;		
		if (mcid >= 0) {
			ret = nl_socket_add_membership(nlSocket, mcid);
			if (ret){
				return -CANNOT_ADD_MULTICAST_MEMBERSHIP;
			}
			//adding callback, extra = current ifIndex
			struct trigger_results results = {.done = 0, .extra = ifIndex};
			//alloc and set the callback
			nlCallback = nl_cb_alloc(NL_CB_DEFAULT);
			if (!nlCallback) {
				nlmsg_free(nlMessage);
				return -CANNOT_ALLOCATE_NETLINK_CALLBACK;
			}
			
			//callback won't be called with NL_VB_VALID but with ML_CB_MSG_IN
			nl_cb_set(nlCallback, NL_CB_MSG_IN, NL_CB_CUSTOM, network_disconnect_multicast_callback, &results);		
			
			//send the message and wait for response
			int ret = nl_send_auto(nlSocket, nlMessage);
			if (ret<0){
				return -CANNOT_SEND_NETLINK_MESSAGE;	
			}
			//fprintf(stdout, "Disconnect request sent\n");
			
			//need to loop because nl_recvmsgs only block once message
			while (!results.done){
				//fprintf(stdout, "hanging waiting for callback\n");
				nl_recvmsgs(nlSocket, nlCallback);
			}
			
			//fprintf(stdout, "OUT OF WHILE LOOP\n");
			
			//no longer listen to multicast event
			nl_socket_drop_membership(nlSocket, mcid);
			nlmsg_free(nlMessage);
			
			return 0;
		}		
	}
}

int get_wiphy_state(struct nl_sock* nlSocket, int netlinkID, int ifIndex, struct wiphy_state* state){
	
	struct nl_msg* nlMessage;
	struct nl_cb* nlCallback;
	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		fprintf(stderr, "Cannot allocate netlink message");
		return -CANNOT_ALLOCATE_NETLINK_MESSAGE;
	}
	else{
		genlmsg_put(nlMessage, 0, 0, netlinkID, 0, 0, NL80211_CMD_GET_STATION, 0);		
		nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, ifIndex);
		//nla_put(nlMessage, NL80211_ATTR_MAC, 6, (*ap).mac_address);
		nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, get_wiphy_state_calback, state);
		int ret = nl_send_auto(nlSocket, nlMessage);
		
		if (ret<0){
			fprintf(stderr, "Cannot send netlink message");
			nlmsg_free(nlMessage);
			return -CANNOT_SEND_NETLINK_MESSAGE;	
		}
		
		ret = nl_recvmsgs_default(nlSocket);
		if (ret<0){
			fprintf(stderr, "Cannot receive netlink message. Error no: %d (%s)\n", ret, nl_geterror(-ret));
			nlmsg_free(nlMessage);		
			return -CANNOT_RECEIVE_NETLINK_MESSAGE;
		}
		nlmsg_free(nlMessage);
		
		return 0;
	}		

}
