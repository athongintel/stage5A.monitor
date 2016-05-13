#include "network.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv){
	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
	for (auto &i : interfaces){
		cout<<i->name<<" "<<i->address<<endl;
		//cout<<"I'm performing a full scan..."<<endl;
		//vector<WifiNetwork*> networks = i->fullNetworkScan();
		//cout<<"Network count: "<<networks.size()<<endl;
		//for (auto &net : networks){
		//	cout<<(net->SSID.empty()? "Hidden network" : net->SSID)<<endl;
			/*if (strcmp(net->SSID, "(Aix*Marseille universite")==0){
				cout<<"Trying to connect to "<<net->SSID<<" access point..."<<endl;				
			}
			bool connect = false;*/
		//	for (auto &ap : net->accessPoints){
		//		cout<<"  "<<ap->BSSID<<"  "<<ap->frequency<<"  "<<endl;
				/*if (!connect){
					//connect to first access point
					connect = true;
					i->connect(ap);
				}*/
		//	}
		//}
		i->disconnect();
		cout<<"Disconnected!"<<endl;
	}

	return 0;
}
