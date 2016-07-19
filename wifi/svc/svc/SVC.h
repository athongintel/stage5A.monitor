/* Secure Virtual Connector (SVC) protocol header */

#ifndef __SVC__
#define __SVC__

	#include "SVCAuthenticator.h"
	#include <functional>
	#include <string>
	
	
	/*	SVC ERROR DESCRIPTION	*/
	#define SVC_ERROR_NAME_EXISTED			"Application name existed"
	#define SVC_ERROR_UNEXPECTED_RESPONSE	"Unexpected response"
	#define SVC_ERROR_NOT_ESTABLISHED		"Connection not established"
	
	
	/*	SVC CONSTANTS	*/
	#define SVC_DEFAULT_TIMEOUT 1000
	#define SVC_DEFAULT_BUFSIZ 65536	
	const uint8_t SVC_DATA_FRAME = 0;
	const uint8_t SVC_COMMAND_FRAME = 1;
	
	string SVC_DAEMON_PATH = "/tmp/svc-daemon";
	string SVC_CLIENT_PATH_PREFIX = "/tmp/svc-client-";
	
	/*	DONT CHANGE THIS, THIS IS ABI!	*/	
	enum SVCCommand : uint8_t{
		SVC_CMD_REGISTER_APP,
		SVC_CMD_NEGOTIATION_STEP1,
		SVC_CMD_NEGOTIATION_STEP2,
		SVC_CMD_NEGOTIATION_STEP3,
		SVC_CMD_NEGOTIATION_STEP4								
	};
	
	enum SVCPriority: uint8_t{
		SVC_URGENT_PRIORITY,
		SVC_HIGH_PRIORITY,
		SVC_NORMAL_PRIORITY,
		SVC_LOW_PRIORITY
	};
	
	/*	END OF ABI	*/
	
	typedef void (*SVCDataReceiveHandler)(const uint8_t* data, size_t datalen);
	
	
	class SVCCommandParam{
		int length;
		uint8_t* param;
		public:
			
			SVCCommandParam(int length, uint8_t* param){
				this->length = length;
				this->param = (uint8_t*)malloc(length);
				memcpy(this->param, param, length);
			}
			
			~SVCCommandParam(){
				delete param;
			}
			
			int getLength(){
				return this->length;
			}
			
			uint8_t* getParam(){
				return param;
			}			
	};		

	class SVC{				
	
		string svcClientPath;
	
		bool isAuthenticated;
		SVCHost* host;
		SVCApp* localApp;
		SVCAuthenticator* authenticator;
		SVCDataReceiveHandler dataHandler;

		struct sockaddr_un daemonSocketAddress;
		struct sockaddr_un svcSocketAddress;
		int svcSocket;
		
		//private methods
		ssize_t _sendCommand(enum SVCCommand command, vector<SVCCommandParam*> params);
		
		//	WAIT FOR A SPECIFIC COMMAND FROM DAEMON, AFTER TIMEOUT RETURN WITH FALSE ELSE RETURN WITH TRUE
		bool _waitCommand(enum SVCCommand command, vector<SVCCommandParam*> &params, int timeout);
	
		public:		
			
			SVC(SVCApp* localApp, SVCAuthenticator* authenticator);
			bool establishConnecion(SVCHost* remoteHost);
			int sendData(const uint8_t* data, size_t dalalen, SVCPriority priority, bool tcp);
			bool setDataReceiveHandler(SVCDataReceiveHandler handler);
	};


#endif
