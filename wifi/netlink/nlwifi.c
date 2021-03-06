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
		state->state=DISCONNECTED;
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
    struct callback_param *result = (callback_param*)arg;

	int* done = (int*)(result->output);
	int* aborted = done+1;
	
    if (gnlh->cmd == NL80211_CMD_SCAN_ABORTED) {
        //printf("Got NL80211_CMD_SCAN_ABORTED.\n");
        *done = 1;
        *aborted = 1;
    } else if (gnlh->cmd == NL80211_CMD_NEW_SCAN_RESULTS) {
        //printf("Got NL80211_CMD_NEW_SCAN_RESULTS.\n");
        *done = 1;
        *aborted = 0;
    }  // else probably an uninteresting multicast message.

    return NL_SKIP;
}

int network_associate_multicast_callback(struct nl_msg* msg, void* args){
struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	struct callback_param* result = (callback_param*)args;
	const unsigned char** input = (const unsigned char**)result->input;
	
	const unsigned char* st_mac_addr = input[0];
	const unsigned char* ap_mac_addr = input[1];
	int* done = (int*)result->output;
	int* status = done+1;
	
	fprintf(stdout, "network_authenticate_multicast_callback catched some messages. id: %d/%d\n", gnlh->cmd, NL80211_CMD_MAX);
	if (gnlh->cmd == NL80211_CMD_ASSOCIATE){
		fprintf(stdout, "network_authenticate_multicast_callback got CMD_ASSOCIATE event\n");
		nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
		if (tb_msg[NL80211_ATTR_TIMED_OUT]){
			//operation timed out, maybe we moved away from the ap
			//see this came from which AP, get NL80211_ATTR_MAC from the returned
			char msg_mac_addr[20];
			memcpy(msg_mac_addr, (unsigned char*)nla_data(tb_msg[NL80211_ATTR_MAC]), nla_len(tb_msg[NL80211_ATTR_MAC]));
			fprintf(stdout, "Mac address from message: %s\n", msg_mac_addr);
			if (strcmp(msg_mac_addr, (char*)st_mac_addr) == 0){
				//ok, our request had timed out
				*done = 1;
				*status = 1; //0: success, 1: timed out				
			}
			else{
				//this is not our request, just ignores			
			}		
			return NL_SKIP;	
		}
		else if(tb_msg[NL80211_ATTR_FRAME]){
			//see what's inside this frame
			//the frame is just an array of bits which contains the information under a convention of order
		
			uint8_t *frame;
			size_t len;
			int i;
			unsigned char macbuf[ETH_ALEN];
			uint16_t tmp;			
		
			frame = (uint8_t*)nla_data(tb_msg[NL80211_ATTR_FRAME]);
			len = nla_len(tb_msg[NL80211_ATTR_FRAME]);

			if (len < 26) {
				printf(" [invalid frame: ");
				return NL_SKIP;
				//this is a bad frame. just ignore			
			}
			else{
				//check if we are the destination of this messsage
				memcpy(macbuf, frame + 4, ETH_ALEN);				
				char mactmp1[20]; 
				char mactmp2[20]; 
				mac_addr_n2a(mactmp1, macbuf);
				mac_addr_n2a(mactmp2, st_mac_addr);				
				fprintf(stdout, "macbuf %s and st_mac_addr %s\n", mactmp1, mactmp2);
				if (strcmp((const char*)macbuf, (const char*)st_mac_addr)==0){					
					fprintf(stdout, "yes we are the destination\n");
					//check the source of this message
					memcpy(macbuf, frame + 10, ETH_ALEN);
					mac_addr_n2a(mactmp1, macbuf);
					mac_addr_n2a(mactmp2, ap_mac_addr);
					fprintf(stdout, "macbuf %s and ap_mac_addr %s\n", mactmp1, mactmp2);
					if (strcmp((const char*)macbuf, (const char*)ap_mac_addr)==0){					
						fprintf(stdout, "yes this is from the ap we wanted to connect\n");
						*done = 1; //ok, we got what we wanted
						//check status
						switch (frame[0] & 0xfc) {
							case 0x10: /* assoc resp */
							case 0x30: /* reassoc resp */
								/* status */
								*status = (frame[27] << 8) + frame[26];
								//printf(" status: %d\n", tmp);
								break;
							case 0x00: /* assoc req */
							case 0x20: /* reassoc req */
								break;
							case 0xb0: /* auth */
								/* status */
								*status = (frame[29] << 8) + frame[28];
								//printf(" status: %d\n", tmp);
								break;
							case 0xa0: /* disassoc */
							case 0xc0: /* deauth */
								/* reason */
								*status = (frame[25] << 8) + frame[24];
								//printf(" reason %d\n", tmp);
								break;
						}
					}
					else{
						//Just skip. This came from an other AP, maybe a network lag??
					}					
				}
			}	
		}
	}	
	
	return NL_SKIP;
}

int network_authenticate_multicast_callback(struct nl_msg* msg, void* args){
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	struct callback_param* result = (callback_param*)args;
	const unsigned char** input = (const unsigned char**)result->input;
	
	const unsigned char* st_mac_addr = input[0];
	const unsigned char* ap_mac_addr = input[1];
	int* done = (int*)result->output;
	int* status = done+1;
	
	fprintf(stdout, "network_authenticate_multicast_callback catched some messages. id: %d/%d\n", gnlh->cmd, NL80211_CMD_MAX);
	if (gnlh->cmd == NL80211_CMD_AUTHENTICATE){
		fprintf(stdout, "network_authenticate_multicast_callback got CMD_AUTHENTICATE event\n");
		nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
		if (tb_msg[NL80211_ATTR_TIMED_OUT]){
			//operation timed out, maybe we moved away from the ap
			//see this came from which AP, get NL80211_ATTR_MAC from the returned
			char msg_mac_addr[20];
			memcpy(msg_mac_addr, (unsigned char*)nla_data(tb_msg[NL80211_ATTR_MAC]), nla_len(tb_msg[NL80211_ATTR_MAC]));
			fprintf(stdout, "Mac address from message: %s\n", msg_mac_addr);
			if (strcmp(msg_mac_addr, (char*)st_mac_addr) == 0){
				//ok, our request had timed out
				*done = 1;
				*status = 1; //0: success, 1: timed out				
			}
			else{
				//this is not our request, just ignores			
			}		
			return NL_SKIP;	
		}
		else if(tb_msg[NL80211_ATTR_FRAME]){
			//see what's inside this frame
			//the frame is just an array of bits which contains the information under a convention of order
		
			uint8_t *frame;
			size_t len;
			int i;
			unsigned char macbuf[ETH_ALEN];
			uint16_t tmp;			
		
			frame = (uint8_t*)nla_data(tb_msg[NL80211_ATTR_FRAME]);
			len = nla_len(tb_msg[NL80211_ATTR_FRAME]);

			if (len < 26) {
				printf(" [invalid frame: ");
				return NL_SKIP;
				//this is a bad frame. just ignore			
			}
			else{
				//check if we are the destination of this messsage
				memcpy(macbuf, frame + 4, ETH_ALEN);				
				char mactmp1[20]; 
				char mactmp2[20]; 
				mac_addr_n2a(mactmp1, macbuf);
				mac_addr_n2a(mactmp2, st_mac_addr);				
				fprintf(stdout, "macbuf %s and st_mac_addr %s\n", mactmp1, mactmp2);
				if (strcmp((const char*)macbuf, (const char*)st_mac_addr)==0){					
					fprintf(stdout, "yes we are the destination\n");
					//check the source of this message
					memcpy(macbuf, frame + 10, ETH_ALEN);
					mac_addr_n2a(mactmp1, macbuf);
					mac_addr_n2a(mactmp2, ap_mac_addr);
					fprintf(stdout, "macbuf %s and ap_mac_addr %s\n", mactmp1, mactmp2);
					if (strcmp((const char*)macbuf, (const char*)ap_mac_addr)==0){					
						fprintf(stdout, "yes this is from the ap we wanted to connect\n");
						*done = 1; //ok, we got what we wanted
						//check status
						switch (frame[0] & 0xfc) {
							case 0x10: /* assoc resp */
							case 0x30: /* reassoc resp */
								/* status */
								*status = (frame[27] << 8) + frame[26];
								//printf(" status: %d\n", tmp);
								break;
							case 0x00: /* assoc req */
							case 0x20: /* reassoc req */
								break;
							case 0xb0: /* auth */
								/* status */
								*status = (frame[29] << 8) + frame[28];
								//printf(" status: %d\n", tmp);
								break;
							case 0xa0: /* disassoc */
							case 0xc0: /* deauth */
								/* reason */
								*status = (frame[25] << 8) + frame[24];
								//printf(" reason %d\n", tmp);
								break;
						}
					}
					else{
						//Just skip. This came from an other AP, maybe a network lag??
					}					
				}
			}	
		}
	}	
	
	return NL_SKIP;
}

int network_disconnect_multicast_callback(struct nl_msg* msg, void* arg){
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	struct callback_param* result = (callback_param*)arg;
	
	const int* ifIndex = (const int*)result->input;
	int* done  = (int*)result->output;
	
	//fprintf(stdout, "disconnect multicast callback called\n");
	if (gnlh->cmd == NL80211_CMD_DISCONNECT){
		//fprintf(stdout, "disconnect multicast callback got DISCONNECT RETURN\n");
		
		//parse message data and check for IFINDEX
		nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
		if (tb_msg[NL80211_ATTR_IFINDEX]){
			//fprintf(stdout,  "OK, get disconnect event from interface %d, required %d\n", nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]), results->extra);
			if (nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]) == *ifIndex){
				*done = 1;
				//OK, we got what we want
				return NL_STOP;
			}
		}
	}
	
	return NL_SKIP;
}

int add_network_interface_handler(struct nl_msg* nlMessage, void* arg){
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(nlMessage));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	
	fprintf(stdout, "add_network_interface_handler called\n");
	fprintf(stdout, "unknown event %d ()\n",
		       gnlh->cmd);
	return NL_SKIP;
}


/*API IMPLEMENTATION*/

int add_network_interface(struct nl_sock* nlSocket, int netlinkID, const struct wiphy* wiphy, struct interface* interface, const char* name, enum nl80211_iftype type, void* args){
	//using this socket to send dump request to kernel and receive response
	struct nl_msg* nlMessage;
	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		//cannot allocate message
		fprintf(stderr, "Error: cannot allocate netlink message\n");
		return -1;
	}
	
	fprintf(stdout, "add network interface called\n");
	
	//set command and add attribute for a wiphy dump
	genlmsg_put(nlMessage, 0, 0, netlinkID, 0, 0, NL80211_CMD_NEW_INTERFACE, 0);
	nla_put_u32(nlMessage, NL80211_ATTR_WIPHY, wiphy->phyIndex);  // Add message attribute, device to use
	nla_put_u32(nlMessage, NL80211_ATTR_IFTYPE, type);  // Add message attribute, which type of interface to be created
	//NLA_PUT_STRING(msg, NL80211_ATTR_IFNAME, name);
	//NLA_PUT_U32(msg, NL80211_ATTR_IFTYPE, type);
	nla_put(nlMessage, NL80211_ATTR_IFNAME, strlen(name), name);  // Add message attribute, which name defined the new interface
	nl_socket_modify_cb(nlSocket, NL_CB_MSG_IN, NL_CB_CUSTOM, add_network_interface_handler, args);
	
	//send the message
	int ret = nl_send_auto(nlSocket, nlMessage);
	if (ret<0){
		fprintf(stdout, "add network interface message send failed\n");
	}
	//block and wait for response packet. Returned after callback.
	ret = nl_recvmsgs_default(nlSocket);
	if (ret<0){
		fprintf(stdout, "add network interface message receive failed\n");
	}
	

	//free the message
   	nlmsg_free(nlMessage);
	
	return ret;	
}

int dump_interface_list(struct nl_sock* nlSocket, int nlID, nl_recvmsg_msg_cb_t handler, void* args){
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

int full_network_scan_trigger(struct nl_sock* nlSocket, int nlID, struct interface* interface) {
	// Starts the scan and waits for it to finish. Does not return until the scan is done or has been aborted (extra).
	int scan_result[2];
	scan_result[0]=0; //done
	scan_result[1]=0; //aborted
	
	struct callback_param results = { .input = NULL, .output = scan_result};
	
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
	nla_put_u32(msg, NL80211_ATTR_IFINDEX, interface->ifIndex);  // Add message attribute, which interface to use.
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
	
	while (!scan_result[0])
		nl_recvmsgs(nlSocket, cb);  // Now wait until the scan is done or aborted.
	
	if (scan_result[1]) {
		printf("ERROR: Kernel aborted scan.\n");
		return 1;
	}

	// Cleanup.
	nlmsg_free(msg);
	nl_cb_put(cb);
	nl_socket_drop_membership(nlSocket, mcid);  // No longer need this.
	return 0;
}

int get_network_scan_result(struct nl_sock* nlSocket, int nlID, struct interface* interface, nl_recvmsg_msg_cb_t handler, void* args){
	struct nl_msg* nlMessage;
	nlMessage = nlmsg_alloc();  // Allocate a message.
	if (!nlMessage){
		fprintf(stderr,"Error: cannot allocate netlink message\n");
		return -1;
	}
	else{
		genlmsg_put(nlMessage, 0, 0, nlID, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);  // Setup which command to run.
		nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, interface->ifIndex);  // Add message attribute, which interface to use.
		nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, handler, args);  // Add the callback.
		int ret = nl_send_auto(nlSocket, nlMessage);  // Send the message.

		ret = nl_recvmsgs_default(nlSocket);
		nlmsg_free(nlMessage);
		if (ret < 0) {
			fprintf(stderr,"ERROR: nl_recvmsgs_default returned %d\n",ret);
		}
		return ret;
	}	
}

int connect_to_access_point(struct nl_sock* nlSocket, int netlinkID, struct interface* interface, struct access_point* ap, void* args){
			
	struct nl_msg* nlMessage;
	struct nl_cb* nlCallback;
	int mcid;
	
	nlMessage = nlmsg_alloc();
	if (!nlMessage){
		fprintf(stderr,"Error: cannot allocate netlink message\n");
		return -1;
	}
	else{
		genlmsg_put(nlMessage, 0, 0, netlinkID, 0, 0, NL80211_CMD_AUTHENTICATE, 0);
		nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, interface->ifIndex);
		nla_put(nlMessage, NL80211_ATTR_SSID, strlen(ap->SSID), ap->SSID);
		if (ap->frequency>0){
			nla_put_u32(nlMessage, NL80211_ATTR_WIPHY_FREQ, ap->frequency);			
		}
		if (strlen((const char*)(ap->mac_address))>0){
			nla_put(nlMessage, NL80211_ATTR_MAC, ETH_ALEN, ap->mac_address);
		}
		nla_put_u32(nlMessage, NL80211_ATTR_AUTH_TYPE, ap->authType);
		
		//listen to multicast event
		mcid = nl_get_multicast_id(nlSocket, NL80211_GENL_NAME, NL80211_MULTICAST_GROUP_MLME);
		int ret;		
		if (mcid >= 0) {
			ret = nl_socket_add_membership(nlSocket, mcid);
			if (ret){
				fprintf(stderr,"Cannot listen to multicast connect event");
				return ret;
			}
			//adding callback
			unsigned char* input[2];
			input[0] = interface->wiphy.mac_addr; //source's mac address
			input[1] = ap->mac_address; //access point's mac address
			
			int connect_result[2];
			connect_result[0] = 0; //done
			connect_result[1] = -1; //status, 0 means success
						
			struct callback_param results = {.input = input, .output = connect_result};
			//alloc and set the callback
			nlCallback = nl_cb_alloc(NL_CB_DEFAULT);
			if (!nlCallback) {
				fprintf(stderr,"Error: Failed to allocate netlink callbacks.\n");
				nlmsg_free(nlMessage);
				return -ENOMEM;
			}
			nl_cb_set(nlCallback, NL_CB_MSG_IN, NL_CB_CUSTOM, network_authenticate_multicast_callback, &results);		
			
			//send the message and wait for response
			int ret = nl_send_auto(nlSocket, nlMessage);
			if (ret < 0){
				fprintf(stderr, "Cannot send authenticate request\n");
				return ret;
			}
			
			char test_mac[20];
			mac_addr_n2a(test_mac, ap->mac_address);
			fprintf(stdout, "Authenticate request sent to %s - %s\n", ap->SSID, test_mac);
			
			//need to loop because nl_recvmsgs only block once message
			while (!connect_result[0])
				nl_recvmsgs(nlSocket, nlCallback);
			
			if (connect_result[1]==0){
				fprintf(stdout, "Authenticated, sending associate request...\n");
				nlMessage = nlmsg_alloc();
				if (!nlMessage){
					fprintf(stderr,"Error: cannot allocate netlink message\n");
					return -1;
				}
				else{
					genlmsg_put(nlMessage, 0, 0, netlinkID, 0, 0, NL80211_CMD_ASSOCIATE, 0);
					nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, interface->ifIndex);
					nla_put(nlMessage, NL80211_ATTR_SSID, strlen(ap->SSID), ap->SSID);
					if (ap->frequency>0){
						nla_put_u32(nlMessage, NL80211_ATTR_WIPHY_FREQ, ap->frequency);			
					}
					if (strlen((const char*)(ap->mac_address))>0){
						nla_put(nlMessage, NL80211_ATTR_MAC, ETH_ALEN, ap->mac_address);
					}
					nla_put_u32(nlMessage, NL80211_ATTR_AUTH_TYPE, NL80211_AUTHTYPE_SHARED_KEY);
		
					//listen to multicast event
					//mcid = nl_get_multicast_id(nlSocket, NL80211_GENL_NAME, NL80211_MULTICAST_GROUP_MLME);
					/*int ret;		
					if (mcid >= 0) {
						ret = nl_socket_add_membership(nlSocket, mcid);
						if (ret){
							fprintf(stderr,"Cannot listen to multicast connect event");
							return ret;
						}*/
						//adding callback
					//unsigned char* input[2];
					input[0] = interface->wiphy.mac_addr; //source's mac address
					input[1] = ap->mac_address; //access point's mac address
		
					//int connect_result[2];
					connect_result[0] = 0; //done
					connect_result[1] = -1; //status, 0 means success
					
					//struct callback_param results;
					results = {.input = input, .output = connect_result};
					//alloc and set the callback
					nlCallback = nl_cb_alloc(NL_CB_DEFAULT);
					if (!nlCallback) {
						fprintf(stderr,"Error: Failed to allocate netlink callbacks.\n");
						nlmsg_free(nlMessage);
						return -ENOMEM;
					}
					nl_cb_set(nlCallback, NL_CB_MSG_IN, NL_CB_CUSTOM, network_associate_multicast_callback, &results);		
		
					//send the message and wait for response
					//int 
					ret = nl_send_auto(nlSocket, nlMessage);
					if (ret < 0){
						fprintf(stderr, "Cannot send authenticate request\n");
						return ret;
					}
		
					//char test_mac[20];
					mac_addr_n2a(test_mac, ap->mac_address);
					fprintf(stdout, "Authenticate request sent to %s - %s\n", ap->SSID, test_mac);
		
					//need to loop because nl_recvmsgs only block once message
					while (!connect_result[0])
						nl_recvmsgs(nlSocket, nlCallback);
				}
			}
			else{
				nl_socket_drop_membership(nlSocket, mcid);
				nlmsg_free(nlMessage);
				return -1;
			}
			
			//no longer listen to multicast event
			nl_socket_drop_membership(nlSocket, mcid);
			nlmsg_free(nlMessage);
			
			return 0;
		}		
	}		
}

/* @NL80211_CMD_DISCONNECT: drop a given connection; also used to notify
 *	userspace that a connection was dropped by the AP or due to other
 *	reasons, for this the %NL80211_ATTR_DISCONNECTED_BY_AP and
 *	%NL80211_ATTR_REASON_CODE attributes are used.
 */

int disconnect_from_access_point(struct nl_sock* nlSocket, int netlinkID, struct interface* interface){
	
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
		nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, interface->ifIndex);				
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
			int disconnect_result[2];
			disconnect_result[0] = 0; //done
			disconnect_result[1] = 0;
			
			struct callback_param results = {.input = NULL, .output = disconnect_result};
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
			while (!disconnect_result[0]){
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

int get_interface_state(struct nl_sock* nlSocket, int netlinkID, struct interface* interface, struct wiphy_state* state){
	
	return get_network_scan_result(nlSocket, netlinkID, interface, get_wiphy_state_calback, state);

}
