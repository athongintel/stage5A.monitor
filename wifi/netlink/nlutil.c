#include "nlutil.h"

/*UTILS IMPLEMENTATION*/

int hex_value(char c){
	if (c>=97) return c-87;
	if (c>=65) return c-55;
	return c-48;
}

int default_ack_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_ACK.
    int* ret = (int*) arg;
    *ret = 0;
    return NL_STOP;
}

int default_finish_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_FINISH.
    int* ret = (int*) arg;
    *ret = 0;
    return NL_SKIP;
}

int default_noseqcheck_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_SEQ_CHECK.
    return NL_OK;
}

int default_error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err, void *arg) {
    // Callback for errors.
    int* ret = (int*) arg;
    *ret = err->error;
    return NL_STOP;
}

void mac_addr_n2a(char *mac_addr, unsigned char *arg) {
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


char* get_ssid_string(unsigned char *ie, int ielen) {
	uint8_t len;
	uint8_t *data;
	int i;
	int counter;
	
	char* name = malloc(256*sizeof(char));
	counter=0;
	
    while (ielen >= 2 && ielen >= ie[1]) {
        if (ie[0] == 0 && ie[1] >= 0 && ie[1] <= 32) {
            len = ie[1];
            data = ie + 2;
            for (i = 0; i < len; i++) {
                if (isprint(data[i]) && data[i] != ' ' && data[i] != '\\') {
                	name[counter]=data[i];
                	counter++;
                }
                else if (data[i] == ' ' && (i != 0 && i != len -1)) {
                	name[counter] = ' ';
                	counter++;
                }                
            }
            break;
        }
        ielen -= ie[1] + 2;
        ie += ie[1] + 2;
    }
	
	name[counter]='\0';
	return name;
}

char *iftype_name(enum nl80211_iftype iftype){	
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

void mac_addr_a2n(unsigned char* raw, char* mac_addr){
	int i, l;
    l = 0;
    for (i = 0; i < 6; i++) {
        if (l == 0) {
        	raw[i] = hex_value(mac_addr[l])*16 + hex_value(mac_addr[l+1]);
            l += 2;
        } else {
            raw[i] = hex_value(mac_addr[l+1])*16 + hex_value(mac_addr[l+2]);
            l += 3;
        }
    }
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

struct nl_sock* create_genlink_socket(int& nlID){
	
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

/*private handler, not callable*/
static int nl_get_multicast_id_handler(struct nl_msg *msg, void *arg) {
    // Callback for NL_CB_VALID within nl_get_multicast_id()
    struct handler_args* grp = (handler_args*)arg;
    struct nlattr *tb[CTRL_ATTR_MAX + 1];
    struct genlmsghdr* gnlh = (genlmsghdr*) nlmsg_data(nlmsg_hdr(msg));
    struct nlattr* mcgrp;
    int rem_mcgrp;

    nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

    if (!tb[CTRL_ATTR_MCAST_GROUPS]) return NL_SKIP;

    nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {  // This is a loop.
        struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

        nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX, nla_data(mcgrp), nla_len(mcgrp), NULL);

        if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] || !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]) continue;
        //fprintf(stdout, "nl80211 group: %s\n", nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]));
        if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]), grp->group,
                nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]))) {
            continue;
                }

        grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
        break;
    }

    return NL_SKIP;
}

int nl_get_multicast_id(struct nl_sock *sock, const char *family, const char *group) {
    struct nl_msg *msg;
    struct nl_cb *cb;
    int ret, ctrlid;
    struct handler_args grp = { .group = group, .id = -ENOENT, };

    msg = nlmsg_alloc();
    if (!msg) return -ENOMEM;

    cb = nl_cb_alloc(NL_CB_DEFAULT);
    if (!cb) {
        ret = -ENOMEM;
        goto out_fail_cb;
    }

    ctrlid = genl_ctrl_resolve(sock, "nlctrl");

    genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);

    ret = -ENOBUFS;
    NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

    ret = nl_send_auto_complete(sock, msg);
    if (ret < 0) goto out;

    ret = 1;

    nl_cb_err(cb, NL_CB_CUSTOM, default_error_handler, &ret);
    nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, default_ack_handler, &ret);
    nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, nl_get_multicast_id_handler, &grp);

    while (ret > 0) nl_recvmsgs(sock, cb);

    if (ret == 0) ret = grp.id;

    nla_put_failure:
        out:
            nl_cb_put(cb);
        out_fail_cb:
            nlmsg_free(msg);
            return ret;
}
