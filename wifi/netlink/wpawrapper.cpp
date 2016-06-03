#include "wpawrapper.h"

WpaControlWrapper::WpaControlWrapper(WifiInterface* interface){
	string ctrlPath = string("/var/run/wpa_supplicant/") + interface->getName();
	this->controller = wpa_ctrl_open(ctrlPath.c_str());
}


WpaControlWrapper::~WpaControlWrapper(){
	if (this->controller!=NULL){
		wpa_ctrl_close(this->controller);
	}
}

int WpaControlWrapper::request(string command, string& response){
	char responseBuffer[2048];
	size_t responseLength;
	
	int result = wpa_ctrl_request(this->controller, command.c_str(), command.size(), responseBuffer, &responseLength, NULL);
	response = string(responseBuffer);
	return result;
}
