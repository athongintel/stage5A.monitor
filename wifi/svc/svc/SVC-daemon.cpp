#include "SVC-header.h"

#include <string>
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <cstdlib>
#include <csignal>
#include <iostream>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>

using namespace std;


#define		SVC_DAEPORT			1221


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
		
		/*	one incomming and one outgoing queue. already thread-safe (at least they said :D)*/
		
		
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
			
			/*	create new HTP socket, bind to 1221
				for now using UDP socket, to be replaced
			*/			
		}
};

unordered_map<uint32_t, DaemonService*> appTable;

struct sockaddr_un daemonSockUnAddress;
struct sockaddr_in daemonSockInAddress;

int daemonUnSocket;
int daemonInSocket;

pthread_t unixReadingThread;
pthread_t htpReadingThread;
pthread_attr_t htpReadingThreadAttr;
pthread_attr_t unixReadingThreadAttr;

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

void processOutgoingCommand(const uint8_t* buffer, size_t len){
	
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
	//	printBuffer(pointer+2, length);
		argv.push_back(new SVCCommandParam(length, pointer+2));
		//add 2 byte of length and the arg's length
		pointer+= 2+length;
	}

	vector<SVCCommandParam*> params;
	switch(cmd->cmdID){
		case SVC_CMD_REGISTER_APP:						
			uint32_t appID;
			ssize_t sendre;
			bool newAppAllowed;
			
			//a.1 check if app existed
			appID = *((uint32_t*)(argv[0]->param));
			//printf("appID: %08x\n",appID);
			
			newAppAllowed = false;
			sessionID = appExisted(appID);
			if (sessionID!=0){
				//check if alive
				sendre = _sendCommand(appTable[sessionID]->sock, sessionID, SVC_CMD_CHECK_ALIVE, &params);				
				newAppAllowed = (sendre ==-1) && (errno == ECONNREFUSED || errno == ENOTCONN);	
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

void processOutgoingData(uint8_t* buffer, size_t len){
}


void processIncomingCommand(uint8_t* buffer, size_t len){
	
}

void signal_handler(int sig){
	if (sig == SIGINT){
		printf("SIGINT caught, stopping daemon\n");
		working = false;
	}
}

void* unixReadingLoop(void* args){

	//string test;
	//read message from socket in blocking-mode
	
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);
	int byteRead;

	while (working){
	
		do{
			byteRead = recv(daemonSocket, receiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
			
		//printf("byte read: %d, errno: %d, working: %d\n", byteRead, errno, working);
		/*	process message. message is stored in receiveBuffer	*/
		if (byteRead>0){
			//printf("read a message of %d\n", (int)byteRead);
			printf("byte read: %d\n", byteRead);
			printBuffer(receiveBuffer, byteRead);
			
			/*	check if sessionID is registered	*/
			uint32_t sessionID = *((uint32_t*)receiveBuffer);
			//printf("sessionID of this message: %08x\n", sessionID);
			if (sessionID==0){				
				if (receiveBuffer[4]==SVC_COMMAND_FRAME && receiveBuffer[5]==SVC_CMD_REGISTER_APP){
					processOutgoingCommand(receiveBuffer, byteRead);
				}
				/*
					else: frame error, ignore. sessionID = 0 only allowed for SVC_CMD_REGISTER_APP
				*/
			}
			else{
				if (appTable[sessionID]!=NULL){
					if (receiveBuffer[4]==SVC_COMMAND_FRAME){
						processOutgoingCommand(receiveBuffer, byteRead);
					}
					else if (receiveBuffer[4]==SVC_DATA_FRAME) {
						processOutgoingData(receiveBuffer, byteRead);
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
		/*
			else: read error
		*/
	}
	cout<<"Exit reading loop"<<endl;	
}

void* htpReadingLoop(void* args){
	
}


int main(int argc, char** argv){

	receiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);	
	
	//1. check if the daemon is already existed
	int unlinkre = unlink(SVC_DAEMON_PATH.c_str());
	if (!(unlinkre==0 || (unlinkre==-1 && errno==ENOENT))){
		cout<<"unlink rs "<<unlinkre<<", error: "<<errno<<endl;
		errorString = SVC_ERROR_NAME_EXISTED;
		goto errorInit;
	}

	//2. create a daemon server socket
	memset(&daemonSockUnAddress, 0, sizeof(daemonSockUnAddress));
	daemonSockUnAddress.sun_family = AF_LOCAL;
	memcpy(daemonSockUnAddress.sun_path, SVC_DAEMON_PATH.c_str(), SVC_DAEMON_PATH.size());		
	//2.1 bind the socket
	daemonSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);	
	if (bind(daemonSocket, (struct sockaddr*) &daemonSockUnAddress, sizeof(daemonSockUnAddress)) == -1) {		
		errorString = SVC_ERROR_BINDING;
        goto errorInit;
    }
    
    /*	TO BE CHANGED TO HTP	*/
    //3. create htp socket
    memset(&daemonSockInAddress, 0, sizeof(daemonSockInAddress));
    daemonSockInAddress.sin_family = AF_INET;
    daemonSockInAddress.sin_port = htons(SVC_DAEPORT);
	daemonSockInAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //3.1	bind this socket to localhost
    daemonInSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(daemonInSocket, (struct sockaddr*) &daemonSockInAddress, sizeof(daemonSockInAddress))){
    	errorString = SVC_ERROR_BINDING;
    	goto errorInit;
    }
    
    //3.	handle signals
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);
    
    //4.	create a thread to read from unix domain socket
    working = true;
    pthread_attr_init(&unixReadingThreadAttr);
    pthread_create(&unixReadingThread, &unixReadingThreadAttr, unixReadingLoop, NULL);
      
	//5.	create a thread to read from htp socket
	pthread_attr_init(&htpReadingThreadAttr);	
	pthread_create(&htpReadingThread, &htpReadingThreadAttr, htpReadingLoop, NULL);
   
    goto initSuccess;
    
    errorInit:
    	cout<<errorString<<endl;
    	throw errorString;
    	
    initSuccess:
    	//do something
    	printf("SVC daemon is running...\n");
    	pthread_join(unixReadingThread, NULL);   
    	
    	printf("SVC daemon stopped.\n");
    	
    	//do cleaning up
    	unlink(SVC_DAEMON_PATH.c_str());
    	
}


