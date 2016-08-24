/*		SVC-header contents common functionalities used by both SVC and SVC-daemon*/

#ifndef __SVC_HEADEAR__
#define __SVC_HEADER__

	#include "MutexedQueue.h"

	#include <unistd.h> //for 'unlink'
	#include <cstring> 	//for 'memcpy'
	#include <vector>
	
	#include <sys/time.h>
	#include <sys/socket.h>
	
	/*	COMPTIBILITY	*/
	#define sigev_notify_thread_id _sigev_un._tid
		
	using namespace std;

	/*	HTP definitions */
	#define AF_HTP								AF_PHONET
	#define PF_HTP								PF_PHONET
	
	/*	SVC ERROR DESCRIPTION	*/
	#define SVC_ERROR_NAME_EXISTED				"Application is already running"
	#define SVC_ERROR_UNEXPECTED_RESPONSE		"Unexpected response"
	#define SVC_ERROR_NOT_ESTABLISHED			"Connection not established"
	#define SVC_ERROR_REQUEST_TIMEDOUT			"Request timed out"
	#define SVC_ERROR_AUTHENTICATION_FAILED		"Authentication failed"
	#define SVC_ERROR_CRITICAL					"Critical error"
	#define SVC_ERROR_BINDING					"Error binding socket"
	#define SVC_ERROR_NOTIFICATOR_DUPLICATED	"Notificator duplicated"	
	

	/*	SVC CONSTANTS	*/
	#define SVC_ACQUIRED_SIGNAL					SIGUSR1
	#define SVC_TIMEOUT_SIGNAL					SIGUSR2
	#define SVC_SHARED_MUTEX_SIGNAL				SVC_ACQUIRED_SIGNAL

	#define SVC_DEFAULT_TIMEOUT 				2000
	#define SVC_SHORT_TIMEOUT					100
	#define SVC_DEFAULT_BUFSIZ 					65536
	#define	SVC_DAEPORT							1221
	
	
	/*	SVC CONSTANTS' LENGTHS	*/
	#define APPID_LENGTH						4
	#define	SESSIONID_LENGTH					4
	#define ENDPOINTID_LENGTH					8

	/*	SVC INFO BIT	*/

	#define SVC_COMMAND_FRAME  					0x80
	#define SVC_DAEMON_RESPONSE					0x40
	#define SVC_ENCRYPTED						0x08
	#define SVC_USING_TCP						0x04
	
	#define SVC_URGENT_PRIORITY 				0x03
	#define	SVC_HIGH_PRIORITY					0x02
	#define SVC_NORMAL_PRIORITY					0x01
	#define SVC_LOW_PRIORITY					0x00

	static uint32_t SVC_DEFAULT_SESSIONID 	= 	0;
	//static uint64_t SVC_DEFAULT_ENDPOINTID 	=	0;
	
	static string SVC_DAEMON_PATH = 			"/tmp/svc-daemon";
	static string SVC_CLIENT_PATH_PREFIX = 		"/tmp/svc-client-";
	
	
	/*	ABI, DO NOT MODIFY UNLESS YOU KNOW EXACTLY WHAT	YOU DO	*/
	enum SVCCommand : uint8_t{
		SVC_CMD_CHECK_ALIVE,
		SVC_CMD_REGISTER_APP,
		SVC_CMD_CONNECT_STEP1,
		SVC_CMD_CONNECT_STEP2,
		SVC_CMD_CONNECT_STEP3,
		SVC_CMD_CONNECT_STEP4,
		_SVC_CMD_COUNT
	};
	/*	END OF ABI	*/

	typedef void (*SVCDataReceiveHandler)(const uint8_t* data, size_t datalen, void* args);
	
	struct SVCDataReceiveNotificator{
		SVCDataReceiveHandler handler;
		void* args;
		pthread_t thread;			
	};
	
	
	class Message{
		public:
			uint8_t* data;
			size_t len;
			
			Message(const uint8_t* data, size_t len){
				this->data = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
				this->len = len;
				memcpy(this->data, data, this->len);
			}
			
			~Message(){
				delete data;
				printf("message destructed\n");
			}		
	};

	/*	CLASSES DEFINITIONS	*/
	class SVCCommandParam{
	
		bool copy;
		public:
			uint16_t length;
			uint8_t* data;
						
			SVCCommandParam(){
				this->copy = false;
			}
					
			SVCCommandParam(uint16_t length, const uint8_t* data){
				this->length = length;
				this->data = (uint8_t*)malloc(length);
				memcpy(this->data, data, length);
				this->copy = true;
			}
		
			~SVCCommandParam(){	
				if (this->copy){
					delete this->data;
					printf("param destructed\n");
				}
			}
	};

	/*	just make sure that there will be no wait for 2 same cmd on a single list	*/	
	class SignalNotificator{
		private:			
			struct SVCDataReceiveNotificator* notificationArray[_SVC_CMD_COUNT];
			shared_mutex notificationArrayMutex;
			
			static void waitCommandHandler(const uint8_t* buffer, size_t datalen, void* args);
			
		public:
			SignalNotificator(){
				/*	need to init this array to NULL, otherwise left memory will cause addNotificator to throw exception	*/
				for (uint8_t cmd = 0; cmd<_SVC_CMD_COUNT; cmd++){
					this->notificationArray[cmd] = NULL;
				}			
			}
			~SignalNotificator(){}
			
			SVCDataReceiveNotificator* getNotificator(enum SVCCommand cmd);			
			void removeNotificator(enum SVCCommand cmd);
			void addNotificator(enum SVCCommand cmd, SVCDataReceiveNotificator* notificator);			
			bool waitCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params, int timeout);
	};
			
	/*	END OF CLASSES DEFINITIONS	*/	

	//--	UTILS FUNCTIONS		--//
	
	//--	return if the command must be encrypted
	bool isEncryptedCommand(enum SVCCommand command);
	
	//--	clear all params in the vector and call their destructors
	void clearParams(vector<SVCCommandParam*>* params);
	
	//--	extract parameters from a buffer without header
	void extractParams(const uint8_t* buffer, vector<SVCCommandParam*>* params);
	
	//--	print current buffer in hex bytes
	void printBuffer(const uint8_t* buffer, size_t len);
	
	//--	timeoutSignal and waitingSignal must be differrent, otherwise the behavior is undefined
	bool waitSignal(int waitingSignal, int timeoutSignal, int timeout);	
	
	//void prepareCommand(uint8_t* buffer, size_t* len, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);
	//void sendCommand(int socket, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);	
	//ssize_t _sendPacket(int socket, const uint8_t* buffer, size_t len);						
	
	
#endif
