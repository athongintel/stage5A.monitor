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


class DaemonService{
	
	public:	
		string svcClientPath;
		sockaddr_un unSockAddress;
		int unSock;
				
		uint32_t appID;
				
		/*	one incomming and one outgoing queue */
		MessageQueue* outgoingQueue;
		MessageQueue* incomingQueue;
		
		DaemonService(){
			//init incoing and outgoing queue
			this->appID = 0;
			this->outgoingQueue = new MessageQueue();
			this->incomingQueue = new MessageQueue();
		}
		
		DaemonService(uint32_t appID) : DaemonService(){
			this->appID = appID;
				
			//create unix socket and connect to send packet
			this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
			memset(&this->unSockAddress, 0, sizeof(this->unSockAddress));
			this->unSockAddress.sun_family = AF_LOCAL;
			memcpy(this->unSockAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());					
			this->unSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
			connect(this->unSock, (struct sockaddr*) &this->unSockAddress, sizeof(this->unSockAddress));
		}
		
		int encryptMessage(const uint8_t* plainMessage, const size_t* plainLen, uint8_t* encryptedMessage, size_t* encryptedLen){
		
		}
		
		int decryptMessage(const uint8_t* encryptedMessage, const size_t* encryptedLen, uint8_t* plainMessage, size_t* plainLen){
		
		}
	
		~DaemonService(){
			delete outgoingQueue;
			delete incomingQueue;
		}
};

unordered_map<uint32_t, DaemonService*> appTable;

struct sockaddr_un daemonSockUnAddress;
struct sockaddr_in daemonSockInAddress;

int daemonUnSocket;
int daemonInSocket;

pthread_t unixReadingThread;
pthread_t htpReadingThread;
pthread_t unixProcessingThread;
pthread_t htpProcessingThread;

pthread_attr_t htpReadingThreadAttr;
pthread_attr_t unixReadingThreadAttr;
pthread_attr_t unixProcessingThreadAttr;
pthread_attr_t htpProcessingThreadAttr;

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


void signal_handler(int sig){
	if (sig == SIGINT){
		printf("SIGINT caught, stopping daemon\n");
		working = false;
	}
}

void* unixReadingLoop(void* args){
	
	//forward packets to its correspond outgoing queue	
	int byteRead;

	while (working){	
		do{
			byteRead = recv(daemonSocket, receiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		/*	process message. message is stored in receiveBuffer	*/
		if (byteRead>0){			
			printf("byte read from unix: %d\n", byteRead);
			printBuffer(receiveBuffer, byteRead);
			
			/*	check if sessionID is registered	*/
			uint32_t sessionID = *((uint32_t*)receiveBuffer);
			printf("sessionID of this message: %08x\n", sessionID);
			if (appTable[sessionID]!=NULL){			
				appTable[sessionID]->outgoingQueue->enqueue(receiveBuffer, byteRead);			
			}			
		}
		/*
		else: read error
		*/
	}
	cout<<"Exit unix reading loop"<<endl;
}

void* htpReadingLoop(void* args){
	
	/*	forward packet to correspondind incoming queue	*/
	int byteRead;
	
	while (working){	
		do{
			byteRead = recv(daemonSocket, receiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		/*	process message. message is stored in receiveBuffer	*/
		if (byteRead>0){			
			printf("byte read from htp: %d\n", byteRead);
			printBuffer(receiveBuffer, byteRead);
						
			/*	check if sessionID is registered	*/
			/*uint32_t sessionID = *((uint32_t*)receiveBuffer);
			printf("sessionID of this message: %08x\n", sessionID);
			if (appTable[sessionID]!=NULL){			
				appTable[sessionID]->outgoingQueue->enqueue(receiveBuffer, byteRead);			
			}*/			
		}
		/*
		else: read error
		*/
	}
	cout<<"Exit htp reading loop"<<endl;
}

void _processIncomingMessage(uint8_t* buffer, size_t len){
	
	/*	
		The buffer passed into this function is stored in the message queue.
		Hence, it is unnecessary to alloc memory as we can modify directly buffer.
		Encrypted messages have already been decrypted.
	*/
	
}

void _processOutgoingMessage(uint8_t* buffer, size_t len){

	/*	
		The buffer passed into this function is stored in the message queue.
		Hence, it is unnecessary to alloc memory as we can modify directly buffer.
	*/
	
	vector<SVCCommandParam*> argv;
	//pass 2 byte of command type and argc
	pointer+= 2;

	for (int i=0; i<cmd->argc; i++){
		uint16_t length = *(pointer);
	//	printf("param %d length: %04x, param: ",i, length);
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


void* htpProcessingLoop(void* args){
	//iteration from all service and read from incoming queue
	uint32_t sessionID;
	DaemonService* service;
	MessageQueue* queue;
	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	uint8_t* decryptedMessage = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t len;
	size_t decryptedLen;
	
	while (working){	
  		for (auto& it : appTable){  			
    		service = it.second;
    		queue = service->incomingQueue;
    		
    		if (queue->messageMutex.try_lock()){
    			//dequeue the message
    			if (queue->notEmpty()){
    				if (queue->dequeue(buffer, &len)!=-1){
    					/*	read for info	*/
    					sessionID = it.first;
						uint8_t infoByte = buffer[4];
						if (infoByte & SVC_ENCRYPTED){							/
							if (sessionID != SVC_DEFAUT_SESSIONID){
								/*	decrypt the message	*/
								if (service->decrypteMessage(buffer+5, len-5, decryptedMessage, &decryptedLen)){
									/*	replace encrypted content with decrypted one and process */
									memcpy(buffer+5, decryptedMessage, decryptedLen);
									/*	update new size	*/
									len = 5 + decryptedSize;
									_processIncomingMessage(buffer, len);
								}
								/*
								else: failed to decrypt message, ignore
								*/							
							}
							/*
							else: ignore encrypted packet with default sessionID
							*/
						}
						else{
							/*	non-encrypted packet	*/
							if ((infoByte & SVC_COMMAND_FRAME)){
								enum SVCCommand cmd = buffer[5];
								if (!isEncryptedCommand(cmd)){
									_processIncomingMessage(buffer, size);
								}
								/*
								else: non-encrypted commands are only allowed for certain commands
								*/
							}
							/*
							else: non-encrypted data frame not allowed
							*/							
						}	
    				}
    			}
    			queue->messageMutex.unlock();
    		}
    	}
    }
    
    delete decryptedMessage;
    delete buffer;
}


void* unixProcessingLoop(void* args){
	
	DaemonService* service;
	MessageQueue* queue;
	
	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t len;
	
	/*	read from service outgoing queue	*/
	while(working){
		for (auto& it : appTable){				
			service = it.second;
			queue = service->outgoingQueue;
			
			/*	dequeue message	*/
			if (queue->messageMutex.try_lock()){			
				if (queue->notEmpty()){
					if (queue->dequeue(buffer, &size)!=-1){
						/*	process message	*/
						_processOutgoingMessage(buffer, size);
					}
					/*
					else: dequeue failed
					*/
				}
				/*
				else: queue is empty
				*/
			}
			/*
			else: cannot lock mutex
			*/
		}
	}
	
	delete buffer;
}


int main(int argc, char** argv){

	receiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);	
	
	//1. check if the daemon is already existed
	int rs = unlink(SVC_DAEMON_PATH.c_str());
	if (!(rs==0 || (rs==-1 && errno==ENOENT))){	
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
	
	//add default DaemonService to handle non-encrypted request
	appTable[SVC_DEFAULT_SESSIONID] = new DaemonService();
    
    //4.	create a thread to read from unix domain socket
    working = true;
    pthread_attr_init(&unixReadingThreadAttr);
    pthread_create(&unixReadingThread, &unixReadingThreadAttr, unixReadingLoop, NULL);
      
	//5.	create a thread to read from htp socket
	pthread_attr_init(&htpReadingThreadAttr);	
	pthread_create(&htpReadingThread, &htpReadingThreadAttr, htpReadingLoop, NULL);
	
	//6.	create a thread to process and redirect incoming messages
	pthread_attr_init(&htpProcessingThreadAttr);	
	pthread_create(&htpProcessingThread, &htpProcessingThreadAttr, htpProcessingLoop, NULL);

	//7.	create a thread to process and redirect outgoing messages
   	pthread_attr_init(&unixProcessingThreadAttr);	
	pthread_create(&unixProcessingThread, &unixProcessingThreadAttr, unixProcessingLoop, NULL);

    goto initSuccess;
    
    errorInit:
    	cout<<errorString<<endl;
    	throw errorString;
    	
    initSuccess:
    	//do something
    	printf("SVC daemon is running...\n");
    	pthread_join(unixReadingThread, NULL);   
    	pthread_join(unixProcessingThread, NULL);
    	pthread_join(htpReadingThread, NULL);
    	pthread_join(htpProcessingThread, NULL);    	
    	printf("SVC daemon stopped.\n");
    	
    	//do cleaning up
    	unlink(SVC_DAEMON_PATH.c_str());    	
}


