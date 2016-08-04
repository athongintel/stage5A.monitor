#include "SVC-header.h"

#include <string>
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdlib>

using namespace std;


struct SVCCmd{
	enum SVCCommand cmdID;
	uint8_t argc;	
};

class DaemonService{
	
	public:	
		string svcClientPath;
		sockaddr_un sockAddress;
		int sock;
		uint32_t appID;	
		
		DaemonService(uint32_t appID){
			this->appID = appID;			
			//create socket address
			this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
			memset(&this->sockAddress, 0, sizeof(this->sockAddress));
			this->sockAddress.sun_family = AF_LOCAL;
			memcpy(this->sockAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());			
			//create new diagram socket and bind
			this->sock = socket(AF_LOCAL, SOCK_DGRAM, 0);
			connect(this->sock, (struct sockaddr*) &this->sockAddress, sizeof(this->sockAddress));
		}
};

unordered_map<uint32_t, DaemonService*> appTable;

struct sockaddr_un daemonAddress;
int daemonSocket;
int htpSocket;
pthread_t readingThread;
pthread_attr_t readingThreadAttr;

string errorString;
volatile bool working;

//receive buffer and msghdr
uint8_t* receiveBuffer;
struct sockaddr messageAddress;
socklen_t messageAddressLen;

//check app existed, return sessionID
uint32_t appExisted(uint32_t appID){
	for (auto& service : appTable){
		if (service.second->appID == appID){
			return service.first;
		}
	}
	return 0;
}

void processCommand(const uint8_t* buffer, size_t len){
	
	//extract params
	uint32_t sessionID = *((uint32_t*)receiveBuffer);			
	//pass 4 byte session and 1 byte frame type
	const uint8_t* pointer = receiveBuffer + 5;
	
	struct SVCCmd* cmd = (struct SVCCmd*)(pointer);
	vector<SVCCommandParam*> argv;

	//pass 2 byte of command type and argc
	pointer+= 2;	
	
	for (int i=0; i<cmd->argc; i++){
		uint16_t length = *(pointer);
		printf("param %d length: %04x, param: ",i, length);
		printBuffer(pointer+2, length);
		argv.push_back(new SVCCommandParam(length, pointer+2));
		//add 2 byte of length and the arg's length
		pointer+= 2+length;
	}

	vector<SVCCommandParam*> params;
	switch(cmd->cmdID){
		case SVC_CMD_REGISTER_APP:						
			uint32_t appID;
			ssize_t sendResult;
			bool newAppAllowed;
			
			//a.1 check if app existed
			appID = *((uint32_t*)(argv[0]->param));
			//printf("appID: %08x\n",appID);
			
			newAppAllowed = false;
			sessionID = appExisted(appID);
			if (sessionID!=0){
				//check if alive
				sendResult = _sendCommand(appTable[sessionID]->sock, sessionID, SVC_CMD_CHECK_ALIVE, &params);				
				newAppAllowed = (sendResult ==-1) && (errno == ECONNREFUSED || errno == ENOTCONN);	
			}
			else{
				newAppAllowed = true;
			}
			
			if (newAppAllowed){
				//a.2 add new record to appTable
				//create sessionID = hash(appID & rand)							
				hash<string> hasher;
				sessionID = (uint32_t)hasher(to_string(appID) + to_string(rand()));
				appTable[sessionID] = new DaemonService(appID);
				//a.3 repsonse sessionID to SVC app interface							
				params.push_back(new SVCCommandParam(4, (uint8_t*)(&sessionID)));
				params.push_back(new SVCCommandParam(4, (uint8_t*)(argv[1]->param)));				
				_sendCommand(appTable[sessionID]->sock, 0, SVC_CMD_REGISTER_APP, &params);
			}
			break;

		case SVC_CMD_CONNECT_STEP1:
			printf("got connect step 1 command\n");
			
			break;
			
		default:
			break;
	}
}

void processData(uint8_t* buffer, size_t len){
}

void* readingLoop(void* args){
	//read message from socket in blocking-mode
	while (working){
		size_t len = recvfrom(daemonSocket, receiveBuffer, SVC_DEFAULT_BUFSIZ, 0, &messageAddress, &messageAddressLen);
		printf("read a message of %d\n", len);
		printBuffer(receiveBuffer, len);
		//process message. message is stored in receiveBuffer
		if (len>0){
			//check if sessionID is registered
			uint32_t sessionID = *((uint32_t*)receiveBuffer);
			//printf("sessionID of this message: %08x\n", sessionID);
			if (sessionID==0){				
				if (receiveBuffer[4]==SVC_COMMAND_FRAME && receiveBuffer[5]==SVC_CMD_REGISTER_APP){
					processCommand(receiveBuffer, len);
				}
				/*
				else: frame error, ignore. sessionID = 0 only allowed for SVC_CMD_REGISTER_APP
				*/
			}
			else{				
				if (appTable[sessionID]!=NULL){
					if (receiveBuffer[4]==SVC_COMMAND_FRAME){
						processCommand(receiveBuffer, len);
					}
					else if (receiveBuffer[4]==SVC_DATA_FRAME) {
						processData(receiveBuffer, len);
					}
					/*
					else: invalid frame format, justs ignore
					*/
				}
				/*
				else: sessionID not existed
				*/
			}
		}
		else{
			//read error
		}
	}
	cout<<"Exit reading loop"<<endl;	
}

int main(int argc, char** argv){

	receiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);	
	
	//1. check if the daemon is already existed
	int unlinkResult = unlink(SVC_DAEMON_PATH.c_str());
	if (!(unlinkResult==0 || (unlinkResult==-1 && errno==ENOENT))){
		cout<<"unlink result "<<unlinkResult<<", error: "<<errno<<endl;
		errorString = SVC_ERROR_NAME_EXISTED;
		goto errorInit;
	}

	//2. create a daemon server socket
	memset(&daemonAddress, 0, sizeof(daemonAddress));
	daemonAddress.sun_family = AF_LOCAL;
	memcpy(daemonAddress.sun_path, SVC_DAEMON_PATH.c_str(), SVC_DAEMON_PATH.size());	
	
	//2.1 bind the socket
	daemonSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);	
	if (bind(daemonSocket, (struct sockaddr*) &daemonAddress, sizeof(daemonAddress)) == -1) {		
		errorString = SVC_ERROR_BINDING;
        goto errorInit;
    }
    
    //3.create a reading thread to read for incoming messages
    working = true;
    pthread_attr_init(&readingThreadAttr);
    pthread_create(&readingThread, &readingThreadAttr, readingLoop, NULL);
   
    goto initSuccess;
    
    errorInit:
    	cout<<errorString<<endl;
    	throw errorString;
    	
    initSuccess:
    	//do something
    	printf("SVC daemon is running...\n");
    	pthread_join(readingThread, NULL);
    	unlink(SVC_DAEMON_PATH.c_str());
}


