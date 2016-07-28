#include "SVCHostIP.h"

using namespace std;

SVCHostIP::SVCHostIP(string ipAddress){
	this->ipAddress = ipAddress;
}

string SVCHostIP::getHostAddress(){
	return this->ipAddress;
}
