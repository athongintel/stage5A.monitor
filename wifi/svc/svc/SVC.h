/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__


	#include "authenticator/SVCAuthenticator.h"
	#include "SVC-header.h"
	#include "SVCHost.h"
	#include "SVCApp.h"

	#include <sys/un.h>
	#include <sys/types.h>

	static SignalNotificator signalNotificator;
	static hash<string> hasher;
	
	class SVCEndPoint(){
		
		private:
			Queue<Message*>* dataQueue;
		
		public:
			SVCEndPoint(){};
			~SVCEndPoint();
				
			int sendData(const uint8_t* data, size_t dalalen, uint8_t priority, bool tcp);
			int readData(uint8_t* data, size_t& len);			
	}
	
	class SVC{				
				
		SVCApp* localApp;
		SVCAuthenticator* authenticator;

		string svcClientPath;
		struct sockaddr_un daemonSocketAddress;		/* 	read from	*/
		struct sockaddr_un svcSocketAddress;		/*	write to	*/
		int svcSocket;
		int svcDaemonSocket;

		pthread_t readingThread;
		volatile bool working;
		
		uint32_t sessionID;
		
		shared_mutex* connectionRequestMutex;
		Queue<Message*>* connectionRequest;
			
	
		void destruct();
		static void* processPacket(void* args);	
				
		public:	
			~SVC();
			SVC(SVCApp* localApp, SVCAuthenticator* authenticator);
					
			SVCEndPoint* establishConnection(SVCHost* remoteHost);
			SVCEndPoint* listenConnection();					
	};		
	
#endif
