#ifndef __SCV_HOSTIP__

#define __SVC_HOSTIP__

#include "SVCHost.h"
#include <string>

class SVCHostIP : public SVCHost {

	std::string ipAddress;

	public:
		SVCHostIP(std::string ipAddress);
		std::string getHostAddress();
};

#endif
