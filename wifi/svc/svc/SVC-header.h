#ifndef __SVC_HEADEAR__
#define __SVC_HEADER__

	#include <vector>
	#include <string>
	#include <cstring>
	#include <sys/socket.h>
	using namespace std;

	/*	HTP definitions */
	#define AF_HTP		AF_PHONET
	#define PF_HTP		PF_PHONET

	/*	SVC ERROR DESCRIPTION	*/
	#define SVC_ERROR_NAME_EXISTED			"Application is already running"
	#define SVC_ERROR_UNEXPECTED_RESPONSE	"Unexpected response"
	#define SVC_ERROR_NOT_ESTABLISHED		"Connection not established"
	#define SVC_ERROR_REQUEST_TIMEDOUT		"Request timed out"
	#define SVC_ERROR_AUTHENTICATION_FAILED	"Authentication failed"
	#define SVC_ERROR_CRITICAL				"Critical error"
	#define SVC_ERROR_BINDING				"Error binding socket"


	/*	SVC CONSTANTS	*/
	#define SVC_DEFAULT_TIMEOUT 			2000
	#define SVC_DEFAULT_BUFSIZ 				65536

	/*	SVC INFO BIT	*/
	#define SVC_COMMAND_FRAME  				0x80
		
	#define SVC_ENCRYPTED					0x08	
	#define SVC_USING_TCP					0x04
	#define SVC_URGENT_PRIORITY 			0x03
	#define	SVC_HIGH_PRIORITY				0x02
	#define SVC_NORMAL_PRIORITY				0x01
	#define SVC_LOW_PRIORITY				0x00


	static uint32_t SVC_DEFAULT_SESSIONID = 0x00000000;
	static string SVC_DAEMON_PATH = "/tmp/svc-daemon";
	static string SVC_CLIENT_PATH_PREFIX = "/tmp/svc-client-";

	/*	DONT CHANGE THIS, THIS IS ABI!	*/	
	enum SVCCommand : uint8_t{
		SVC_CMD_CHECK_ALIVE,
		SVC_CMD_REGISTER_APP,
		SVC_CMD_CONNECT_STEP1,
		SVC_CMD_CONNECT_STEP2,
		SVC_CMD_CONNECT_STEP3,
		SVC_CMD_CONNECT_STEP4
	};

	enum SVCPriority: uint8_t{
		SVC_URGENT_PRIORITY,
		SVC_HIGH_PRIORITY,
		SVC_NORMAL_PRIORITY,
		SVC_LOW_PRIORITY
	};

	/*	END OF ABI	*/

	class SVCCommandParam{
		public:
			uint16_t length;
			uint8_t* param;
			bool copy;
		
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
			const int MESSAGE_QUEUE_DEFAULT_SIZE = 4096;
			mutex messageMutex;
	
			MessageQueue(int maxsize=MessageQueue::MESSAGE_QUEUE_DEFAULT_SIZE);
			~MessageQueue();
		
			int enqueue(const uint8_t* message, const size_t* len);
			int dequeue(uint8_t* returnedMessage, size_t* len);
			bool notEmpty();
		
	};

	ssize_t _sendCommand(int socket, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params);
	void printBuffer(const uint8_t* buffer, size_t len);
	bool isEncryptedCommand(enum SVCCommand command);
	
#endif
