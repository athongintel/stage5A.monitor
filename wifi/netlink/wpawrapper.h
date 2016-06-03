#include <stdio.h>
#include <string>

#include "wpa_ctrl.h"
#include "network.h"

using namespace std;

class WpaControlWrapper{

	struct wpa_ctrl* controller;
	~WpaControlWrapper();
	
	public:
		WpaControlWrapper(WifiInterface* interface);

		
		//methods
		int request(string command, string& response);

};
