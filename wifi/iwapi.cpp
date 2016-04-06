#include "iwapi.h"

Iwapi::Iwapi() throw (){
	//check for iw version
	const std::string result = Utils::exec("iw --version");
	
	std::smatch match;
	std::regex rgx("iw version\\s([0-9]+[.][0-9])+");

    if (std::regex_search(result.begin(), result.end(), match, rgx)){
		this->version = match[1];
	}
	else{
		throw "Error: iw not found on this computer!";
	}
	
}

std::vector<std::string> Iwapi::getWlanInterfaceNames(){
	const std::string result = Utils::exec("iw dev");
	std::vector<std::string> interfaces;	
	
	std::smatch match;
	std::regex rgx("Interface\\s(wlan[0-9]+)");
	
	std::string::const_iterator textIterator = result.begin();
	
	while (regex_search(textIterator, result.end(), match, rgx)){
		int count = match.size();
		//for (int i=0; i<count; i++){		
			interfaces.push_back(match[1]);
		//}
	   textIterator = match[0].second;
	}
	return interfaces;
}


std::vector<struct Network> Iwapi::scanNetworks(std::string  devName){
	const std::string result = Utils::exec("iw " + devName + " scan");
	std::vector<struct Network> networks;	
	
	std::smatch match;
	std::regex rgx("BSS\\s((?:[0-9|a-f]{2}:){5}(?:[0-9|a-f]{2}))[\\S\\s]+?(?:signal:)\\s(-[0-9]+.[0-9]+)[\\S\\s]+?(?:SSID:)\\s(.*)[\\S\\s]+?(?=RSN:|\\bBSS)(?:RSN:[\\S\\s]+?(?:Group cipher:\\s(.+)))?");
	
	std::string::const_iterator textIterator = result.begin();
	
	while (regex_search(textIterator, result.end(), match, rgx)){
		int count = match.size();
		for (int i=1; i<count; i++){
			/*output from match:
				1: bss id
				2: signal strength in dBm
				3: ssid
				4: encryption method
				*/
			std::cout<<match[i]<<" "<<std::endl;
		}
			
		//find existed network
		struct Network* net;
		bool found=false;
		for  (int i=0;i<networks.size();i++){
			net = &networks[i];
			if (net->ssid.compare(match[3])==0){
				found = true;
				break;
			}
		}
		
		if (!found){
			//std::cout<<"Network "<<match[3]<<" not found"<<std::endl;
			net = new Network();
			net->ssid = match[3];							
		}

		//std::cout<<"adding bss: "<<match[1]<<std::endl;
		struct BSS* bss;
		bss = new BSS();
		
		bss->id = match[1];
		std::string signal = match[2];
		bss->signal = atof(signal.c_str());
		bss->encryption = match[4];
		net->aps.push_back(*bss);
		//std::cout<<"length now: "<<net->aps.size()<<std::endl;
		//std::cout<<"test first element: "<<net->aps[0].id<<std::endl;
		
		if (!found){
			networks.push_back(*net);
		}
	
	   textIterator = match[3].second;
	}
	return networks;
}

