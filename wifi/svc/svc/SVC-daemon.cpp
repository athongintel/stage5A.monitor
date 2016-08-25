#include "SVC-daemon.h"

//--	class DaemonService
	
int DaemonService::encryptMessage(const uint8_t* plainMessage, size_t plainLen, uint8_t* encryptedMessage, size_t* encryptedLen){
	
}
	
int DaemonService::decryptMessage(const uint8_t* encryptedMessage, size_t encryptedLen, uint8_t* plainMessage, size_t* plainLen){
	
}
	
void DaemonService::sendData(const uint8_t* buffer, size_t bufferLen){
	send(this->sock, buffer, bufferLen, 0);
}
	
	
DaemonService::DaemonService(const struct sockaddr_in* sockaddr, socklen_t sockLen){
	this->isConnected = false;
	this->sessionID = SVC_DEFAULT_SESSIONID;
	this->endPointsMutex = new shared_mutex();
	this->address = 0x00000000;
	
	//--	create socket to sendout data
	//--TODO: to be changed to htp
	memcpy(&this->sockAddr, sockaddr, sockLen);
	this->sock = socket(AF_INET, SOCK_DGRAM, 0);
	connect(this->sock, (struct sockaddr*) &this->sockAddr, sockLen);
	this->working = true;
	printf("service started with address: ");
	printBuffer((uint8_t*) &this->sockAddr, sockLen);
}
		
bool DaemonService::isWorking(){
	return working;
}

void DaemonService::stopWorking(){
	this->isConnected = false;
	this->working = false;
	//--	stop all remaining endpoint
	this->endPointsMutex->lock();			
	for (auto& it : this->endPoints){
		uint64_t endPointID = it.first;
		DaemonEndPoint* endPoint = it.second;
		if (endPoint!=NULL){
			endPoint->stopWorking();
			delete endPoint;
			this->endPoints[endPointID] = NULL;
		}
	}
	this->endPointsMutex->unlock();
	
	//--	remove all references to current service
	serviceTableMutex->lock();
	for (auto& it : this->endPoints){
		uint64_t endPointID = it.first;
		serviceTable[endPointID] = NULL;
	}
	serviceTableMutex->unlock();
}

DaemonService::~DaemonService(){
	//--	TODO:	remove crypto variables
	delete endPointsMutex;
}


DaemonEndPoint* DaemonService::addDaemonEndPoint(uint64_t endPointID, uint32_t appID){
	DaemonEndPoint* endPoint = new DaemonEndPoint(this, endPointID, appID);
	this->endPointsMutex->lock();
	this->endPoints[endPointID] = endPoint;
	this->endPointsMutex->unlock();
	return endPoint;
}

DaemonEndPoint* DaemonService::getDaemonEndPoint(uint64_t endPointID){
	DaemonEndPoint* endPoint;
	this->endPointsMutex->lock_shared();
	endPoint = endPoints[endPointID];
	this->endPointsMutex->unlock_shared();
	return endPoint;
}

//--	class DaemonEndPoint

void* DaemonEndPoint::processingIncomingMessage(void* args){
	DaemonEndPoint* _this = (DaemonEndPoint*)args;
	Message* message;
	uint8_t* decryptedBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t decryptedLen;

	while (_this->working){
		if (_this->incomingQueue->peak(&message)){
			printf("can peak: ");
			printBuffer(message->data, message->len);
			uint8_t infoByte = message->data[ENDPOINTID_LENGTH];
			printf("info byte: %02x\n", infoByte);
			bool process = true;
			if (infoByte & SVC_ENCRYPTED){
				if (_this->daemonService->decryptMessage(message->data + ENDPOINTID_LENGTH+1, message->len - ENDPOINTID_LENGTH-1, decryptedBuffer, &decryptedLen)){
					//--	replace the encrypted content with decrypted content
					memcpy(message->data+ENDPOINTID_LENGTH + 1, decryptedBuffer, decryptedLen);
					message->len = ENDPOINTID_LENGTH + 1 + decryptedLen;
				}
				else{
					//--	failed to decrypt
					process = false;
				}
			}
			else{				
				if (infoByte & SVC_COMMAND_FRAME){
					enum SVCCommand cmd = (enum SVCCommand)(message->data[ENDPOINTID_LENGTH+1]);
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
					enum SVCCommand cmd = (enum SVCCommand)(message->data[ENDPOINTID_LENGTH + 1]);
					switch (cmd){
						case SVC_CMD_CONNECT_STEP1:								
							//--	TODO:	extract key exchange 1 info
						
							//--	forward to SVC connection queue
							_this->inQueue->enqueue(message);
							_this->incomingQueue->dequeue();
							break;
						
						case SVC_CMD_CONNECT_STEP2:
							//--	TODO:	extract key exchange 2 info
						
							//--	forward
							_this->inQueue->enqueue(message);
							_this->incomingQueue->dequeue();
							break;
													
						case SVC_CMD_CONNECT_STEP3:
							//--	TODO:	extract key exchange 3 info
														
							//--	no forward, remove message
							delete _this->incomingQueue->dequeue();
							break;
					
						case SVC_CMD_CONNECT_STEP4:
							//--	forward
							_this->inQueue->enqueue(message);
							_this->incomingQueue->dequeue();
							break;
						
						default:
							//--	remove the message
							delete _this->incomingQueue->dequeue();
							break;
					}
				}
				else{
					//--	forward data packet to app
					_this->inQueue->enqueue(message);
					_this->incomingQueue->dequeue();
				}
			}
			else{
				//--	remove for not being valid
				delete _this->incomingQueue->dequeue();
			}				
		}
		//--	else: queue is empty
	}
}

void* DaemonEndPoint::processingOutgoingMessage(void* args){
	DaemonEndPoint* _this = (DaemonEndPoint*)args;
	Message* message;
	Message* tmpMessage;

	while (_this->working){		
		if (_this->outgoingQueue->peak(&message)){
			printf("can peak: ");
			printBuffer(message->data, message->len);
			uint8_t infoByte = message->data[ENDPOINTID_LENGTH];
			printf("info byte: %02x\n", infoByte);
			if (infoByte & SVC_COMMAND_FRAME){
				enum SVCCommand cmd = (enum SVCCommand) message->data[ENDPOINTID_LENGTH + 1];
				switch (cmd){
					case SVC_CMD_CONNECT_STEP1:
						printf("processing SVC_CMD_CONNECT_STEP1\n");						
						//--	remove the address param (1)
						tmpMessage = new Message(message->data, message->len - ((2 + 4)*1));
						tmpMessage->data[ENDPOINTID_LENGTH + 2]--;
						//--	TODO:	add key exchange step 1
						_this->outQueue->enqueue(tmpMessage);
						delete _this->outgoingQueue->dequeue();
						break;
			
					case SVC_CMD_CONNECT_STEP2:
						//--	TODO:	add key exchange step 2
						_this->outQueue->enqueue(message);
						_this->outgoingQueue->dequeue();
						break;
					
					case SVC_CMD_CONNECT_STEP3:
						//--	TODO:	add key exchange step 3
						_this->outQueue->enqueue(message);
						_this->outgoingQueue->dequeue();
						break;
				
					case SVC_CMD_CONNECT_STEP4:
						//--	this will be encrypted later
						_this->outQueue->enqueue(message);
						_this->outgoingQueue->dequeue();
				
					default:
						break;
				}
			}
			else{
				_this->outQueue->enqueue(message);
				_this->outgoingQueue->dequeue();
			}
		}
	}
}

void* DaemonEndPoint::sendPacketToApp(void* args){
	DaemonEndPoint* _this = (DaemonEndPoint*)args;
	Message* message;

	while (_this->working){			
		if (_this->inQueue->peak(&message)){
			send(_this->unSock, message->data, message->len, 0);
			printf("to app: ");
			printBuffer(message->data, message->len);
			//--	remove the message from queue
			delete _this->inQueue->dequeue();				
		}
	}
}

void* DaemonEndPoint::sendPacketOutside(void* args){
	DaemonEndPoint* _this = (DaemonEndPoint*)args;	
	Message* message;
	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t bufferLen;

	uint8_t* encryptedBuffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t encryptedLen;
	
	while (_this->working){
		
		if (_this->outQueue->peak(&message)){
			printf("peak inside outQueue: ");
			printBuffer(message->data, message->len);
			//--	append the sessionID and encrypt the message if required		
			uint8_t infoByte = message->data[ENDPOINTID_LENGTH];
			//printf("info byte: %02x\n", infoByte);
			bool mustEncrypted = true;
			if (infoByte & SVC_COMMAND_FRAME){
				enum SVCCommand cmd = (enum SVCCommand)(message->data[ENDPOINTID_LENGTH + 1]);
				mustEncrypted = isEncryptedCommand(cmd);
				//printf("must encrypted: %d\n", mustEncrypted);
			}
			//--	else: data frame must always be encrypted
			
			if (mustEncrypted){				
				_this->daemonService->encryptMessage(message->data + ENDPOINTID_LENGTH + 1, message->len - ENDPOINTID_LENGTH - 1, encryptedBuffer, &encryptedLen);
				//printf("replace encrypted data\n");
				memcpy(message->data + ENDPOINTID_LENGTH + 1, encryptedBuffer, encryptedLen);
				message->len = 	ENDPOINTID_LENGTH + 1 + encryptedLen;
			}
			//--	not to be encrypted
			bufferLen = SESSIONID_LENGTH + message->len;			
			memcpy(buffer, (uint8_t*) &_this->daemonService->sessionID, SESSIONID_LENGTH);			
			memcpy(buffer + SESSIONID_LENGTH, message->data, message->len);

			_this->daemonService->sendData(buffer, bufferLen + SESSIONID_LENGTH);
		
			printf("to outside: ");
			printBuffer(buffer, bufferLen);
			//--	remove the message from queue
			delete _this->outQueue->dequeue();
		}
		//--	else: queue is empty
	}
}

DaemonEndPoint::DaemonEndPoint(DaemonService* daemonService, uint64_t endPointID, uint32_t appID){
	this->daemonService = daemonService;
	this->endPointID = endPointID;
	this->appID = appID;
	
	//--	init queues
	this->incomingQueue = new MutexedQueue<Message*>();
	this->outgoingQueue = new MutexedQueue<Message*>();
	this->inQueue = new MutexedQueue<Message*>();
	this->outQueue = new MutexedQueue<Message*>();
	
	//--	create unix socket and connect to app
	string clientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);
	memset(&unSockAddr, 0, sizeof(unSockAddr));
	unSockAddr.sun_family = AF_LOCAL;
	memcpy(unSockAddr.sun_path, clientPath.c_str(), clientPath.size());
	unSock = socket(AF_LOCAL, SOCK_DGRAM, 0);
	connect(unSock, (struct sockaddr*) &unSockAddr, sizeof(unSockAddr));
	
	//--	start the threads
	this->working = true;
	pthread_attr_init(&threadAttr);
	pthread_create(&processIncomingThread, &threadAttr, processingIncomingMessage, this);
	pthread_create(&processOutgoingThread, &threadAttr, processingOutgoingMessage, this);
	pthread_create(&sendInThread, &threadAttr, sendPacketToApp, this);
	pthread_create(&sendOutThread, &threadAttr, sendPacketOutside, this);
	
	printf("endpoint %016x for appID %08x started\n", endPointID, appID);
}

void DaemonEndPoint::stopWorking(){
	working = false;
	pthread_join(processIncomingThread, NULL);
	pthread_join(processOutgoingThread, NULL);
	pthread_join(sendInThread, NULL);
	pthread_join(sendOutThread, NULL);
}

DaemonEndPoint::~DaemonEndPoint(){

	while (incomingQueue->notEmpty()) delete incomingQueue->dequeue();
	while (outgoingQueue->notEmpty()) delete outgoingQueue->dequeue();
	while (inQueue->notEmpty()) delete inQueue->dequeue();
	while (outQueue->notEmpty()) delete outQueue->dequeue();
	
	delete incomingQueue;
	delete outgoingQueue;
	delete inQueue;
	delete outQueue;
	
	printf("endpoint %16x for appID %08x stopped\n", endPointID, appID);
}		

//--	HELPER FUNCTIONS	--//
DaemonService* getServiceByAddress(uint32_t address){
	serviceTableMutex->lock_shared();
	for (auto&it : serviceTable){
		DaemonService* service = it.second;
		if (service!=NULL){
			if (service->address == address){
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
//-----------------------------------//

void* unixReadingLoop(void* args){
	
	int byteRead;
	vector<SVCCommandParam*> params;

	while (working){	
		do{
			byteRead = recv(daemonUnSocket, unixReceiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		//--	process message. message is stored in unixReceiveBuffer
		if (byteRead>0){			
			
			printf("read from unix: ");
			printBuffer(unixReceiveBuffer, byteRead);
			
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
						extractParams(unixReceiveBuffer + ENDPOINTID_LENGTH + 2, &params);
						//--	check for service if connect to the same address
						DaemonService* service;
						uint32_t address = *((uint32_t*)(params[2]->data));
						printf("address: %08x\n", address);
						if (address!=0){
							service = getServiceByAddress(address);
							if (service == NULL){						
								//--TODO:	to be changed to htp
								struct sockaddr_in sockAddr;
								size_t sockLen = sizeof(sockAddr);							
								sockAddr.sin_family = AF_INET;
								sockAddr.sin_port = htons(SVC_DAEPORT);
								sockAddr.sin_addr.s_addr = address;
								printf("address order: ");
								printBuffer((uint8_t*)&sockAddr, sockLen);
								service = new DaemonService(&sockAddr, sockLen);
								service->address = address;
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
			}		
			else{
				if (service->isWorking()){
					DaemonEndPoint* endPoint;
					endPoint = service->getDaemonEndPoint(endPointID);
					endPoint->outgoingQueue->enqueue(new Message(unixReceiveBuffer, byteRead));			
				}
				//--	else: current service is not working
			}
		}
		//--	else: read error		
	}
	printf("Exit unix reading loop\n");
}

void* htpReadingLoop(void* args){
	
	int byteRead;
	struct sockaddr_in sockAddr;
	socklen_t sockLen = sizeof(sockAddr);
	vector<SVCCommandParam*> params;

	while (working){	
		do{
			byteRead = recvfrom(daemonInSocket, htpReceiveBuffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT, (struct sockaddr*) &sockAddr, &sockLen);
		}
		while((byteRead==-1 && (errno==EAGAIN || errno==EWOULDBLOCK)) && working);		
		
		//--	process message. message is stored in unixReceiveBuffer
		if (byteRead>0){			
			
			printf("read from htp: ");
			printBuffer(htpReceiveBuffer, byteRead);
			
			//--	check if we have service for this endPointID
			uint32_t sessionID = *((uint32_t*)htpReceiveBuffer);
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
						extractParams(htpReceiveBuffer + SESSIONID_LENGTH + ENDPOINTID_LENGTH + 2, &params);
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
						DaemonEndPoint* endPoint = service->addDaemonEndPoint(endPointID, *((uint32_t*)(params[1]->data)));
						endPoint->incomingQueue->enqueue(new Message(htpReceiveBuffer + SESSIONID_LENGTH, byteRead-SESSIONID_LENGTH));
						clearParams(&params);
					}
					//--	else: other commands not allowed without service
				}
				//--	else: data frame not allowed without service
			}
			else{
				//--	service existed with endPointID, remove sessionID before forward
				if (service->isWorking()){
					DaemonEndPoint* endPoint = service->getDaemonEndPoint(endPointID);
					endPoint->incomingQueue->enqueue(new Message(htpReceiveBuffer + SESSIONID_LENGTH, byteRead-SESSIONID_LENGTH));
				}
				//--	else: current service is not working
			}
		}
		//--	else: read error
	}
	printf("Exit htp reading loop\n");
}

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

    //--	create a thread to read from unix domain socket
    working = true;
    pthread_attr_init(&unixReadingThreadAttr);
    pthread_create(&unixReadingThread, &unixReadingThreadAttr, unixReadingLoop, NULL);
      
	//--	create a thread to read from htp socket
	pthread_attr_init(&htpReadingThreadAttr);	
	pthread_create(&htpReadingThread, &htpReadingThreadAttr, htpReadingLoop, NULL);

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
    	pthread_join(htpReadingThread, NULL);
    	    	   
    	
    	//--	DO CLEANING UP BEFORE EXIT	--//    	
    	//--	remove all DaemonService instances	
		for (auto& it : serviceTable){
			DaemonService* service = it.second;	
			if (service!=NULL){
				//--	remove all reference of this service				
				service->stopWorking();
				delete service;
			}
		}
		
		delete serviceTableMutex;
		delete unixReceiveBuffer;
		delete htpReceiveBuffer;
    	unlink(SVC_DAEMON_PATH.c_str());    	
    	printf("SVC daemon stopped.\n");
}


