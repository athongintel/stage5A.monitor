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
	std::regex rgx("BSS\\s((?:[0-9|a-f]{2}:){5}(?:[0-9|a-f]{2}))[\\S\\s]+?(?:freq:)\\s([0-9]+)[\\S\\s]+?(?:signal:)\\s(-[0-9]+.[0-9]+)[\\S\\s]+?(?:SSID:)\\s(.*)[\\S\\s]+?(?=RSN:|\\bBSS)(?:RSN:[\\S\\s]+?(?:Group cipher:\\s(.+)))?");
	
	std::string::const_iterator textIterator = result.begin();
	
	while (regex_search(textIterator, result.end(), match, rgx)){
		int count = match.size();
		/*for (int i=1; i<count; i++){
			//output from match:
				//1: bss id
				//2: freq
				//3: signal strength in dBm
				//4: ssid
				//5: encryption method				
				
			std::cout<<i<<" "<<match[i]<<" "<<std::endl;
		}*/
			
		//find existed network
		struct Network* net;
		bool found=false;
		for  (int i=0;i<networks.size();i++){
			net = &networks[i];
			if (net->ssid.compare(match[SSID_REGEX])==0){
				found = true;
				break;
			}
		}
		
		if (!found){			
			net = new Network();
			net->ssid = match[SSID_REGEX];							
		}

		struct BSS* bss;
		bss = new BSS();
		
		bss->id = match[BSSID_REGEX];
		std::string signal = match[SIGNAL_REGEX];
		std::string freq = match[FREQ_REGEX];
		bss->signal = atof(signal.c_str());
		bss->encryption = match[CIPHER_REGEX];
		bss->freq = atof(freq.c_str());
		net->aps.push_back(*bss);
		
		if (!found){
			networks.push_back(*net);
		}
		
		//move itertor forward
	   textIterator = match[0].second;
	}
	return networks;
}

