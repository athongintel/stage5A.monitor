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
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	
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
	
	
	//get SSID
	SSID = string(get_ssid_string(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS])));
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
		wifiInterface->wifiNetworks.push_back(network);
	}
			
	//create new access point
	AccessPoint* ap = new AccessPoint();
	//get BSSID
	mac_addr_n2a(mac_addr, nla_data(bss[NL80211_BSS_BSSID]));
	ap->BSSID = string(mac_addr);
	//get frequency
	ap->frequency = (int)nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	ap->network = network;
	
	//add the new access point to current network
	network->accessPoints.push_back(ap);
	
	return NL_SKIP;
}

vector<WifiNetwork*> WifiInterface::fullNetworkScan(){
	
	this->wifiNetworks.clear();
	int nlID;
	struct nl_sock* nlSocket = create_genlink_socket(nlID);
	if (full_network_scan_trigger(nlSocket, nlID, this->ifIndex)<0){
		//error
		cout<<"Error: cannot trigger wifi scan"<<endl;		
	}
	else{
		get_scan_result(nlSocket, nlID, this->ifIndex, full_network_scan_handler, this);			
	}
	return this->wifiNetworks;	
}


vector<WifiNetwork*> WifiInterface::freqNetworkScan(){
}

int WifiInterface::connect(AccessPoint* accessPoint){

	int nlID;
	struct nl_sock* nlSocket = create_genlink_socket(nlID);
	
	//construct a backward access point struct
	struct access_point ap;
	strcpy(ap.SSID, accessPoint->network->SSID.c_str());
	mac_addr_a2n(ap.mac_address, accessPoint->BSSID.c_str());
	ap.frequency = accessPoint->frequency;

	//calling API
	int ret = connect_to_access_point(nlSocket, nlID, this->ifIndex, &ap, NULL);
	cout<<"Connect returned with code: "<<ret<<endl;
	
	return ret;
}

int WifiInterface::disconnect(){
	int nlID;
	struct nl_sock* nlSocket = create_genlink_socket(nlID);
	
	//calling API
	int ret = disconnect_from_access_point(nlSocket, nlID, this->ifIndex);
	cout<<"Disconnect returned with code: "<<ret/*<<" ("<<nl_geterror(ret)<<")"*/<<endl;
	return ret;
}








