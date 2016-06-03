#include "wpawrapper.h"

WpaControlWrapper::WpaControlWrapper(WifiInterface* interface){
	string ctrlPath = string("/var/run/wpa_supplicant/") + interface->getName();
	cout<<"init wpa control on: "<<ctrlPath.c_str()<<endl;
	
	this->controller = NULL;	
	while (this->controller == NULL){
		this->controller = wpa_ctrl_open(ctrlPath.c_str());
	}
	cout<<"init wpa control success: "<<this->controller<<endl;
}


WpaControlWrapper::~WpaControlWrapper(){
	wpa_ctrl_close(this->controller);
}

int WpaControlWrapper::request(string command, string& response){
	char responseBuffer[512] = "";
	size_t responseLength;
	
	cout<<"requesting: "<<command.c_str()<<endl;
	
	int result = wpa_ctrl_request(this->controller, command.c_str(), strlen(command.c_str()), responseBuffer, &responseLength, NULL);
	if (result == 0){
		cout<<"response with: "<<responseBuffer<<endl;
		response = string(responseBuffer);
	}
	else{
		cout<<"error: "<<result<<endl;
	}
	return result;
}
