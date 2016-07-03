/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__

	#include "SVCAuthenticator.h"

	class SVC{
	
		bool isAuthenticated;
		SVCHost* host;
		SVCAuthenticator* authenticator;
	
		public:
			SVC(SVCAuthenticator* authenticator);
			bool establishConnecion(SVCApp* localApp, SVCHost* remoteHost);
			int sendData(unsigned char* data, size_t dalalen, SVCPriority priority, bool sureProtocol);
			bool setDataReceiveHandler(SVCDataReceiveHandler handler);
	};


#endif
