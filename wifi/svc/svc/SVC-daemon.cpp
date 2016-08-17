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
#include <netinet/in.h>

using namespace std;


#define		SVC_DAEPORT			1221

class DaemonService;
unordered_map<uint32_t, DaemonService*> appTable;
uint32_t appExisted(uint32_t appID);

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

uint8_t* htpReceiveBuffer;
uint8_t* unixReceiveBuffer;
struct sockaddr messageAddress;
socklen_t messageAddressLen;

class DaemonService{

	uint8_t** tempBuffer1;
	uint8_t** tempBuffer2;
	size_t tempSize1;
	size_t tempSize2;
	
	int unSock;
	sockaddr_un unSockAddress;
	/*	this inSockAddress use the global htpSocket	*/
	struct sockaddr_in inSockAddress;
	
	string svcClientPath;
	
	pthread_attr_t threadAttrIn;
	pthread_attr_t threadAttrOut;
	pthread_t incomingProcessingThread;
	pthread_t outgoingProcessingThread;
	
	//ssize_t sendToApp(const uint8_t* buffer, size_t len);
	//ssize_t sendOutside(const uint8_t* buffer, size_t len);
	
	public:	

		uint32_t appID;			
		
		volatile bool working;

		MessageQueue* incomingQueue; 	/*	this queue is to be processed by local threads	*/
		MessageQueue* outgoingQueue; 	/*	this queue is to be processed by local threads	*/
		MessageQueue* inQueue; 			/*	this queue is to be redirect to app	*/
		MessageQueue* outQueue; 		/*	this queue is to be sent outside	*/
		
		DaemonService(uint32_t appID){
			this->appID = appID;
			
			/*	init queues	*/
			this->outgoingQueue = new MessageQueue();
			this->incomingQueue = new MessageQueue();
			this->inQueue = new MessageQueue();
			this->outQueue = new MessageQueue();		
			
			//printf("init 4 queues\n");
			
			
			this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
			/*	create unix socket and connect to send packet	*/
			memset(&this->unSockAddress, 0, sizeof(this->unSockAddress));
			this->unSockAddress.sun_family = AF_LOCAL;
			memcpy(this->unSockAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());					
			this->unSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
			connect(this->unSock, (struct sockaddr*) &this->unSockAddress, sizeof(this->unSockAddress));
			
		}
		
		void startService(){
			this->working = true;
			pthread_attr_init(&threadAttrIn);
			pthread_attr_init(&threadAttrOut);
			pthread_create(&incomingProcessingThread, &threadAttrIn, processIncomingMessage, this);
			pthread_create(&outgoingProcessingThread, &threadAttrOut, processOutgoingMessage, this);
		}
		
		void stopService(){
			this->working = false;
		}
		
		int receiveMessageFromOutside(const uint8_t* buffer, size_t len){
			this->incomingQueue->enqueue(buffer, len);
			/*	check whether one thread is waiting for this message*/
		}
		
		int receiveMessageFromApp(const uint8_t* buffer, size_t len){
			//printf("try to enqueue:\n");
			this->outgoingQueue->enqueue(buffer, len);
			/*	check whether one thread is waiting for this message*/
		}
		
		void sendPacketToApp(){
			/*	dequeue message	*/			
			if (this->inQueue->notEmpty()){
				if (this->inQueue->peak(tempBuffer1, &tempSize1)!=-1){
					/*	send message to the app	*/
					this->sendToApp(*tempBuffer1, tempSize1);
					this->inQueue->dequeue();
				}
				/*
				else: peak failed, mutex locked
				*/
			}
			/*
			else: queue is empty
			*/			
		}
		
		void sendPacketOutside(){
			if (this->outQueue->notEmpty()){
				/*	peak the message, only dequeue after used	*/
				if (this->outQueue->peak(tempBuffer2, &tempSize2)!=-1){
					this->sendOutside(*tempBuffer2, tempSize2);
					this->outQueue->dequeue();
				}
				/*
				esle: peak failed, mutex locked
				*/						
    		}
    		/*
    		else: queue is empty
    		*/  		
		}
		
		ssize_t sendToApp(const uint8_t* buffer, size_t len){
			return send(this->unSock, buffer, len, 0);
		}
		
		ssize_t sendOutside(const uint8_t* buffer, size_t len){
			return sendto(daemonInSocket, buffer, len, 0, (struct sockaddr*) &this->inSockAddress, sizeof(this->inSockAddress));
		}
		
		int encryptMessage(const uint8_t* plainMessage, const size_t* plainLen, uint8_t* encryptedMessage, size_t* encryptedLen){
		
		}
		
		int decryptMessage(const uint8_t* encryptedMessage, size_t encryptedLen, uint8_t* plainMessage, size_t* plainLen){
		
		}
		
		static void* processIncomingMessage(void* args){
			DaemonService* _this = (DaemonService*)args;
			
			while (_this->working){
				
			}
		}

		static void* processOutgoingMessage(void* args){
			DaemonService* _this = (DaemonService*)args;

			/*	iterate the outgoing queue	*/
			uint8_t** buf;
			uint8_t* buffer;
			size_t len;
			
			while (_this->working){		
				if (_this->outgoingQueue->peak(&buffer, &len)){
					/*	for easy reference	*/
					buffer = *buf;
					/*	data or command	*/
					uint32_t sessionID = *((uint32_t*)buffer);
					uint8_t infoByte = buffer[5];
	
					if (infoByte & SVC_COMMAND_FRAME){
						/*	command frame, extract arguments	*/
						
						uint8_t argc = buffer[7];
						SVCCommandParam* argv[argc];
						uint8_t* pointer = buffer+7;
						for (int i=0; i<argc; i++){
							/*	just get pointer, not performing copy	*/
							argv[i] = new SVCCommandParam();
							argv[i]->length = *((uint16_t*)pointer);
							argv[i]->param = pointer + 2;
							pointer += 2+argv[i]->length;
						}
		
						/*	use this 'params' to hold params for response	*/
						vector<SVCCommandParam*> params;
		
						enum SVCCommand cmd = (enum SVCCommand)buffer[6];
						switch (cmd){
							case SVC_CMD_CHECK_ALIVE:
								/* do nothing	*/
								break;
							case SVC_CMD_REGISTER_APP:
								uint32_t appID;
								ssize_t rs;
								bool newAppAllowed;

								/*	check if app existed	*/
								appID = *((uint32_t*)(argv[0]->param));								

								newAppAllowed = false;
								sessionID = appExisted(appID);
								if (sessionID != -1){
									size_t size;
									if (sessionID!=SVC_DEFAULT_SESSIONID){
										
										prepareCommand(buffer, &size, sessionID, SVC_CMD_CHECK_ALIVE, &params);
										/*	send this to app	*/
										_this->inQueue->enqueue(buffer, size);
										/*	wait for alive response from app	*/
										newAppAllowed = !waitCommand(SVC_CMD_CHECK_ALIVE, &params, SVC_DEFAULT_TIMEOUT);			
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
										appTable[sessionID]->startService();
					
										//a.3 repsonse sessionID to SVC app
										params.push_back(new SVCCommandParam(4, (uint8_t*)(&sessionID)));
										params.push_back(new SVCCommandParam(4, (uint8_t*)(argv[1]->param)));				
										prepareCommand(buffer, &size, SVC_DEFAULT_SESSIONID, SVC_CMD_REGISTER_APP, &params);
										_this->inQueue->enqueue(buffer, size);
									}
								}
								/*
								else: session ID not exist
								*/
								break;
							default:
								break;
						}
					}
					else{
						/*	This is a data frame	*/
						
					}
					/*	done, remove message from queue	*/
					_this->outgoingQueue->dequeue();
				}
				/*
				else: peak failed, mutex locked
				*/
			}		
		}
	
		~DaemonService(){
			this->working = false;
			pthread_join(incomingProcessingThread, NULL);
			pthread_join(outgoingProcessingThread, NULL);
			delete this->outgoingQueue;
			delete this->incomingQueue;
			delete this->inQueue;
			delete this->outQueue;
		}
};


//check app existed, return sessionID
uint32_t appExisted(uint32_t appID){
	for (auto& it : appTable){
		if (it.second->appID == appID){
			return it.first;
		}
	}
	return -1;
}

void signal_handler(int sig){
	if (sig == SIGINT){
		printf("SIGINT caught, stopping daemon\n");
		/*	stop all services	*/
		for (auto& it : appTable){
			it.second->working = false;
		}
		/*	stop main threads	*/
		working = false;
	}
}

void* unixReadingLoop(void* args){
	
	//forward packets to its correspond outgoing queue	
	int byteRead;

	while (working){	
		do{
			byteRead = recv(daemonUnSocket, unixReceiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		/*	process message. message is stored in unixReceiveBuffer	*/
		if (byteRead>0){			
			printf("byte read from unix: %d\n", byteRead);
			printBuffer(unixReceiveBuffer, byteRead);
			
			/*	check if sessionID is registered	*/
			uint32_t sessionID = *((uint32_t*)unixReceiveBuffer);
			
			if (appTable[sessionID]!=NULL){			
				printf("sessionID: %08x\n", sessionID);
				appTable[sessionID]->receiveMessageFromApp(unixReceiveBuffer, byteRead);	
			}
			/*
			else: invalid session ID
			*/	
		}
		/*
		else: read error
		*/
	}
	printf("Exit unix reading loop\n");
}

void* htpReadingLoop(void* args){
	
	/*	forward packet to correspondind incoming queue	*/
	size_t byteRead;
	uint8_t* decryptedMessage = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t decryptedLen;
	
	while (working){	
		do{
			byteRead = recv(daemonInSocket, htpReceiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		/*	process message. message is stored in htpReceiveBuffer	*/
		if ((int)byteRead>0){			
			printf("byte read from htp: %d\n", (int)byteRead);
			printBuffer(htpReceiveBuffer, byteRead);
			
			/*	check if sessionID is registered	*/
			uint32_t sessionID = *((uint32_t*)htpReceiveBuffer);
			DaemonService* service = appTable[sessionID];
			
			if (service != NULL){			
				uint8_t infoByte = htpReceiveBuffer[4];
				if (infoByte & SVC_ENCRYPTED){
					if (sessionID != SVC_DEFAULT_SESSIONID){
						/*	decrypt the message	*/
						if (service->decryptMessage(htpReceiveBuffer+5, byteRead-5, decryptedMessage, &decryptedLen)){
							/*	replace encrypted content with decrypted one and process */
							memcpy(htpReceiveBuffer+5, decryptedMessage, decryptedLen);
							/*	update new size	*/
							byteRead = 5 + decryptedLen;
							/*	add this message to processing queue	*/
							service->receiveMessageFromOutside(htpReceiveBuffer, byteRead);
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
						enum SVCCommand cmd = (enum SVCCommand)htpReceiveBuffer[5];
						if (!isEncryptedCommand(cmd)){
							service->receiveMessageFromOutside(htpReceiveBuffer, byteRead);
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
			/*
			else: invalid session ID
			*/									
		}
		/*
		else: read error
		*/
	}
	printf("Exit htp reading loop\n");
}


void* htpRedirectingLoop(void* args){
	
	uint32_t sessionID;
	DaemonService* service;
	
	while (working){	
  		for (auto& it : appTable){  			
    		service = it.second;
    		if (service->working){
				service->sendPacketOutside();
	    	}
    	}
    }
    printf("Exit htp redirecting loop\n");
}


void* unixRedirectingLoop(void* args){
	
	DaemonService* service;
	
	/*	read from service in queue	*/
	while(working){
		for (auto& it : appTable){				
			service = it.second;
			if (service->working){
				service->sendPacketToApp();
			}
		}
	}
	printf("Exit unix redirecting loop\n");
}

int main(int argc, char** argv){

	htpReceiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	unixReceiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	
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
	daemonUnSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);	
	if (bind(daemonUnSocket, (struct sockaddr*) &daemonSockUnAddress, sizeof(daemonSockUnAddress)) == -1) {		
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
	
	/*	
		add default DaemonService to handle non-encrypted request
		we can use SVC_DEFAULT_SESSIONID for init DaemonService
	*/
	appTable[SVC_DEFAULT_SESSIONID] = new DaemonService(SVC_DEFAULT_SESSIONID);
	//appTable[SVC_DEFAULT_SESSIONID]->startService();
    
    //4.	create a thread to read from unix domain socket
    working = true;
    pthread_attr_init(&unixReadingThreadAttr);
    pthread_create(&unixReadingThread, &unixReadingThreadAttr, unixReadingLoop, NULL);
      
	//5.	create a thread to read from htp socket
	pthread_attr_init(&htpReadingThreadAttr);	
	pthread_create(&htpReadingThread, &htpReadingThreadAttr, htpReadingLoop, NULL);
	
	//6.	create a thread to process and redirect incoming messages
	pthread_attr_init(&htpProcessingThreadAttr);	
	pthread_create(&htpProcessingThread, &htpProcessingThreadAttr, htpRedirectingLoop, NULL);

	//7.	create a thread to process and redirect outgoing messages
   	pthread_attr_init(&unixProcessingThreadAttr);	
	pthread_create(&unixProcessingThread, &unixProcessingThreadAttr, unixRedirectingLoop, NULL);

    goto initSuccess;
    
    errorInit:
		delete unixReceiveBuffer;
		delete htpReceiveBuffer;
    	cout<<errorString<<endl;
    	throw errorString;
    	
    initSuccess:
		/*	POST-SUCCESS JOBS	*/
    	printf("SVC daemon is running...\n");
    	pthread_join(unixReadingThread, NULL);   
    	pthread_join(unixProcessingThread, NULL);
    	pthread_join(htpReadingThread, NULL);
    	pthread_join(htpProcessingThread, NULL);    	 	
    	/*	wait here until catch SIGINT	*/
    	
    	/*	DO CLEANING UP BEFORE EXIT	*/
    	printf("SVC daemon stopped.\n");
    	/*	remove all DaemonService instances	*/
		for (auto& it : appTable){
			delete it.second;
		}
		delete unixReceiveBuffer;
		delete htpReceiveBuffer;
    	unlink(SVC_DAEMON_PATH.c_str());

}


