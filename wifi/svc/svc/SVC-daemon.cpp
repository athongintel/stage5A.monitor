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

#define SVC_VERSION 0x01<<6

using namespace std;

class DaemonService;

static unordered_map<uint64_t, DaemonService*> serviceTable;
static shared_mutex* serviceTableMutex;

//uint32_t appExisted(uint32_t appID);

struct sockaddr_un daemonSockUnAddress;
struct sockaddr_in daemonSockInAddress;

int daemonUnSocket;
int daemonInSocket;

pthread_t unixReadingThread;
pthread_t htpReadingThread;

//pthread_t unixProcessingThread;
//pthread_t htpProcessingThread;

pthread_attr_t htpReadingThreadAttr;
pthread_attr_t unixReadingThreadAttr;
//pthread_attr_t unixProcessingThreadAttr;
//pthread_attr_t htpProcessingThreadAttr;

string errorString;
volatile bool working;

uint8_t* htpReceiveBuffer;
uint8_t* unixReceiveBuffer;


class DaemonService{
	
	private:
	
		class ServiceEndPoint{
			private:
				/*TODO:	this must be changed to htp socket	*/
				int inSock;
				struct sockaddr_in inSockAddress;
				
				pthread_attr_t threadAttrIn;
				pthread_attr_t threadAttrOut;
				pthread_t incomingProcessingThread;
				pthread_t outgoingProcessingThread;
				
				MutexedQueue<Message*>* incomingQueue; 		/*	this queue is to be processed by local threads	*/
				MutexedQueue<Message*>* outgoingQueue; 		/*	this queue is to be processed by local threads	*/
				MutexedQueue<Message*>* inQueue; 			/*	this queue is to be redirect to app	*/
				MutexedQueue<Message*>* outQueue; 			/*	this queue is to be sent outside	*/
				
				static void* processIncomingMessage(void* args){
					DaemonService* _this = (DaemonService*)args;
			
					Message* message;
					uint8_t* decryptedMessage = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
					size_t decryptedLen;
			
					while (_this->isWorking()){
						if (_this->incomingQueue->peak(&message)){
				
							uint32_t sessionID;
							uint8_t infoByte;
							bool process = true;
					
							printf("incoming message: ");
							printBuffer(message->data, message->len);
							infoByte = message->data[4];
							sessionID = *((uint32_t*)message->data);
					
							if (infoByte & SVC_ENCRYPTED){
								/*	decrypt the message	*/
								if (_this->decryptMessage(message->data+5, message->len-5, decryptedMessage+5, &decryptedLen)){
									//copy first 5 bytes in to decryptedMessage
									memcpy(message->data, decryptedMessage, 5);
									/*	update new size	*/
									decryptedLen = 5 + decryptedLen;														
								}
								else{
									/*	failed to decrypt, do not process this message	*/
									process = false;
								}
							}
							/*
							else: non-encypted messages had been already filtered out
							*/
							if (process){
								if (infoByte & SVC_COMMAND_FRAME){
									/*	process command	*/
									enum SVCCommand cmd = (enum SVCCommand) decryptedMessage[5];
									switch (cmd){
										case SVC_CMD_CHECK_ALIVE:
										case SVC_CMD_REGISTER_APP:								
											/*	do nothing	*/
											break;
									
										case SVC_CMD_CONNECT_STEP1:
											//if (_this->svcRole == SVC_ROLE_SERVER){
											//	/*	read appID 	*/
										
											//}
											break;
									
										case SVC_CMD_CONNECT_STEP2:
											break;
										case SVC_CMD_CONNECT_STEP3:
											break;
										case SVC_CMD_CONNECT_STEP4:
											break;
									}
								}
								else{
									/*	forward data to app	*/
									_this->inQueue->enqueue(new Message(decryptedMessage, decryptedLen));
								}
							}
					
							/*	process finished, remove message from queue	*/
							_this->incomingQueue->dequeue();
						}
					}
			
					delete decryptedMessage;
					//printf("incoming process stopped\n");
					//return;
				}

				static void* processOutgoingMessage(void* args){
					DaemonService* _this = (DaemonService*)args;

					/*	iterate the outgoing queue	*/
					uint8_t* allocBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
					size_t allocSize;
			
					//size_t len;
			
					Message* message;
						
					while (_this->isWorking()){
						if (_this->outgoingQueue->peak(&message)){
							uint32_t sessionID;
							uint8_t infoByte;
					
							printf("outgoing message: \n");
							/*	data or command	*/
							printBuffer(message->data, message->len);
							sessionID = *((uint32_t*)message->data);
							infoByte = message->data[4];					
					
							if (infoByte & SVC_COMMAND_FRAME){
					
								/*	command frame, extract arguments to argv	*/												
						
								uint8_t argc = message->data[6];
								SVCCommandParam* argv[argc];
								uint8_t* pointer = message->data+7;
						
								for (int i=0; i<argc; i++){
									/*	just get pointer, not performing copy	*/
									argv[i] = new SVCCommandParam();
									argv[i]->length = *((uint16_t*)pointer);
									argv[i]->data = pointer + 2;
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
										printf("processing CMD_REGISTER_APP\n");
								
										uint32_t appID;
										ssize_t rs;
										bool newAppAllowed;
										bool serviceRemoved;

										/*	check if app existed	*/								
										appID = *((uint32_t*)(argv[0]->data));
										sessionID = appExisted(appID);
										newAppAllowed = false;
										serviceRemoved = false;								

										/*	wait for alive response from app if sessionID != SVC_DEFAULT_SESSIONID	*/
										if (sessionID != SVC_DEFAULT_SESSIONID && sessionID!=-1){
											//printf("check for app alive\n");
											params.clear();
											prepareCommand(allocBuffer, &allocSize, sessionID, SVC_CMD_CHECK_ALIVE, &params);
											/*	send this to app	*/
											appTableMutex->lock_shared();
											if (appTable[sessionID]!=NULL){
												appTable[sessionID]->inQueue->enqueue(new Message(allocBuffer, allocSize));
												newAppAllowed = !(appTable[sessionID]->signalNotificator.waitCommand(SVC_CMD_CHECK_ALIVE, &params, SVC_SHORT_TIMEOUT));
												if (newAppAllowed){
													/*	remove old service */
													serviceRemoved = true;
												}
												/*
												else: do nothing with this request
												*/
											}
											else{
												newAppAllowed = true;
											}
											//printf("before unlock shared\n");
											appTableMutex->unlock_shared();
											//printf("after unlock shared\n");								
										}
										else
											newAppAllowed = true;											

										if (newAppAllowed){
											if (serviceRemoved){										
												DaemonService * service;
												appTableMutex->lock_shared();
												service = appTable[sessionID];
												appTableMutex->unlock_shared();
												/*	find all sessionID attached to this service	*/
												do{
													sessionID = appExisted(appID);
													if (sessionID != -1){
														/*	remove the reference	*/
														//printf("remove reference %08x for service %08x\n", sessionID, appID);
														appTableMutex->lock();
														appTable[sessionID] = NULL;
														appTableMutex->unlock();
													}
												}
												while (sessionID != -1);
										
												service->stopService();
												delete service;
											}
									
											/*	create new sessionID = hash(appID & rand)	*/
											hash<string> hasher;
											sessionID = (uint32_t)hasher(to_string(appID) + to_string(rand()));

											printf("add new service for: %08x, sessionID %08x\n", appID, sessionID);

											DaemonService* service = new DaemonService(appID);
											service->startService();
											appTableMutex->lock();						
											appTable[sessionID] = service;
											appTableMutex->unlock();
																		
											/*	repsonse sessionID to SVC app	*/
											params.clear();
											params.push_back(new SVCCommandParam(SESSIONID_LENGTH, (uint8_t*) &sessionID));
											params.push_back(new SVCCommandParam(4, argv[1]->data));
											prepareCommand(allocBuffer, &allocSize, sessionID, SVC_CMD_REGISTER_APP, &params);
									
											/*	this service has now its own session ID	*/
											service->inQueue->enqueue(new Message(allocBuffer, allocSize));									
										}
										/*
										else: session ID not exist
										*/
										break;
								
									case SVC_CMD_CONNECT_STEP1:
										printf("processing SVC_CMD_CONNECT_STEP1\n");
										if (!_this->isConnected){	
											//_this->svcRole = *(argv[0]->data);
											//if (_this->svcRole == SVC_ROLE_CLIENT){
												/*	get 4 bytes remote address then connect to server SVC-daemon	*/
												_this->setAddress(*((uint32_t*)(argv[1]->data)));
										
												/*	prepare to send out SVC_CMD_CONNECT_STEP1	*/											
												params.clear();
												/*	append appID	*/
												params.push_back(new SVCCommandParam(4, (uint8_t*) &(_this->appID)));
												/*	add version info	*/
												uint8_t version = _this->svcRole | SVC_VERSION;
												params.push_back(new SVCCommandParam(argv[0]->length, &version));
												params.push_back(new SVCCommandParam(argv[2]->length, argv[2]->data));
												prepareCommand(allocBuffer, &allocSize, SVC_DEFAULT_SESSIONID, SVC_CMD_CONNECT_STEP1, &params);
												/*	send out	*/
												_this->outQueue->enqueue(new Message(allocBuffer, allocSize));
										//	}
											//else{
										//		/*	do nothing - for now	*/
											//}
										}
										/*
										else: connected state require disconnect first
										*/
										break;
								
									case SVC_CMD_CONNECT_STEP2:
										//if (_this->svcRole==SVC_ROLE_SERVER){
											/*	prepare to send out SVC_CMD_CONNECT_STEP2	*/											
											params.clear();
											params.push_back(new SVCCommandParam(4, (uint8_t*) &(_this->appID)));
											/*	add version info	*/
											params.push_back(new SVCCommandParam(argv[0]->length, argv[0]->data));
											params.push_back(new SVCCommandParam(argv[2]->length, argv[2]->data));
											prepareCommand(allocBuffer, &allocSize, SVC_DEFAULT_SESSIONID, SVC_CMD_CONNECT_STEP2, &params);
											/*	send out	*/
											_this->outQueue->enqueue(new Message(allocBuffer, allocSize));
										//}
										/*
										else:	only server sends this command
										*/
										break;
								
									case SVC_CMD_CONNECT_STEP3:
							
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
			
					delete allocBuffer;
					//printf("outgoing process stopped\n");
					//return;
				}
				
			public:
				uint64_t endPointID;
				
				void setAddress(uint32_t address){
					/*	to be changed to AF_HTP	*/
					this->inSock = socket(AF_INET, SOCK_DGRAM, 0);
					
					this->inSockAddress.sin_addr.s_addr = address;
					this->inSockAddress.sin_port = htons(SVC_DAEPORT);			
					this->inSockAddress.sin_family = AF_INET;
						
					connect(this->inSock, (struct sockaddr*) &(this->inSockAddress), sizeof(this->inSockAddress));
				}
				
				ssize_t sendOutside(const Message* message){
					return send(this->inSock, message->data, message->len, 0);
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
		};
		
		
		unordered_map<uint64_t, ServiceEndpoint*> serviceEndpoints;
	
		/*	this is unix domain socket	*/
		string svcClientPath;
		int unSock;
		struct sockaddr_un unSockAddress;
	
		/**/
		pthread_attr_t threadAttrIn;
		pthread_attr_t threadAttrOut;
		pthread_t incomingProcessingThread;
		pthread_t outgoingProcessingThread;
	
		/*	infos concern service status	*/
		uint8_t svcRole;
		bool working;
		bool isConnected;
		
		/*	private functions	*/
		
		ssize_t sendToApp(const Message* message){
			return send(this->unSock, message->data, message->len, 0);
		}
	
	public:	

//		uint32_t appID;		
		
		MutexedQueue<Message*>* incomingQueue; 		/*	this queue is to be processed by local threads	*/
		MutexedQueue<Message*>* outgoingQueue; 		/*	this queue is to be processed by local threads	*/
		MutexedQueue<Message*>* inQueue; 			/*	this queue is to be redirect to app	*/
		MutexedQueue<Message*>* outQueue; 			/*	this queue is to be sent outside	*/
		
		//--	crypto parameters here		--//
		
		
		DaemonService(){
			//--	default state
			this->working = false;
			this->isConnected = false;
			//this->svcRole = SVC_ROLE_UNDEFINED;
			//this->appID = appID;
					
			//--	init queues
			this->incomingQueue = new MutexedQueue<Message*>();
			this->outgoingQueue = new MutexedQueue<Message*>();
			this->inQueue = new MutexedQueue<Message*>();
			this->outQueue = new MutexedQueue<Message*>();
			
			//this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
			/*	create unix socket and connect to send packet	*/
			memset(&this->unSockAddress, 0, sizeof(this->unSockAddress));
			this->unSockAddress.sun_family = AF_LOCAL;
			memcpy(this->unSockAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());					
			this->unSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
			connect(this->unSock, (struct sockaddr*) &this->unSockAddress, sizeof(this->unSockAddress));
		}
		
		void startService(){
			//this->workingMutex->lock();
			this->working = true;
			//this->workingMutex->unlock();
			pthread_attr_init(&threadAttrIn);
			pthread_attr_init(&threadAttrOut);
			pthread_create(&incomingProcessingThread, &threadAttrIn, processIncomingMessage, this);
			pthread_create(&outgoingProcessingThread, &threadAttrOut, processOutgoingMessage, this);
		}
		
		void stopService(){			
			//printf("stop service called for %08x\n", this->appID);
			//this->workingMutex->lock();
			//printf("lock working mutex, reader %d, writer %d\n", this->workingMutex->readerPresence, this->workingMutex->writerPresence);
			this->working = false;
			//printf("this->working set to false\n");
			//printf("try unlock working mutex\n");
			//this->workingMutex->unlock();
			//printf("unlock working mutex\n");
			
			//printf("unlock working mutex\n");
			pthread_join(incomingProcessingThread, NULL);
			pthread_join(outgoingProcessingThread, NULL);
			//printf("child threads joined\n");
		}
		
		bool isWorking(){
			bool rs;
			//this->workingMutex->lock_shared();
			//printf("isworking lock_shared, reader %d, writer %d\n", this->workingMutex->readerPresence, this->workingMutex->writerPresence);
			rs = this->working;
			//if (!rs){
				//printf("service %08x isWoking = false\n", this->appID);
			//}
			//this->workingMutex->unlock_shared();
			return rs;
		}
		
		int receiveMessageFromOutside(Message* message){
			this->incomingQueue->enqueue(message);
			/*	check whether one thread is waiting for this message*/
			//checkNotificationList(message);
		}
		
		int receiveMessageFromApp(Message* message){			
			this->outgoingQueue->enqueue(message);
			/*	check whether one thread is waiting for this message*/
			//checkNotificationList(message);
		}
		
		/*void checkNotificationList(Message* message){
			uint8_t infoByte = message->data[5];
			if (infoByte & SVC_COMMAND_FRAME){
				enum SVCCommand cmd = (enum SVCCommand)message->data[6];
				switch(cmd){
					case SVC_CMD_CONNECT_STEP1:
					case SVC_CMD_CONNECT_STEP2:
					case SVC_CMD_CONNECT_STEP3:
					case SVC_CMD_CONNECT_STEP4:
				}
				SVCDataReceiveNotificator* notificator = signalNotificator.getNotificator(cmd);
				//	check if the received command matches handler's command	
				if (notificator != NULL){
					//	perform callback	
					notificator->handler(message->data, message->len, notificator);
					//	remove handler	
					signalNotificator.removeNotificator(cmd);
				}				
				//else: there is no notificator for this command
							
			}			
			//else: no notification for data frame			
		}*/
		
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
		}			
	
		~DaemonService(){
			//printf("destructor called, remove queues %08x\n", this->appID);
			/*	these queues are warrantied to dequeue all left message in destructor */
			delete this->outgoingQueue;
			delete this->incomingQueue;
			delete this->inQueue;
			delete this->outQueue;
			//delete this->workingMutex;
			printf("service for appID %08x stopped\n", this->appID);
		}
};


/*
//check app existed, return sessionID
uint32_t appExisted(uint32_t appID){
	appTableMutex->lock_shared();
	for (auto& it : appTable){
		DaemonService* service = it.second;
		uint32_t sessionID = it.first;
		if (service != NULL){
			if (service->appID == appID){
				appTableMutex->unlock_shared();
				return sessionID;
			}
		}
	}
	appTableMutex->unlock_shared();
	return -1;
}*/

DaemonService* getServiceByAddress(uint32_t address){
	serviceTableMutex->lock_shared();
	for (auto&it : serviceTable){
		DaemonService* service = it.second;
		if (service!=NULL){
			if (service->getAddress() == address){
				serviceTableMutex->unlock_shared();
				return service;
			}
		}
	}
	serviceTableMutex->unlock_shared();
	return NULL;
}

DaemonService* getServiceBySessionID(uint32_t sessionID){
	serviceTableMutex->lock_shared();
	for (auto&it : serviceTable){
		DaemonService* service = it.second;
		if (service!=NULL){
			if (service->sessionID == sessionID){
				serviceTableMutex->unlock_shared();
				return service;
			}
		}
	}
	serviceTableMutex->unlock_shared();
	return NULL;
}

void signal_handler(int sig){
	if (sig == SIGINT){
		printf("SIGINT caught, stopping daemon\n");
		/*	stop main threads	*/
		working = false;
	}
}

class DaemonEndPoint{

	MutexedQueue<Message*>* incomingQueue; 		/*	this queue is to be processed by local threads	*/
	MutexedQueue<Message*>* outgoingQueue; 		/*	this queue is to be processed by local threads	*/
	MutexedQueue<Message*>* inQueue; 			/*	this queue is to be redirect to app	*/
	MutexedQueue<Message*>* outQueue; 			/*	this queue is to be sent outside	*/
	
	pthread_attr_t threadAttr;
	pthread_t processIncomingThread;
	pthread_t processOutgoingThread;
	pthread_t sendInThread;
	pthread_t sendOutThread;
	bool working;
	
	DaemonService* daemonService;
	uint64_t endPointID;
	uint32_t appID;	
	
	struct sockaddr_un unSockAddr;
	int unSock;
			
	static void* processingIncomingMessage(void* args){
		DaemonEndPoint* _this = (DaemonEndPoint*)args;
		Message* message;
		uint8_t* decryptedBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
		size_t decryptedLen;
		
		while (_this->working){
			if (_this->incomingQueue->peak(message)){
				uint8_t infoByte = message->data[ENDPOINTID_LENGTH];
				bool process = true;
				if (infoByte & SVC_ENCRYPTED){
					if (_this->daemonService->decryptMessage(message->data + ENDPOINTID_LENGTH+1, message->len - ENDPOINTID_LENGTH-1, decryptedBuffer, &decryptedLen)){
						//--	replace the encrypted content with decrypted content
						memcpy(message->data+ENDPOINTID_LENGTH+1, decryptedBuffer, decryptedLen);
					}
					else{
						//--	failed to decrypt
						process = false;
					}
				}
				else{				
					if (infoByte & SVC_COMMAND_FRAME){
						enum SVCCommand cmd = message->data[ENDPOINTID_LENGTH+1];
						process = !isEncryptedCommand(cmd);
					}
					else{
						//--	ignore unencrypted data packet
						process = false;
					}
				}
				
				//--	packet checking done, start processing
				if (process){
					if (infoByte & SVC_COMMAND_FRAME){
						enum SVCCommand cmd = message->data[ENDPOINTID_LENGTH+1];
						switch (cmd){
							
							
							default:
								break;
						}
					}
					else{
						//--	forward data packet to app
						_this->inQueue->enqueue(new Message(message->data, message->len));
					}
				}
				//--	remove message from this queue
				_this->incomingQueue->dequeue();
			}
			//--	else: queue is empty
		}
	}
	
	static void* processingOutgoingMessage(void* args){
		DaemonEndPoint* _this = (DaemonEndPoint*)args;	
	}
	
	static void* sendPacketToApp(void* args){
		DaemonEndPoint* _this = (DaemonEndPoint*)args;
		Message* message;

		while (_this->working){			
			if (_this->inQueue->peak(message)){
				send(this->unSock, message->data, message->len, 0);
			}
		}
	}
	
	static void* sendPacketOutside(void* args){
		DaemonEndPoint* _this = (DaemonEndPoint*)args;	
		Message* message;
		uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
		size_t bufferLen;
		
		uint8_t* encryptedBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
		size_t encryptedLen;
				
		while (_this->working){
			Message* message;
			if (_this->outQueue->peak(message)){
				//--	append the sessionID and encrypt the message if required
				memcpy(buffer, (uint8_t*) &_this->daemonService->sessionID, SESSIONID_LENGTH);
				uint8_t infoByte = message->data[ENDPOINTID_LENGTH];
				bool mustEncrypted = true;
				if (infoByte & SVC_COMMAND_FRAME){
					enum SVCCommand cmd = message->data[ENDPOINTID_LENGTH + 1];
					mustEncrypted = isEncryptedCommand(cmd);
				}
				//--	else: data fram must always be encrypted
				if (mustEncrypted){
					buffer[ENDPOINTID_LENGTH] = infoByte | SVC_ENCRYPTED;
					_this->daemonService->encryptMessage(message->data + ENDPOINTID_LENGTH + 1, message->len - ENDPOINTID_LENGTH - 1, encryptedBuffer, &encryptedLen);
					memcpy(buffer + SESSIONID_LENGTH + 1, encryptedBuffer, encryptedLen);
					bufferLen = SESSIONID_LENGTH + ENDPOINTID_LENGTH + 1 + encryptedLen;
				}
				else{
					buffer[ENDPOINTID_LENGTH] = infoByte;
					memcpy(buffer + SESSIONID_LENGTH +1, message->data, message->len);
					bufferLen = SESSIONID_LENGTH + message->len;
				}
				
				_this->daemonService->sendData(buffer, bufferLen, 0);
				
				//--	remove the message from queue
				_this->outQueue->dequeue();
			}
			//--	else: queue is empty
		}
	}

	public:	
		SignalNotificator* signalNotificator;
		MutexedQueue<Message*>* outDataQueue;
		
		DaemonEndPoint(DaemonService* daemonService, uint64_t endPointID, uint32_t appID){
			this->daemonService = daemonService;
			this->endPointID = endPointID;
			this->addID = appID;
			
			//--	create unix socket and connect to app
			string clientPath = SVC_CLIENT_PATH_PREFIX + to_string(this->appID);
			memset(&this->unSockAddr, 0, sizeof(this->unSockAddr));
			this->unSockAddr.sun_family = AF_LOCAL;
			memcpy(this->unSockAddr.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());					
			this->unSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
			connect(this->unSock, (struct sockaddr*) &this->unSockAddr, sizeof(this->unSockAddr));
			
			//--	start the threads
		}
		
		~DaemonEndPoint(){
		}
		
};

class DaemonService{
	
	friend class DaemonEndPoint;
	
	uint32_t address;
	uint32_t sessionID;
	bool isConnected;
	
	struct sockaddr_in sockAddr;
	int sock;
	
	unordered_map<uint64_t, DaemonEndPoint*> endPoints;
	shared_mutex* endPointsMutex;		
	
	//--	CRYPTO VARIABLES	--//
			
	int encryptMessage(const uint8_t* plainMessage, const size_t* plainLen, uint8_t* encryptedMessage, size_t* encryptedLen){
		
	}
		
	int decryptMessage(const uint8_t* encryptedMessage, size_t encryptedLen, uint8_t* plainMessage, size_t* plainLen){
		
	}
	
	void sendData(const uint8_t* buffer, size_t bufferLen){
		send(this->sock, buffer, bufferLen, 0);
	}
	
	public:
		
		DaemonService(const struct sockaddr_in* sockaddr, size_t sockLen){
			this->isConnected = false;
			this->sessionID = SVC_DEFAULT_SESSIONID;
			this->endPointsMutex = new shared_mutex();
			this->address = 0x00000000;
			
			//--	create socket to sendout data
			//--TODO: to be changed to htp
			memcpy(&this->sockAddr, sockaddr, sockLen);
			this->sock = socket(AF_INET, SOCK_DGRAM, 0);
			connect(this->sock, (struct sockaddr*) &this->sockAddr, sockLen);
		}
	
		~DaemonService(){
		}
		
		void setAddress(uint32_t address){
			this->address = address;
		}
		
		uint32_t getAddress(){
			return this->address;
		}
	
		DaemonEndPoint* addDaemonEndPoint(uint64_t endPointID, uint32_t appID){
			DaemonEndPoint* endPoint = new DaemonEndPoint(this, endPointID, appID);
			this->endPointsMutex->lock();
			this->endPoints[endPointID] = endPoint;
			this->endPointsMutex->unlock();
			return endPoint;
		}
		
		DaemonEndPoint* getDaemonEndPoint(uint64_t endPointID){
			DaemonEndPoint* endPoint;
			this->endPointsMutex->lock_shared();
			endPoint = endPoints[endPointID];
			this->endPointsMutex->unlock_shared();
			return endPoint;
		}		
};

void* unixReadingLoop(void* args){
	
	int byteRead;

	while (working){	
		do{
			byteRead = recv(daemonUnSocket, unixReceiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		//--	process message. message is stored in unixReceiveBuffer
		if (byteRead>0){			
			
			//--	check if we have service for this endPointID
			uint64_t endPointID = *((uint64_t*)unixReceiveBuffer);			
			uint8_t infoByte = unixReceiveBuffer[ENDPOINTID_LENGTH];
			
			DaemonService* service;
			serviceTableMutex->lock_shared();
			service = serviceTable[endPointID];
			serviceTableMutex->unlock_shared();
			
			if (service==NULL){
				if (infoByte & SVC_COMMAND_FRAME){
					enum SVCCommand cmd = (enum SVCCommand)unixReceiveBuffer[ENDPOINTID_LENGTH + 1];
					if (cmd == SVC_CMD_CONNECT_STEP1){
						extractParams(buffer + ENDPOINTID_LENGTH + 2, &params);
						//--	check for service if connect to the same address
						DaemonService* service;
						uint32_t address = *((uint32_t*)(params[0]->data));
						if (address!=0){
							service = getServiceByAddress(address);
							if (service == NULL){						
								//--TODO:	to be changed to htp
								struct sockaddr_in sockAddr;
								size_t sockLen = sizeof(sockAddr);							
								sockAddr.sin_family = AF_INET;
								sockAddr.sin_port = htons(SVC_DAEPORT);
								sockAddr.sin_addr.s_addr = address;
								service = new DaemonService(&sockAddr, sockLen);
								service->setAddress(address);
								//--	register service with endPointID
								serviceTableMutex->lock();
								serviceTable[endPointID] = service;
								serviceTableMutex->unlock();
							}
							//--	else: use this service
							DaemonEndPoint* endPoint = service->addDaemonEndPoint(endPointID, *((uint32_t*)(params[1]->data)));
							endPoint->outgoingQueue->enqueue(new Message(unixReceiveBuffer, byteRead));						
							clearParams(&params);
						}
						//--	else: incorrect address
					}
					//--	else: other commands not allows without service
				}
				//--	else: data frame not allowed without service		
			else{				
				DaemonEndPoint* endPoint;
				endPoint = service->getDaemonEndPoint(endPointID);
				endPoint->outgoingQueue->enqueue(new Message(unixReceiveBuffer, byteRead));
			}
		}
		//--	else: read error
	}
	printf("Exit unix reading loop\n");
}

void* htpReadingLoop(void* args){
	
	int byteRead;
	struct sockaddr_in sockAddr;
	size_t sockLen = sizeof(sockAddr);
	vector<SVCCommandParam*> params;

	while (working){	
		do{
			byteRead = recvfrom(daemonInSocket, htpReceiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT, (struct sockaddr*) &sockAddr, &sockLen);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		//--	process message. message is stored in unixReceiveBuffer
		if (byteRead>0){			
			
			//--	check if we have service for this endPointID
			uint32_t sessionID = ((uint32_t*)htpReceiveBuffer);
			uint64_t endPointID = *((uint64_t*)(htpReceiveBuffer+SESSIONID_LENGTH));		
			uint8_t infoByte = htpReceiveBuffer[SESSIONID_LENGTH + ENDPOINTID_LENGTH];
			
			DaemonService* service;
			serviceTableMutex->lock_shared();
			service = serviceTable[endPointID];
			serviceTableMutex->unlock_shared();
			
			if (service==NULL){
				if (infoByte & SVC_COMMAND_FRAME){				
					enum SVCCommand cmd = (enum SVCCommand)htpReceiveBuffer[SESSIONID_LENGTH + ENDPOINTID_LENGTH + 1];
					if (cmd == SVC_CMD_CONNECT_STEP1){
						extractParams(htpReceiveBuffer + SESSIONID_LENGTH + ENDPOINTID_LENGTH + 3, &params);
						//--	check if we have service for this sessionID
						service = getServiceBySessionID(sessionID);
						if (service==NULL){						
							//--	create new DaemonService
							service = new DaemonService(&sockAddr, sockLen);
							//--	register this service with endPointID
							serviceTableMutex->lock();
							serviceTable[endPointID] = service;
							serviceTableMutex->unlock();
						}
						//--	else: use this service
						DaemonEndPoint* endPoint = service->addDaemonEndPoint(endPointID, *((uint32_t*)(params[0]->data)));
						endPoint->incomingQueue->enqueue(new Message(htpReceiveBuffer + SESSIONID_LENGTH, byteRead-SESSIONID_LENGTH));
						clearParams(&params);
					}
					//--	else: other commands not allowed without service
				}
				//--	else: data frame not allowed without service
			}
			else{
				//--	service existed with endPointID, remove sessionID before forward
				DaemonEndPoint* endPoint = service->getDaemonEndPoint(endPointID);
				endPoint->incomingQueue->enqueue(new Message(htpReceiveBuffer + SESSIONID_LENGTH, byteRead-SESSIONID_LENGTH));
			}
		}
		//--	else: read error
	}
	printf("Exit htp reading loop\n");
}

/*

void* htpRedirectingLoop(void* args){
	
	DaemonService* service;
	
	while (working){
		appTableMutex->lock_shared();
  		for (auto& it : appTable){
  			service = it.second;
  			if (service!=NULL){ 							
				if (service->isWorking()){
					service->sendPacketOutside();
				}
	    	}
    	}
    	appTableMutex->unlock_shared();
    }
    printf("Exit htp redirecting loop\n");
}


void* unixRedirectingLoop(void* args){
	
	DaemonService* service;
		
	while(working){
		appTableMutex->lock_shared();			
		for (auto& it : appTable){				
			service = it.second;
			if (service!=NULL){
				if (service->isWorking()){					
					service->sendPacketToApp();
				}
			}
		}
		appTableMutex->unlock_shared();
	}
	printf("Exit unix redirecting loop\n");
}

*/

int main(int argc, char** argv){

	htpReceiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	unixReceiveBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	serviceTableMutex = new shared_mutex();
	
	//--	check if the daemon is already existed
	int rs = unlink(SVC_DAEMON_PATH.c_str());
	if (!(rs==0 || (rs==-1 && errno==ENOENT))){	
		errorString = SVC_ERROR_NAME_EXISTED;
		goto errorInit;
	}

	//--	create a daemon server unix socket
	memset(&daemonSockUnAddress, 0, sizeof(daemonSockUnAddress));
	daemonSockUnAddress.sun_family = AF_LOCAL;
	memcpy(daemonSockUnAddress.sun_path, SVC_DAEMON_PATH.c_str(), SVC_DAEMON_PATH.size());		
	//--	bind the socket
	daemonUnSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);	
	if (bind(daemonUnSocket, (struct sockaddr*) &daemonSockUnAddress, sizeof(daemonSockUnAddress)) == -1) {		
		errorString = SVC_ERROR_BINDING;
        goto errorInit;
    }
    
    //--TODO:	TO BE CHANGED TO HTP
    //--	create htp socket
    memset(&daemonSockInAddress, 0, sizeof(daemonSockInAddress));
    daemonSockInAddress.sin_family = AF_INET;
    daemonSockInAddress.sin_port = htons(SVC_DAEPORT);
	daemonSockInAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //--	bind this socket to localhost
    daemonInSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (bind(daemonInSocket, (struct sockaddr*) &daemonSockInAddress, sizeof(daemonSockInAddress))){
    	errorString = SVC_ERROR_BINDING;
    	goto errorInit;
    }
    
    //--	set thread signal mask
    sigset_t sigset;
    sigemptyset(&sigset);    
    sigaddset(&sigset, SVC_ACQUIRED_SIGNAL);
    sigaddset(&sigset, SVC_TIMEOUT_SIGNAL);
    sigaddset(&sigset, SVC_SHARED_MUTEX_SIGNAL);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
    
    //--	handle signals
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);
	
	/*
		add default DaemonService to handle non-encrypted request
		we can use SVC_DEFAULT_SESSIONID for init DaemonService
	*/
	/*appTableMutex->lock();
	appTable[SVC_DEFAULT_SESSIONID] = new DaemonService(SVC_DEFAULT_SESSIONID);
	appTable[SVC_DEFAULT_SESSIONID]->startService();
	appTableMutex->unlock();
    */
    //--	create a thread to read from unix domain socket
    working = true;
    pthread_attr_init(&unixReadingThreadAttr);
    pthread_create(&unixReadingThread, &unixReadingThreadAttr, unixReadingLoop, NULL);
      
	//--	create a thread to read from htp socket
	pthread_attr_init(&htpReadingThreadAttr);	
	pthread_create(&htpReadingThread, &htpReadingThreadAttr, htpReadingLoop, NULL);
	
	/*
	//6.	create a thread to process and redirect incoming messages
	pthread_attr_init(&htpProcessingThreadAttr);	
	pthread_create(&htpProcessingThread, &htpProcessingThreadAttr, htpRedirectingLoop, NULL);

	//7.	create a thread to process and redirect outgoing messages
   	pthread_attr_init(&unixProcessingThreadAttr);	
	pthread_create(&unixProcessingThread, &unixProcessingThreadAttr, unixRedirectingLoop, NULL);
	*/
    goto initSuccess;
    
    errorInit:
		delete unixReceiveBuffer;
		delete htpReceiveBuffer;
    	cout<<errorString<<endl;
    	throw errorString;
    	
    initSuccess:
		//--	POST-SUCCESS JOBS	--//
    	printf("SVC daemon is running...\n");
    	pthread_join(unixReadingThread, NULL);   
    	//pthread_join(unixProcessingThread, NULL);
    	pthread_join(htpReadingThread, NULL);
    	//pthread_join(htpProcessingThread, NULL);    	 	
    	/*	wait here until catch SIGINT	*/
    	
    	//--	DO CLEANING UP BEFORE EXIT	--//
    	
    	//--	remove all DaemonService instances

		/*
		for (auto& it : appTable){
			DaemonService* service = it.second;	
			if (service!=NULL){
				//--	remove all reference of this service
				uint32_t appID = service->appID;
				uint32_t sessionID;
				do{
					sessionID = appExisted(appID);
					if (sessionID != -1){
						//--	remove the reference
						///printf("remove reference %08x for service %08x\n", sessionID, appID);
						appTableMutex->lock();
						appTable[sessionID] = NULL;
						appTableMutex->unlock();
					}
				}
				while (sessionID!=-1);
				
				service->stopService();
				delete service;
			}
		}
		*/
		
		delete serviceTableMutex;
		delete unixReceiveBuffer;
		delete htpReceiveBuffer;
    	unlink(SVC_DAEMON_PATH.c_str());    	
    	printf("SVC daemon stopped.\n");
}


