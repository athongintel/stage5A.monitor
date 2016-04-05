#include <iostream>
#include "iwapi.h"

int main(){

	try{
		Iwapi* iw = new Iwapi();
		
		//get all wireless device
		std::vector<std::string> inames = iw->getWlanInterfaceNames();
		
		//scan all available networks
		std::vector<struct Network> networks  = iw->scanNetworks(inames[1]);
		std::cout<<"network size: "<<networks.size()<<std::endl;
		for (int i=0; i<networks.size();i++){
			//printout again
			std::cout<<networks[i].ssid<<std::endl;
			std::cout<<"ap size: "<<networks[i].aps.size()<<std::endl;
			for (int j=0;j<networks[i].aps.size();j++){
				std::cout<<networks[i].aps[j].id<<" "<<networks[i].aps[j].signal<<" "<<networks[i].aps[j].encryption<<std::endl;
			}
		}
		
	}
	catch (std::exception& e){
		std::cout<<e.what();
		return 1;
	}
}