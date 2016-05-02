#include "network.h"



//WifiController class implementation

WifiController* WifiController::instance = nullptr;

WifiController* WifiController::getInstance(){

	if (!instance)
		instance = new WifiController();
	return instance;
}

WifiController::WifiController(){

	//alloc a netlink socket
	nlSocket = nl_socket_alloc();
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
}

WifiController::~WifiController(){
	if (nlSocket){
		nl_socket_free(nlSocket);
	}
}

vector<WifiInterface*> WifiController::getNetworkInterfaces(){
	vector<WifiInterface*> result;
	fprintf(stdout, "dump wiphy list called");
	dump_wiphy_list(nlSocket, nlID, NULL);
	return result;
}















