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
		int frequency;
		int strength;
};


class WifiNetwork{
	
	public:
		string SSID;
		vector<AccessPoint*> accessPoints;

};

class LinkState{
	public:
		enum LinkStateEnum {
			DISCONNECTED,
			AUTHENTICATED,
			CONNECTED					
		};
		
		LinkState();
		enum LinkStateEnum state;
};


class WifiInterface{

	nl_sock* nlSocket;
	int nlID;
	vector<WifiNetwork*> wifiNetworks;
	
	//destructor
	~WifiInterface();
	

	//handlers	
	static int full_network_scan_handler(struct nl_msg* msg, void* args);
	public:
		WifiInterface();
		//properties
		string name;
		string address;
		int ifIndex;

		//methods
		vector<WifiNetwork*> fullNetworkScan();
		vector<WifiNetwork*> freqNetworkScan();
		LinkState* getState();
		
		int connect(AccessPoint* ap);
		int disconnect();
};


class WifiController{

	nl_sock* nlSocket;
	int nlID;
	
	vector<WifiInterface*> wifiInterfaces;
	
	//destructor
	~WifiController();

	//handlers
	static int dump_wiphy_list_handler(struct nl_msg* msg, void* args);

	public:
		WifiController();
		vector<WifiInterface*> getNetworkInterfaces();		
};
