#include "nlwifi.h"


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

int dump_wiphy_list_handler(struct nl_msg *msg, void *arg){
	/*struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	unsigned int *wiphy = arg;
	const char *indent = "";

	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (wiphy && tb_msg[NL80211_ATTR_WIPHY]) {
		unsigned int thiswiphy = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
		indent = "\t";
		if (*wiphy != thiswiphy)
			printf("phy#%d\n", thiswiphy);
		*wiphy = thiswiphy;
	}

	if (tb_msg[NL80211_ATTR_IFNAME])
		printf("%sInterface %s\n", indent, nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
	else
		printf("%sUnnamed/non-netdev interface\n", indent);
	if (tb_msg[NL80211_ATTR_IFINDEX])
		printf("%s\tifindex %d\n", indent, nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
	if (tb_msg[NL80211_ATTR_WDEV])
		printf("%s\twdev 0x%llx\n", indent,
		       (unsigned long long)nla_get_u64(tb_msg[NL80211_ATTR_WDEV]));
	if (tb_msg[NL80211_ATTR_MAC]) {
		char mac_addr[20];
		mac_addr_n2a(mac_addr, nla_data(tb_msg[NL80211_ATTR_MAC]));
		printf("%s\taddr %s\n", indent, mac_addr);
	}
	if (tb_msg[NL80211_ATTR_SSID]) {
		printf("%s\tssid ", indent);
		print_ssid_escaped(nla_len(tb_msg[NL80211_ATTR_SSID]),
				   nla_data(tb_msg[NL80211_ATTR_SSID]));
		printf("\n");
	}
	if (tb_msg[NL80211_ATTR_IFTYPE])
		printf("%s\ttype %s\n", indent, iftype_name(nla_get_u32(tb_msg[NL80211_ATTR_IFTYPE])));
	if (!wiphy && tb_msg[NL80211_ATTR_WIPHY])
		printf("%s\twiphy %d\n", indent, nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]));
	if (tb_msg[NL80211_ATTR_WIPHY_FREQ]) {
		uint32_t freq = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]);

		printf("%s\tchannel %d (%d MHz)", indent,
		       ieee80211_frequency_to_channel(freq), freq);

		if (tb_msg[NL80211_ATTR_CHANNEL_WIDTH]) {
			printf(", width: %s",
				channel_width_name(nla_get_u32(tb_msg[NL80211_ATTR_CHANNEL_WIDTH])));
			if (tb_msg[NL80211_ATTR_CENTER_FREQ1])
				printf(", center1: %d MHz",
					nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ1]));
			if (tb_msg[NL80211_ATTR_CENTER_FREQ2])
				printf(", center2: %d MHz",
					nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ2]));
		} else if (tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]) {
			enum nl80211_channel_type channel_type;

			channel_type = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]);
			printf(" %s", channel_type_name(channel_type));
		}

		printf("\n");
	}
	*/
	fprintf(stdout,"callback called");
	return NL_SKIP;
}

int dump_wiphy_list(struct nl_sock* nlSocket, int nlID, void* handler){
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
	nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, dump_wiphy_list_handler, NULL);
	
	//send the message
	int ret = nl_send_auto(nlSocket, nlMessage);
	printf("NL80211_CMD_GET_INTERFACE sent %d bytes to the kernel.\n", ret);
	//block and wait for response packet. Returned after callback.
    	ret = nl_recvmsgs_default(nlSocket);

	//free the message
   	nlmsg_free(nlMessage);
	
	return 0;
}




























