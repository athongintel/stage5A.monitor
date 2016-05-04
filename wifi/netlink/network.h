#include "nlwifi.h"

#include <iostream>
#include <vector>
#include <mutex>
#include <string>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/socket.h>

using namespace std;

class WifiNetwork;

class AccessPoint{ 

	public:
		WifiNetwork* network;
		string BSSID; //MAC address
		float frequency;
		float strength;
};


class WifiNetwork{
	
	public:
		string SSID;
		vector<AccessPoint*> accessPoints;

};


class WifiInterface{

	vector<WifiNetwork*> wifiNetworks;

	//handlers	
	static int full_network_scan_handler(struct nl_msg* msg, void* args);
	public:
		//properties
		string name;
		string address;
		int ifIndex;

		//methods
		vector<WifiNetwork*> fullNetworkScan();
};


class WifiController{

	nl_sock* nlSocket;
	int nlID;
	
	vector<WifiInterface*> wifiInterfaces;
	
	//constructor
	
	//destructor
	~WifiController();

	//handlers
	static int dump_wiphy_list_handler(struct nl_msg* msg, void* args);

	public:
		WifiController();
		vector<WifiInterface*> getNetworkInterfaces();		
};
