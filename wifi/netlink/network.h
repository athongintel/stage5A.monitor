#include "nlwifi.h"

#include <iostream>
#include <vector>

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
	public:
		string name;
};


class WifiController{

	nl_sock* nlSocket;
	int nlID;

	//constructor
	WifiController();
	
	//destructor
	~WifiController();

	public:
		static WifiController* instance;
		static WifiController* getInstance();
		vector<WifiInterface*> getNetworkInterfaces();
		vector<WifiNetwork*> fullNetworkScan(string interface);

};
