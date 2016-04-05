#ifndef _IWAPI_
#define _IWAPI_

#include <regex>
#include <iostream>
#include <vector>
#include "utils.h"

struct BSS{
	std::string id;
	double signal;
	std::string encryption;
};

struct Network{
	std::string ssid;
	std::vector<struct BSS> aps;
	
};

class Iwapi{
	
	std::string version;
	
	public:
		Iwapi() throw ();	
		std::vector<std::string> getWlanInterfaceNames();
		std::vector<struct Network> scanNetworks(std::string  devName);

};

#endif