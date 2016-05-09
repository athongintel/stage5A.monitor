#include "network.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv){
	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
	for (auto &i : interfaces){
		cout<<i->name<<" "<<i->address<<endl;
		cout<<"I'm performing a full scan..."<<endl;
		vector<WifiNetwork*> networks = i->fullNetworkScan();
		cout<<"Network count: "<<networks.size()<<endl;
		for (auto &net : networks){
			cout<<(net->SSID.empty()? "Hidden network" : net->SSID)<<endl;
			for (auto &ap : net->accessPoints){
				cout<<"  "<<ap->BSSID<<"  "<<ap->frequency<<"  "<<endl;
			}
		}
	}

	return 0;
}
