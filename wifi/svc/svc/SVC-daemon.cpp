#include "SVC-header.h"

#include <string>
#include <iostream>
#include <unordered_map>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;


struct SVCCmd{
	enum SVCCommand cmdID;
	uint8_t argc;
	vector<SVCCommandParam*>* argv;
};

class DaemonService{
	public:
		sockaddr_un sockAddress;
		int sock;
		string svcClientPath;
		
		DaemonService(uint32_t appID){
			//create socket address
			this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
			memset(&this->sockAddress, 0, sizeof(this->sockAddress));
			this->sockAddress.sun_family = AF_LOCAL;
			memcpy(this->sockAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());
			printf("connect to sock: %s\n", this->svcClientPath.c_str());
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


volatile bool working;

//receive buffer and msghdr
uint8_t receiveBuffer[SVC_DEFAULT_BUFSIZ];
struct msghdr messageHeader;
struct iovec io[1];

void* readingLoop(void* args){
	//read message from socket in blocking-mode
	while (working){
		size_t len = recvmsg(daemonSocket, &messageHeader, 0);
		cout<<"read a message of "<<len<<endl;
		//process message. message is stored in receiveBuffer
		if (len>0){
			if (receiveBuffer[0]==SVC_COMMAND_FRAME){
				cout<<"a command frame"<<endl;
				//this is command frame
				//extract params
				printBuffer(receiveBuffer, len);
				const uint8_t* pointer = receiveBuffer;
				//pass 1 byte of frame type
				pointer = pointer+1;
				struct SVCCmd* cmd = (struct SVCCmd*)(pointer);
				cmd->argv = new vector<SVCCommandParam*>();
				vector<SVCCommandParam*> &refVector = *(cmd->argv);
				//pass 2 byte of command type and argc
				pointer+=2;
				printf("param count: %d\n",cmd->argc);
				for (int i=0; i<cmd->argc; i++){
					//printf("before length\n");
					uint16_t length = *(pointer);
					printf("param %d length: %d\n",i, length); 
					refVector.push_back(new SVCCommandParam(length, pointer+2));
					pointer+=2+length;
				}
				
				vector<SVCCommandParam*> params;
				switch(cmd->cmdID){
					case SVC_CMD_REGISTER_APP:						
						uint8_t response;
						uint32_t appID;				
						//a.1 check if app existed						
						appID = *((uint32_t*)(refVector[0]->param));
						printf("appID: %08x\n",appID);
						if (appTable[appID]!=NULL){
							//app existed
							response = 0;
						}
						else{
							response = 1;
							//a.2 add new record to appTable
							appTable[appID] = new DaemonService(appID);	
						}					
						//a.3 repsonse to SVC app interface
						params.push_back(new SVCCommandParam(1, &response));
						printf("send back response %d to SVC interface %d %s\n", response, appTable[appID]->sock, appTable[appID]->sockAddress.sun_path);
						_sendCommand(appTable[appID]->sock, SVC_CMD_REGISTER_APP, &params);
						break;
						
					default:
						break;
				}
			}
			else{
				//this is data frame
			}
		}
		else{
			//read error
		}
	}
	cout<<"Exit reading loop"<<endl;
	
}

int main(int argc, char** argv){

	io[0].iov_base = receiveBuffer;
	io[0].iov_len = SVC_DEFAULT_BUFSIZ;
	messageHeader.msg_iov = io;
	messageHeader.msg_iovlen = 1;

	string errorString;
	
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
		cout<<"err no :"<<errno;
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
    	cout<<"SVC daemon is running...";
    	pthread_join(readingThread, NULL);
}


