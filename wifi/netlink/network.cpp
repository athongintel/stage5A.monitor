#include "network.h"

//WifiDevice class implementation

WifiDevice::WifiDevice(){
	this->nlSocket = create_genlink_socket(this->nlID);
}

WifiDevice::WifiDevice(const struct wiphy* wiphy){
	this->nlSocket = create_genlink_socket(this->nlID);
	//a full struct copy
	this->wiphy = (*wiphy);
}

WifiDevice::~WifiDevice(){
	if (this->nlSocket){
		nl_socket_free(nlSocket);
	}
}

int WifiDevice::getPhysicalIndex(){
	return this->wiphy.phyIndex;
}

WifiInterface* WifiDevice::addVirtualInterface(WifiInterface* interface, string name, enum nl80211_iftype type){
	//TODO
	struct interface vi;
	//WifiDevice* dev = interface->getDevice();
	//int ret = add_network_interface(dev->nlSocket, dev->nlID, &(dev->wiphy), &vi, name.c_str(), type, NULL);
	string typeString[NL80211_IFTYPE_MAX] = {
		"unspec",
		"adhoc",
		"station",
		"ap",
		"ap_vlan",
		"wds",
		"monitor",
		"mesh",
		"p2pclient",
		"p2pgo",
		"p2pdev"};
	string command = string("iw dev ") + interface->getName() + string(" interface add ") + name + string(" type ") + typeString[type];
	int ret = system(command.c_str());
	WifiInterface* result = NULL;
	if (ret == 0){
		WifiInterface* result = new WifiInterface(&vi);
	}

	return result;
}

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
	dump_interface_list(this->nlSocket, this->nlID, dump_interface_list_handler, this);
	return this->wifiInterfaces;
}

int WifiController::dump_interface_list_handler(struct nl_msg *msg, void *args){
	struct genlmsghdr* gnlh = (genlmsghdr*)nlmsg_data(nlmsg_hdr(msg));
	struct nlattr* tb_msg[NL80211_ATTR_MAX + 1];
	
	//retreive calling instance from args
	WifiController* instance = (WifiController*)args;

	//parse received data into tb_msg
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	char interfaceName[20];
	int interfaceIndex;
	int physicIndex;
	unsigned char macaddr[ETH_ALEN];
	
	if (tb_msg[NL80211_ATTR_WIPHY]){
		physicIndex = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY]);
	}
	else{
		physicIndex = -1;
	}
	
	//interface name
	if (tb_msg[NL80211_ATTR_IFNAME])
		memcpy(interfaceName, nla_get_string(tb_msg[NL80211_ATTR_IFNAME]), nla_len(tb_msg[NL80211_ATTR_IFNAME]));
	else
		memset(interfaceName, 0, strlen(interfaceName));
		
	//interface mac address
	if (tb_msg[NL80211_ATTR_MAC]){		
		memcpy(macaddr,(unsigned char*)nla_data(tb_msg[NL80211_ATTR_MAC]), ETH_ALEN);
	}
	else{
		//fill zero because we received nothing
		memset(macaddr, 0, ETH_ALEN);		
	}	
	
	//ifindex
	if (tb_msg[NL80211_ATTR_IFINDEX])
		interfaceIndex = nla_get_u32(tb_msg[NL80211_ATTR_IFINDEX]);
	else
		interfaceIndex = -1;

	//create new interface
	WifiInterface* interface = new WifiInterface();
	interface->interface.ifIndex = interfaceIndex;
	interface->interface.wiphy.phyIndex = physicIndex;
	memcpy(interface->interface.wiphy.mac_addr, macaddr, ETH_ALEN);
	memcpy(interface->interface.name, interfaceName, strlen(interfaceName));
	
	//add this new interface into final list
	instance->wifiInterfaces.push_back(interface);	
	return NL_SKIP;
}


//WifiInterface class implementation

WifiInterface::WifiInterface(){
	this->wifiDevice = new WifiDevice();
}

WifiInterface::WifiInterface(const struct interface* interface){
	this->wifiDevice = new WifiDevice(&(interface->wiphy));
	//a full copy will be performed
	this->interface = (*interface);
}

WifiInterface::~WifiInterface(){
	if (this->wifiDevice!=NULL){
		if (this->wifiDevice->nlSocket!=NULL){
			nl_socket_free(this->wifiDevice->nlSocket);
		}
		delete this->wifiDevice;
	}
}

int WifiInterface::getIfIndex(){
	return this->interface.ifIndex;
}

WifiDevice* WifiInterface::getDevice(){
	return this->wifiDevice;
}

const unsigned char* WifiDevice::getMacAddress(){
	return this->wiphy.mac_addr;
}

string WifiDevice::getDisplayableMacAddress(){
	char mac[20];
	mac_addr_n2a(mac, this->wiphy.mac_addr);
	return string(mac);
}

string WifiInterface::getName() const{
	return string(this->interface.name);
}

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
	
	/*if (bss[NL80211_ATTR_AUTH_TYPE]){
		cout<<"gotcha!"<<nla_data(bss[NL80211_ATTR_AUTH_TYPE])<<endl;
	}*/
	
	//get SSID
	SSID = string(get_ssid_string((unsigned char*)nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS])));
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
		
	unsigned char mac_addr[ETH_ALEN];
	memcpy(mac_addr, (unsigned char*)nla_data(bss[NL80211_BSS_BSSID]), ETH_ALEN);
	int freq = (int)nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	int signal = (int)nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);

	//create new access point
	AccessPoint* accessPoint = new AccessPoint(network, mac_addr, freq, signal);
	memcpy(accessPoint->ap.SSID, SSID.c_str(), SSID.length());
	//add the new access point to current network
	network->accessPoints.push_back(accessPoint);
	
	return NL_SKIP;
}

vector<WifiNetwork*> WifiInterface::fullNetworkScan(){
	
	this->wifiNetworks.clear();

	if (full_network_scan_trigger(this->wifiDevice->nlSocket, this->wifiDevice->nlID, &(this->interface))<0){
		//error
		cout<<"Error: cannot trigger wifi scan"<<endl;		
	}
	else{
		get_network_scan_result(this->wifiDevice->nlSocket, this->wifiDevice->nlID, &(this->interface), full_network_scan_handler, this);			
	}
	return this->wifiNetworks;
}


vector<WifiNetwork*> WifiInterface::freqNetworkScan(){
}

int WifiInterface::connect(AccessPoint* accessPoint){	

	//calling API	
	int ret = connect_to_access_point(this->wifiDevice->nlSocket, this->wifiDevice->nlID, &(this->interface), &(accessPoint->ap), NULL);
	cout<<"Connect returned with code: "<<ret<<endl;
	
	return ret;
}

int WifiInterface::disconnect(){
	//calling API
	int ret;
	LinkState* state = this->getState();
	if (state->state == LinkState::DISCONNECTED){
		cout<<"Already disconnected"<<endl;
		return -1;
	}
	else{
		ret = disconnect_from_access_point(this->wifiDevice->nlSocket, this->wifiDevice->nlID, &(this->interface));
		//cout<<"Disconnect returned with code: "<<ret/*<<" ("<<nl_geterror(ret)<<")"*/<<endl;
		return ret;
	}
}


//AccessPoint class implementation
AccessPoint::AccessPoint(WifiNetwork* network, const unsigned char* mac_addr, int freq, int signal){
	this->network = network;
	memcpy(this->ap.mac_address, mac_addr, ETH_ALEN);
	this->ap.frequency = freq;
	this->ap.signal = signal;
}

string AccessPoint::getDisplayableBSSID(){
	char macbuf[20];
	mac_addr_n2a(macbuf, this->ap.mac_address);
	return string(macbuf);
}

const unsigned char* AccessPoint::getBSSID(){
	return this->ap.mac_address;
}

int AccessPoint::getFrequency(){
	return this->ap.frequency;
}

int AccessPoint::getSignalStrength(){
	return this->ap.signal;
}

AccessPoint::~AccessPoint(){

}

//LinkState class implementation

LinkState::LinkState(){
	this->state = LinkState::DISCONNECTED;
}

LinkState* WifiInterface::getState(){
	
	LinkState* result = new LinkState();
	
	struct wiphy_state state = {.state = DISCONNECTED};
	int ret = get_interface_state(this->wifiDevice->nlSocket, this->wifiDevice->nlID, &(this->interface), &state);	
	
	switch (state.state){
		case ASSOCIATED:
			result->state = LinkState::CONNECTED;
			break;
		case AUTHENTICATED:
			result->state = LinkState::AUTHENTICATED;
			break;
		default:
			result->state = LinkState::DISCONNECTED;
	}
	return result;
}








