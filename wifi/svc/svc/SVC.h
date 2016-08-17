/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__


	#include "authenticator/SVCAuthenticator.h"
	#include "SVC-header.h"
	#include "SVCHost.h"
	#include "SVCApp.h"
	#include <string>
	#include <cstring>
	#include <sys/un.h>
	#include <sys/socket.h>
	#include <sys/types.h>

	static SignalNotificator signalNotificator;
			
	class SVC{				
				
		bool isAuthenticated;
		SVCHost* host;
		SVCApp* localApp;
		SVCAuthenticator* authenticator;

		string svcClientPath;
		struct sockaddr_un daemonSocketAddress;
		struct sockaddr_un svcSocketAddress;
		int svcSocket;
		int svcDaemonSocket;
		
		uint32_t sessionID;
		
		pthread_t readingThread;
		volatile bool working;
		
		
		SVCDataReceiveHandler dataHandler;
	
		void destruct();
		static void* processPacket(void* args);			
		//wait for a specific command from the lower layer, return false if timeout or error, params will be filled with received parameters
				
		public:	
			~SVC();
			SVC(SVCApp* localApp, SVCAuthenticator* authenticator);
			bool establishConnection(SVCHost* remoteHost);
			int sendData(const uint8_t* data, size_t dalalen, uint8_t priority, bool tcp);
			bool setDataReceiveHandler(SVCDataReceiveHandler handler);
	};
				
	
#endif
