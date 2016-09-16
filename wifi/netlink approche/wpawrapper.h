#include <stdio.h>
#include <string>
#include <cstring>

#include "wpa_ctrl.h"
#include "os.h"
#include "network.h"

using namespace std;

class WpaControlWrapper{

	struct wpa_ctrl* controller;
	
	
	public:
		~WpaControlWrapper();
		WpaControlWrapper(WifiInterface* interface);

		
		//methods
		int request(string command, string& response);

};
