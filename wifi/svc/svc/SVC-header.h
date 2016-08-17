/*		SVC-header contents common functionalities used by both SVC and SVC-daemon*/

#ifndef __SVC_HEADEAR__
#define __SVC_HEADER__

	#include <vector>
	#include <string>
	#include <cstring>
	#include <mutex>
	#include <csignal>
	#include <vector>
	#include <sys/time.h>
	#include <sys/socket.h>
	#include <sys/syscall.h>
	#include <functional>
	#include <unistd.h>
	
	/*	COMPTIBILITY	*/
	#define sigev_notify_thread_id _sigev_un._tid
	#define gettid() syscall(SYS_gettid)
	
	
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


	/*	SVC CONSTANTS	*/
	#define SVC_ACQUIRED_SIGNAL					SIGUSR1
	#define SVC_TIMEOUT_SIGNAL					SIGUSR2
	#define SVC_DEFAULT_TIMEOUT 				2000
	#define SVC_DEFAULT_BUFSIZ 					65536

	/*	SVC INFO BIT	*/
	#define SVC_COMMAND_FRAME  					0x80
	#define SVC_DATA_FRAME  					0x00
	#define SVC_ENCRYPTED						0x08
	#define SVC_USING_TCP						0x04
	
	#define SVC_URGENT_PRIORITY 				0x03
	#define	SVC_HIGH_PRIORITY					0x02
	#define SVC_NORMAL_PRIORITY					0x01
	#define SVC_LOW_PRIORITY					0x00

	static uint32_t SVC_DEFAULT_SESSIONID = 	0x00000000;
	static string SVC_DAEMON_PATH = 			"/tmp/svc-daemon";
	static string SVC_CLIENT_PATH_PREFIX = 		"/tmp/svc-client-";
	
	
	/*	SVC COMMAND	*/
	enum SVCCommand : uint8_t{
		SVC_CMD_CHECK_ALIVE,
		SVC_CMD_REGISTER_APP,
		SVC_CMD_CONNECT_STEP1,
		SVC_CMD_CONNECT_STEP2,
		SVC_CMD_CONNECT_STEP3,
		SVC_CMD_CONNECT_STEP4
	};
	/*	END OF ABI	*/


	/*	CLASSES DEFINITIONS	*/
	class SVCCommandParam{
		public:
			uint16_t length;
			uint8_t* param;
			bool copy;
			
			SVCCommandParam(){}
					
			SVCCommandParam(uint16_t length, const uint8_t* param){
				this->length = length;
				this->param = (uint8_t*)malloc(length);
				memcpy(this->param, param, length);
				this->copy = true;
			}
		
			~SVCCommandParam(){	
				printf("param destructed\n");		
				if (this->copy) delete param;
			}
	};

	class MessageQueue{
	
		int head;
		int tail;
		int size;
		int maxsize;
		uint8_t** array;	
		size_t* arrayLen;
		

		public:	
			static const int MESSAGE_QUEUE_DEFAULT_SIZE = 4096;
			mutex messageMutex;
	
			MessageQueue(int maxsize=MessageQueue::MESSAGE_QUEUE_DEFAULT_SIZE);
			~MessageQueue();
		
			int enqueue(const uint8_t* message, size_t len);
			int peak(uint8_t** message, size_t* len);
			int dequeue();
			bool notEmpty();
		
	};
	/*	END OF CLASSES DEFINITIONS	*/
	
	typedef void (*SVCDataReceiveHandler)(const uint8_t* data, size_t datalen, void* args);
	
	struct SVCDataReceiveNotificator{
		SVCDataReceiveHandler handler;
		void* args;		
		enum SVCCommand command;
		pthread_t thread;			
	};
	
	
	/*	GLOBAL VARIABLES	*/
	static vector<struct SVCDataReceiveNotificator*> notificationList;
	static mutex notificationListMutex;
	/*	END OF GLOBAL VARIABLES	*/
	
	
	bool isEncryptedCommand(enum SVCCommand command);
	
	void waitCommandHandler(const uint8_t* buffer, size_t datalen, void* args);	
	bool waitCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params, int timeout);
	void prepareCommand(uint8_t* buffer, size_t* len, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);
	void sendCommand(int socket, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);
	
	ssize_t _sendPacket(int socket, const uint8_t* buffer, size_t len);			
	
	
	/*	UTILS FUNCTIONS	*/
	/*	print current buffer in hex bytes	*/
	void printBuffer(const uint8_t* buffer, size_t len);
	/*	timeoutSignal and waitingSignal must be differrent, or the behavior is undefined*/
	bool waitSignal(int waitingSignal, int timeoutSignal, int timeout);	
	
#endif
