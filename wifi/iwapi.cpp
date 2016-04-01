#include "iwapi.h"

Iwapi::Iwapi(){
	//check for iw version
	const std::string result = Utils::exec("iw --version");
	std::cout<<result;
	std::smatch match;
	std::regex rgx("iw version ([0-9]+[.][0-9])+");

    if (std::regex_search(result.begin(), result.end(), match, rgx)){
		for (unsigned i=0; i<match.size(); ++i)
    std::cout << "match #" << i << ": " << match[i] << std::endl;
	}
	
}

std::vector<std::string> Iwapi::getWlanInterfaceNames(){
	
}