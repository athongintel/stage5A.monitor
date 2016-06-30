/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__

	#include "SVCAuthenticator.h"

	class SVC{
	
		bool isAuthenticated;
		SVCHost* host;
		SVCAuthenticator* authenticator;
	
		public:
			SVC(SVCHost* remotehost, SVCAuthenticator* authenticator);
			bool establishConnecion();
			int sendData(unsigned char* data, size_t dalalen, SVCPriority priority, bool sureProtocol);
			bool setDataReceiveHandler(SVCDataReceiveHandler handler);
	};


#endif
