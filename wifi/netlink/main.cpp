#include "network.h"

#include <iostream>

using namespace std;

int main(int argc, char** argv){
	WifiController* netController = WifiController::getInstance();
	cout<<"Trying to call wiphy dump from main";
	netController->getNetworkInterfaces();
	return 0;
}
