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
	AccessPoint(WifiNetwork* network, const unsigned char* mac_addr, int freq, int signal);
	~AccessPoint();

	public:
		WifiNetwork* network;
		string getDisplayableBSSID(); //MAC address
		const unsigned char* getBSSID();
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

	friend class WifiController;

	nl_sock* nlSocket;
	int nlID;
	vector<WifiNetwork*> wifiNetworks;
	struct wiphy* wiphy;
	
	//destructor
	WifiInterface(const char* name, int index, const unsigned char* macadrr);
	~WifiInterface();
	
	//handlers	
	static int full_network_scan_handler(struct nl_msg* msg, void* args);
	
	public:		
		//properties
		string getName();
		int getIfIndex();
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
