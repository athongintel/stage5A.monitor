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
		interface->ifIndex = nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]));
	else
		interface->ifIndex = -1;
	//add this new interface into final list
	instance->wifiInterfaces.push_back(interface);	
	return NL_SKIP;
}


//WifiInterface class implementation

vector<WifiNetwork*> WifiInterface::fullNetworkScan(){
	
	int nlID;
	struct nl_sock* nlSocket = create_genlink_socket(nlID);
	if (full_network_scan_trigger(nlSocket, this->ifIndex, nlID)<0){
		//error	
	}
	
}










