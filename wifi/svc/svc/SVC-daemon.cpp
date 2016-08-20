#include "SVC-header.h"

#include <string>
#include <iostream>
#include <unordered_map>
#include <unistd.h>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <shared_mutex>

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>

using namespace std;


#define		SVC_DAEPORT			1221

class DaemonService;

static unordered_map<uint32_t, DaemonService*> appTable;
static shared_mutex appTableMutex;

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

	uint8_t* tempBuffer1;
	uint8_t* tempBuffer2;
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
	SignalNotificator signalNotificator;
	
	bool working;
	shared_mutex workingMutex;
	
	public:	

		uint32_t appID;			
		
		Queue<Message*>* incomingQueue; 	/*	this queue is to be processed by local threads	*/
		Queue<Message*>* outgoingQueue; 	/*	this queue is to be processed by local threads	*/
		Queue<Message*>* inQueue; 		/*	this queue is to be redirect to app	*/
		Queue<Message*>* outQueue; 		/*	this queue is to be sent outside	*/
		
		DaemonService(uint32_t appID){
			this->appID = appID;
			
			/*	init queues	*/
			this->incomingQueue = new Queue<Message*>();
			this->outgoingQueue = new Queue<Message*>();
			this->inQueue = new Queue<Message*>();
			this->outQueue = new Queue<Message*>();
			
			this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
			/*	create unix socket and connect to send packet	*/
			memset(&this->unSockAddress, 0, sizeof(this->unSockAddress));
			this->unSockAddress.sun_family = AF_LOCAL;
			memcpy(this->unSockAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());					
			this->unSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
			connect(this->unSock, (struct sockaddr*) &this->unSockAddress, sizeof(this->unSockAddress));
			
		}
		
		void startService(){
			this->workingMutex.lock();
			this->working = true;
			this->workingMutex.unlock();
			pthread_attr_init(&threadAttrIn);
			pthread_attr_init(&threadAttrOut);
			pthread_create(&incomingProcessingThread, &threadAttrIn, processIncomingMessage, this);
			pthread_create(&outgoingProcessingThread, &threadAttrOut, processOutgoingMessage, this);
		}
		
		void stopService(){
			this->workingMutex.lock();
			this->working = false;
			this->workingMutex.unlock();
		}
		
		bool isWorking(){
			bool rs;
			this->workingMutex.lock_shared();
			rs = this->working;
			this->workingMutex.unlock_shared();
			return rs;
		}
		
		int receiveMessageFromOutside(Message* message){
			this->incomingQueue->enqueue(message);
			/*	check whether one thread is waiting for this message*/
			checkNotificationList(message);
		}
		
		int receiveMessageFromApp(Message* message){			
			this->outgoingQueue->enqueue(message);
			/*	check whether one thread is waiting for this message*/
			checkNotificationList(message);
		}
		
		void checkNotificationList(Message* message){
			uint8_t infoByte = message->data[5];
			if (infoByte & SVC_COMMAND_FRAME){
				enum SVCCommand cmd = (enum SVCCommand)message->data[6];			
				SVCDataReceiveNotificator* notificator = signalNotificator.getNotificator(cmd);
				/*	check if the received command matches handler's command	*/
				if (notificator != NULL){
					/*	perform callback	*/
					notificator->handler(message->data, message->len, notificator);
					/*	remove handler	*/
					signalNotificator.removeNotificator(cmd);
				}
				/*
				else: there is no notificator for this command
				*/			
			}
			/*
			else: no notification for data frame
			*/
		}
		
		void sendPacketToApp(){
			/*	dequeue message	*/
			if (this->inQueue->notEmpty()){
				Message* message;
				if (this->inQueue->peak(&message)){
					/*	send message to the app	*/

					this->sendToApp(message);
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
				Message* message;
				if (this->outQueue->peak(&message)){
					this->sendOutside(message);
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
		
		ssize_t sendToApp(const Message* message){
			return send(this->unSock, message->data, message->len, 0);
		}
		
		ssize_t sendOutside(const Message* message){
			return sendto(daemonInSocket, message->data, message->len, 0, (struct sockaddr*) &this->inSockAddress, sizeof(this->inSockAddress));
		}
		
		int encryptMessage(const uint8_t* plainMessage, const size_t* plainLen, uint8_t* encryptedMessage, size_t* encryptedLen){
		
		}
		
		int decryptMessage(const uint8_t* encryptedMessage, size_t encryptedLen, uint8_t* plainMessage, size_t* plainLen){
		
		}
		
		static void* processIncomingMessage(void* args){
			DaemonService* _this = (DaemonService*)args;
			
			while (_this->isWorking()){
				
			}
		}

		static void* processOutgoingMessage(void* args){
			DaemonService* _this = (DaemonService*)args;

			/*	iterate the outgoing queue	*/			
			uint8_t* allocBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
			size_t len;
			
			Message* message;
						
			while (_this->isWorking()){
				if (_this->outgoingQueue->peak(&message)){
					uint32_t sessionID;
					uint8_t infoByte;
					
					printf("can peak\n");
					/*	data or command	*/
					printBuffer(message->data, message->len);
					sessionID = *((uint32_t*)message->data);
					infoByte = message->data[4];					
					
					if (infoByte & SVC_COMMAND_FRAME){
						/*	command frame, extract arguments	*/												
						
						uint8_t argc = message->data[6];
						SVCCommandParam* argv[argc];
						uint8_t* pointer = message->data+7;
						
						for (int i=0; i<argc; i++){
							/*	just get pointer, not performing copy	*/
							argv[i] = new SVCCommandParam();
							argv[i]->length = *((uint16_t*)pointer);
							argv[i]->param = pointer + 2;
							pointer += 2+argv[i]->length;
						}						
						
						/*	use this 'params' to hold params for response	*/
						vector<SVCCommandParam*> params;
		
						enum SVCCommand cmd = (enum SVCCommand)message->data[5];

						switch (cmd){
							case SVC_CMD_CHECK_ALIVE:
								/* do nothing	*/
								break;
							case SVC_CMD_REGISTER_APP:
								uint32_t appID;
								ssize_t rs;
								bool newAppAllowed;
								
								printf("processing CMD_REGISTER_APP\n");

								/*	check if app existed	*/
								appID = *((uint32_t*)(argv[0]->param));
								printf("appID %08x\n", appID);
								newAppAllowed = false;
								sessionID = appExisted(appID);
								printf("sessionID %08x\n", sessionID);
								size_t size;

								/*	wait for alive response from app if sessionID != SVC_DEFAULT_SESSIONID	*/
								if (sessionID != SVC_DEFAULT_SESSIONID){
									printf("check for app alive\n");
									params.clear();
									prepareCommand(allocBuffer, &size, sessionID, SVC_CMD_CHECK_ALIVE, &params);
									/*	send this to app	*/
									appTableMutex.lock_shared();
									if (appTable[sessionID]!=NULL){
										appTable[sessionID]->inQueue->enqueue(new Message(allocBuffer, size));
										newAppAllowed = !(appTable[sessionID]->signalNotificator.waitCommand(SVC_CMD_CHECK_ALIVE, &params, SVC_SHORT_TIMEOUT));
										if (newAppAllowed){
											/*	remove old service */
											appTable[sessionID]->stopService();
											delete appTable[sessionID];
										}
										/*
										else:
										*/
									}
									else{
										newAppAllowed = true;
									}									
									appTableMutex.unlock_shared();									
								}
								else
									newAppAllowed = true;											

								if (newAppAllowed){
									printf("add new service for: %08x\n", appID);
									/*	create sessionID = hash(appID & rand)	*/
									hash<string> hasher;
									sessionID = (uint32_t)hasher(to_string(appID) + to_string(rand()));
									printf("new sessionID : %08x\n", sessionID);

									appTableMutex.lock();									
									appTable[sessionID] = new DaemonService(appID);
									appTable[sessionID]->startService();
									appTableMutex.unlock();								
									
									/*	repsonse sessionID to SVC app	*/
									params.clear();
									params.push_back(new SVCCommandParam(4, (uint8_t*)(&sessionID)));
									params.push_back(new SVCCommandParam(4, (uint8_t*)(argv[1]->param)));
									prepareCommand(allocBuffer, &size, sessionID, SVC_CMD_REGISTER_APP, &params);
									
									/*	this service has now its own session ID	*/
									appTableMutex.lock_shared();									
									appTable[sessionID]->inQueue->enqueue(new Message(allocBuffer, size));
									appTableMutex.unlock_shared();
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
					//printf("seg 1\n");
					_this->outgoingQueue->dequeue();
					//printf("seg 2\n");
				}
				/*
				else: peak failed, mutex locked
				*/
			}
			
			delete allocBuffer;		
		}
	
		~DaemonService(){
			printf("stopping service for appID %08x\n", this->appID);
			this->working = false;
			pthread_join(incomingProcessingThread, NULL);
			pthread_join(outgoingProcessingThread, NULL);
			printf("child threads joined\n");
			/*	these queues are warrantied to dequeue all left message in destructor */
			delete this->outgoingQueue;
			delete this->incomingQueue;
			delete this->inQueue;
			delete this->outQueue;
			printf("service for appID %08x stopped\n", this->appID);
		}
};


//check app existed, return sessionID
uint32_t appExisted(uint32_t appID){
	appTableMutex.lock_shared();
	for (auto& it : appTable){
		if (it.second != NULL){
			if (it.second->appID == appID){
				appTableMutex.unlock_shared();
				return it.first;
			}
		}
	}
	appTableMutex.unlock_shared();
	return SVC_DEFAULT_SESSIONID;
}

void signal_handler(int sig){
	if (sig == SIGINT){
		printf("SIGINT caught, stopping daemon\n");
		/*	stop all services	*/
		appTableMutex.lock_shared();
		for (auto& it : appTable){
			if(it.second!=NULL){ 
				it.second->stopService();
			}
		}
		appTableMutex.unlock_shared();
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
			
			/*	check if sessionID is registered	*/
			uint32_t sessionID = *((uint32_t*)unixReceiveBuffer);
			
			appTableMutex.lock_shared();
			if (appTable[sessionID]!=NULL){							
				appTable[sessionID]->receiveMessageFromApp(new Message(unixReceiveBuffer, byteRead));
			}
			appTableMutex.unlock_shared();
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
			appTableMutex.lock_shared();
			DaemonService* service = appTable[sessionID];
			appTableMutex.unlock_shared();
			
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
							service->receiveMessageFromOutside(new Message(htpReceiveBuffer, byteRead));
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
							service->receiveMessageFromOutside(new Message(htpReceiveBuffer, byteRead));
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
	
	DaemonService* service;
	
	while (working){
		appTableMutex.lock_shared();
  		for (auto& it : appTable){
  			if (service!=NULL){ 			
				service = it.second;
				if (service->isWorking()){
					service->sendPacketOutside();
				}
	    	}
    	}
    	appTableMutex.unlock_shared();
    }
    printf("Exit htp redirecting loop\n");
}


void* unixRedirectingLoop(void* args){
	
	DaemonService* service;
	
	/*	read from service in queue	*/
	while(working){
		appTableMutex.lock_shared();
		for (auto& it : appTable){	
			//if (it.first != SVC_DEFAULT_SESSIONID) printf("ok1\n");	
			service = it.second;
			if (service!=NULL){
				if (service->isWorking()){
					//if (it.first != SVC_DEFAULT_SESSIONID) printf("ok2\n");
					service->sendPacketToApp();
				}
			}
		}
		appTableMutex.unlock_shared();
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
    
    /*	set thread signal mask	*/
    sigset_t sigset;
    sigemptyset(&sigset);    
    sigaddset(&sigset, SVC_ACQUIRED_SIGNAL);
    sigaddset(&sigset, SVC_TIMEOUT_SIGNAL);
    sigaddset(&sigset, SVC_SHARED_MUTEX_SIGNAL);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    
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
	appTableMutex.lock();
	appTable[SVC_DEFAULT_SESSIONID] = new DaemonService(SVC_DEFAULT_SESSIONID);
	appTable[SVC_DEFAULT_SESSIONID]->startService();
	appTableMutex.unlock();
    
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
    	appTableMutex.lock();
		for (auto& it : appTable){
			if (it.second!=NULL)
				delete it.second;
		}
		appTableMutex.unlock();
		
		delete unixReceiveBuffer;
		delete htpReceiveBuffer;
    	unlink(SVC_DAEMON_PATH.c_str());

}


