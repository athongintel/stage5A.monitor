/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__


	#include "authenticator/SVCAuthenticator.h"
	#include "SVC-header.h"
	#include "SVCHost.h"
	#include "SVCApp.h"
	#include <functional>
	#include <string>
	#include <cstring>	
	#include <mutex>
	#include <vector>
	#include <csignal>
	#include <sys/un.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <sys/time.h>
	#include <sys/types.h>
	#include <sys/syscall.h>
	
	#define sigev_notify_thread_id _sigev_un._tid
	#define gettid() syscall(SYS_gettid)
		
	typedef void (*SVCDataReceiveHandler)(const uint8_t* data, size_t datalen, void* args);
	struct SVCCommandReceiveHandler{
		SVCDataReceiveHandler handler;
		int repeat;
		enum SVCCommand command;
		void* params;
	};
	

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
		
		vector<SVCCommandReceiveHandler*> commandHandlers;
		SVCDataReceiveHandler dataHandler;
		mutex handlerMutex;
	
		static void* processPacket(void* args);			
		//wait for a specific command from the lower layer, return false if timeout or error, params will be filled with received parameters
		bool _waitCommand(enum SVCCommand command, vector<SVCCommandParam*>* params, int timeout);
		
		public:		
			~SVC();
			SVC(SVCApp* localApp, SVCAuthenticator* authenticator);
			bool establishConnection(SVCHost* remoteHost);
			int sendData(const uint8_t* data, size_t dalalen, SVCPriority priority, bool tcp);
			bool setDataReceiveHandler(SVCDataReceiveHandler handler);
	};
				
	
#endif
