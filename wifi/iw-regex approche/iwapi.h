#ifndef _IWAPI_
#define _IWAPI_

#include <regex>
#include <iostream>
#include <vector>
#include "utils.h"


#define BSSID_REGEX 1
#define FREQ_REGEX 2
#define SIGNAL_REGEX 3
#define SSID_REGEX 4
#define CIPHER_REGEX 5

struct BSS{
	std::string id;
	double signal;
	std::string encryption;
	double freq;
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