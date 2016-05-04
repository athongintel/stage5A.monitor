#include "network.h"


//WifiController class implementation

WifiController::WifiController(){

	//alloc a netlink socket	
	this->nlSocket = create_genlink_socket(nlID);
}

WifiController::~WifiController(){
	if (this->nlSocket){
		nl_socket_free(nlSocket);
	}
}

vector<WifiInterface*> WifiController::getNetworkInterfaces(){
	this->wifiInterfaces.clear();
	/*	using api from nlwifi to dump a list of wifi interface
		for each interface found, handler is called
		'this' is passed into callback function as argument to update its 'wifiInterfaces'
	*/
	dump_wiphy_list(this->nlSocket, this->nlID, dump_wiphy_list_handler, this);
	return this->wifiInterfaces;
}

int WifiController::dump_wiphy_list_handler(struct nl_msg *msg, void *args){
	struct genlmsghdr *gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	
	//retreive calling instance from args
	WifiController* instance = (WifiController*)args;

	//parse received data into tb_msg
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	//create new interface
	WifiInterface* interface = new WifiInterface();

	//interface name
	if (tb_msg[NL80211_ATTR_IFNAME])
		interface->name = string((char*)nla_get_string(tb_msg[NL80211_ATTR_IFNAME]));
	else
		interface->name = "";
	//interface mac address
	if (tb_msg[NL80211_ATTR_MAC]){
		char macaddr[20];
		mac_addr_n2a(macaddr, (unsigned char*)nla_data(tb_msg[NL80211_ATTR_MAC]));
		interface->address = string(macaddr);
	}
	else
		interface->address = "";	
	
	//if index
	if (tb_msg[NL80211_ATTR_IFINDEX])
		interface->ifIndex = nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]);
	else
		interface->ifIndex = -1;
	//add this new interface into final list
	instance->wifiInterfaces.push_back(interface);	
	return NL_SKIP;
}


//WifiInterface class implementation

int WifiInterface::full_network_scan_handler(struct nl_msg* msg, void* args){
	
	struct genlmsghdr* gnlh = (genlmsghdr*) nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *bss[NL80211_BSS_MAX + 1];

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

	string SSID;
	char mac_addr[20];

	// Parse return message and error check.
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[NL80211_ATTR_BSS])
		return NL_SKIP;

	//Parse nested messages
	if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy))
		return NL_SKIP;
	
	if (!bss[NL80211_BSS_BSSID])
		return NL_SKIP;
	if (!bss[NL80211_BSS_INFORMATION_ELEMENTS])
		return NL_SKIP;

	//get instance from args
	WifiInterface* wifiInterface = (WifiInterface*) args;
	
	
	// Start printing.
	//get SSID
	SSID = string(get_ssid_string(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS])));
	//cout<<"SSID:"<<SSID<<" ";
	//check if SSID is already on the list
	bool newNetwork = false;
	WifiNetwork* network;
	newNetwork = wifiInterface->wifiNetworks.empty();
	if (!newNetwork){
		newNetwork = true;
		for (auto &net : wifiInterface->wifiNetworks){
			if (net->SSID == SSID){
				newNetwork = false;
				network = net;
				break;
			}
		}
	}
	if (newNetwork){
		network = new WifiNetwork();
		network->SSID = SSID;
	}
	//create new access point
	AccessPoint* ap = new AccessPoint();
	
	
	network->accessPoints->push_back();
	//get BSSID

	//get frequency

	
	mac_addr_n2a(mac_addr, nla_data(bss[NL80211_BSS_BSSID]));
	printf("%s, ", mac_addr);
	printf("%d MHz, ", nla_get_u32(bss[NL80211_BSS_FREQUENCY]));

	printf("\n");
	return NL_SKIP;
}

vector<WifiNetwork*> WifiInterface::fullNetworkScan(){
	
	this->wifiNetworks.clear();
	int nlID;
	struct nl_sock* nlSocket = create_genlink_socket(nlID);
	if (full_network_scan_trigger(nlSocket, this->ifIndex, nlID)<0){
		//error
		cout<<"Error: cannot trigger wifi scan"<<endl;
		
	}
	else{
		struct nl_msg* nlMessage;
		nlMessage = nlmsg_alloc();  // Allocate a message.
		if (!nlMessage){
			cout<<"Error: cannot allocate netlink message"<<endl;
		}
		else{
			genlmsg_put(nlMessage, 0, 0, nlID, 0, NLM_F_DUMP, NL80211_CMD_GET_SCAN, 0);  // Setup which command to run.
			nla_put_u32(nlMessage, NL80211_ATTR_IFINDEX, this->ifIndex);  // Add message attribute, which interface to use.
			nl_socket_modify_cb(nlSocket, NL_CB_VALID, NL_CB_CUSTOM, full_network_scan_handler, this);  // Add the callback.
			int ret = nl_send_auto(nlSocket, nlMessage);  // Send the message.

			ret = nl_recvmsgs_default(nlSocket);
			nlmsg_free(nlMessage);
			if (ret < 0) {
				cout<<"ERROR: nl_recvmsgs_default() returned "<<ret<<" ("<<nl_geterror(-ret)<<" )"<<endl;
			}
		}	
	}
	return this->wifiNetworks;	
}










