#include "network.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv){
	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
	for (auto &i : interfaces){
		cout<<i->name<<" "<<i->address<<endl;
		cout<<"I'm performing a full scan..."<<endl;
		i->fullNetworkScan();
	}

	return 0;
}
