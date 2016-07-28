#ifndef __SVC_HEADEAR__
#define __SVC_HEADER__

#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
using namespace std;

/*	SVC ERROR DESCRIPTION	*/
#define SVC_ERROR_NAME_EXISTED			"Application is already existed"
#define SVC_ERROR_UNEXPECTED_RESPONSE	"Unexpected response"
#define SVC_ERROR_NOT_ESTABLISHED		"Connection not established"
#define SVC_ERROR_REQUEST_TIMEDOUT		"Request timed out"
#define SVC_ERROR_AUTHENTICATION_FAILED	"Authentication failed"
#define SVC_ERROR_CRITICAL				"Critical error"
#define SVC_ERROR_BINDING				"Error binding socket"


/*	SVC CONSTANTS	*/
#define SVC_DEFAULT_TIMEOUT 2000
#define SVC_DEFAULT_BUFSIZ 65536
#define SVC_USING_TCP 0x04

static const uint8_t SVC_DATA_FRAME = 0;
static const uint8_t SVC_COMMAND_FRAME = 1;

static string SVC_DAEMON_PATH = "/tmp/svc-daemon";
static string SVC_CLIENT_PATH_PREFIX = "/tmp/svc-client-";

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

class SVCCommandParam{
	public:
		uint16_t length;
		uint8_t* param;
		SVCCommandParam(uint16_t length, const uint8_t* param){
			this->length = length;
			this->param = (uint8_t*)malloc(length);
			memcpy(this->param, param, length);
		}
		
		~SVCCommandParam(){
			delete param;
		}				
};

ssize_t _sendCommand(int socket, enum SVCCommand command, const vector<SVCCommandParam*>* params);


void printBuffer(const uint8_t* buffer, size_t len);
#endif
