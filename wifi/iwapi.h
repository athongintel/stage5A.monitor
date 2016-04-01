#ifndef _IWAPI_
#define _IWAPI_

#include <regex>
#include <iostream>
#include <vector>
#include "utils.h"

class Iwapi{
	
	public:
		Iwapi();	
		std::vector<std::string> getWlanInterfaceNames();

};

#endif