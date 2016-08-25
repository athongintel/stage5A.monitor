/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__


	#include "authenticator/SVCAuthenticator.h"
	#include "SVC-header.h"
	#include "host/SVCHost.h"
	#include "SVCApp.h"

	#include <unordered_map>
	#include <sys/un.h>
	#include <sys/types.h>

	using namespace std;
	
	static hash<string> hasher;
	
	//--	FORWARD DECLARATION		--//
	class SVC;
	
	class SVCEndPoint{	
		
		friend class SVC;
			
		private:			
			MutexedQueue<Message*>* dataQueue;
			
			SVC* svc;
			uint64_t endPointID;
			SignalNotificator* signalNotificator;
			
			SVCEndPoint(SVC* svc, SignalNotificator* sigNot);
			
			void sendCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params);
		
		public:
			~SVCEndPoint();
			int sendData(const uint8_t* data, size_t dalalen, uint8_t priority, bool tcp);
			int readData(uint8_t* data, size_t* len);
	};
	
	class SVC{				
		
		friend class SVCEndPoint;
		
		private:				
			
			SVCApp* localApp;
			SVCAuthenticator* authenticator;
				
			shared_mutex* endPointsMutex;
			vector<SVCEndPoint*> endPoints;

			string svcClientPath;
			struct sockaddr_un daemonSocketAddress;		//--	write to
			struct sockaddr_un svcSocketAddress;		//--	read from
			int svcSocket;
			int svcDaemonSocket;

			pthread_t readingThread;
			volatile bool working;		
		
			Queue<Message*>* connectionRequest;
			uint32_t appID;

			void destruct();
			SVCEndPoint* getEndPointByID(uint64_t endPointID);
			
			static void* processPacket(void* args);
			
		public:				
	
			~SVC();
			void stopWorking();
			SVC(SVCApp* localApp, SVCAuthenticator* authenticator);						
			SVCEndPoint* establishConnection(SVCHost* remoteHost);
			SVCEndPoint* listenConnection();					
	};		
	
#endif
