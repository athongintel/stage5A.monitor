#include "nlwifi.h"

#include <iostream>
#include <vector>
#include <mutex>
#include <string>

#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/socket.h>

using namespace std;


class WifiController;
class WifiNetwork;
class WifiInterface;
class AccessPoint;


class AccessPoint{ 
	friend class WifiInterface;

	struct access_point* ap;
	
	
	//private constructor
	AccessPoint();

	public:
		WifiNetwork* network;
		string getBSSID(); //MAC address
		int getFrequency();
		int getSignalStrength();
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
	struct wiphy* wiphy;
	
	//destructor
	~WifiInterface();
	
	//handlers	
	static int full_network_scan_handler(struct nl_msg* msg, void* args);
	
	public:
		WifiInterface(string name, int index, const unsigned char* macadrr);
		//properties
		string name;
		int getIfIndex();
		void setIfIndex(int index);
		const unsigned char* getMacAddress();
		string getDisplayableMacAddress();

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
