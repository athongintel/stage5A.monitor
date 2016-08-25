#include "SVC.h"

//---for debugging, to be removed
#include <iostream>
#include <errno.h>


//--	SVC IMPLEMENTATION	//
SVC::SVC(SVCApp* localApp, SVCAuthenticator* authenticator){
	
	const char* errorString;
	
	this->localApp = localApp;
	this->authenticator = authenticator;
	
	endPointsMutex = new shared_mutex();
	
	//--	check for existed socket
	this->appID = (uint32_t)hasher(localApp->getAppID());
	this->svcClientPath = SVC_CLIENT_PATH_PREFIX + to_string(appID);	
	if (unlink(this->svcClientPath.c_str())==0){
		errorString = SVC_ERROR_NAME_EXISTED;
		goto errorInit;
	}
		
	//--	daemon's endpoint to write to
	memset(&this->daemonSocketAddress, 0, sizeof(this->daemonSocketAddress));
	this->daemonSocketAddress.sun_family = AF_LOCAL;
	memcpy(this->daemonSocketAddress.sun_path, SVC_DAEMON_PATH.c_str(), SVC_DAEMON_PATH.size());
	this->svcDaemonSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);
	connect(this->svcDaemonSocket, (struct sockaddr*) &this->daemonSocketAddress, sizeof(this->daemonSocketAddress));
	
	//--	svc's endpoint to read from
	memset(&this->svcSocketAddress, 0, sizeof(this->svcSocketAddress));
	this->svcSocketAddress.sun_family = AF_LOCAL;
	memcpy(this->svcSocketAddress.sun_path, this->svcClientPath.c_str(), this->svcClientPath.size());
	this->svcSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);	
	if (bind(this->svcSocket, (struct sockaddr*)&this->svcSocketAddress, sizeof(this->svcSocketAddress))==-1){
		errorString = SVC_ERROR_BINDING;
		goto errorInit;	
	}
	
	this->connectionRequest = new Queue<Message*>();
	
	//--	create reading thread	
	sigset_t sig;
	sigfillset(&sig);
	sigaddset(&sig, SIGUSR2);
	sigaddset(&sig, SIGUSR1);
	if (pthread_sigmask(SIG_BLOCK, &sig, NULL)!=0){
		errorString = SVC_ERROR_CRITICAL;
		goto errorInit;
	}		
	this->working = true;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_create(&this->readingThread, &attr, processPacket, this);
	
	goto success;
	
	errorInit:
		//destruct params manually
		this->destruct();
		throw errorString;
		
	success:
		printf("svc created\n");	
}

void SVC::destruct(){
	this->working = false;	
	pthread_join(this->readingThread, NULL);
	
	//--	remove remaining endpoints
	this->endPointsMutex->lock();
	for (SVCEndPoint* endPoint : this->endPoints){
		if (endPoint!=NULL){
			delete endPoint;
		}
	}
	this->endPoints.clear();
	this->endPointsMutex->unlock();
	
	unlink(this->svcClientPath.c_str());
	cout<<"svc destructed\n";
}

void SVC::stopWorking(){
	this->working = false;
}

SVCEndPoint* SVC::getEndPointByID(uint64_t endPointID){
	this->endPointsMutex->lock_shared();
	for (SVCEndPoint* endPoint : this->endPoints){
		if (endPoint!=NULL){
			if (endPoint->endPointID == endPointID){
				this->endPointsMutex->unlock_shared();
				return endPoint;
			}
		}
	}
	this->endPointsMutex->unlock_shared();
	return NULL;
}

SVC::~SVC(){
	this->destruct();
}

/* SVC PRIVATE FUNCTION IMPLEMENTATION	*/

void* SVC::processPacket(void* args){

	SVC* _this = (SVC*)args;
		
	int byteRead;
	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);

	uint64_t endPointID;	
	vector<SVCCommandParam*> params;
	SVCEndPoint* endPoint;
	
	while (_this->working){
		do{
			byteRead = recv(_this->svcSocket, buffer, SVC_DEFAULT_BUFSIZ, MSG_DONTWAIT);		
		}
		while((byteRead==-1) && _this->working);
		
		if (byteRead>0){
			printf("read a packet: ");
			printBuffer(buffer, byteRead);
			
			endPointID = *((uint64_t*)buffer);
			uint8_t infoByte = buffer[ENDPOINTID_LENGTH];
			
			_this->endPointsMutex->lock_shared();
			endPoint = _this->getEndPointByID(endPointID);
			_this->endPointsMutex->unlock_shared();
		
			if (infoByte & SVC_COMMAND_FRAME){				
				extractParams(buffer + ENDPOINTID_LENGTH + 2, &params);
				
				enum SVCCommand cmd = (enum SVCCommand)buffer[ENDPOINTID_LENGTH + 1];
				
				if (endPoint==NULL){
					if (cmd == SVC_CMD_CONNECT_STEP1){
						//--	add this to connection request
						printf("SVC_CMD_CONNECT_STEP1\n");
						if (_this->connectionRequest->notEmpty()){
							_this->connectionRequest->enqueue(new Message(buffer, byteRead));
						}
						else{
							//--	notify first needing endPoint, no enqueue
							_this->endPointsMutex->lock_shared();
							for (SVCEndPoint* ep : _this->endPoints){
								if (ep!=NULL){
									SVCDataReceiveNotificator* notificator = ep->signalNotificator->getNotificator(cmd);
									if (notificator!=NULL){
										notificator->handler(buffer, byteRead, notificator);
										ep->signalNotificator->removeNotificator(cmd);
										break;	//--	we only notify the first
									}
									//--	else: this endPoint doesn't wait for this cmd
								}
								//--	else: NULL
							}
							_this->endPointsMutex->unlock_shared();				
						}
					}
					//--	else: other commands not allows without endPoint
				}
				else{
					//--	notify the corresponding endPoint
					SVCDataReceiveNotificator* notificator = endPoint->signalNotificator->getNotificator(cmd);
					if (notificator!=NULL){
						notificator->handler(buffer, byteRead, notificator);
						endPoint->signalNotificator->removeNotificator(cmd);
					}
				}				
			}
			else{
				if (endPoint!=NULL){
					//--	forward to dataQueue
					endPoint->dataQueue->enqueue(new Message(buffer, byteRead));
				}
				//--	else: no data allowed without endPoint				
			}
		}
	}
	
	free(buffer);
	printf("svc process packet stopped\n");
}


/*	SVC PUBLIC FUNCTION IMPLEMENTATION	*/

SVCEndPoint* SVC::establishConnection(SVCHost* remoteHost){
	
	SVCEndPoint* rs = NULL;
	SVCEndPoint* endPoint;
	SignalNotificator* sigNot;		
	vector<SVCCommandParam*> params;
	
	sigNot = new SignalNotificator();
	uint64_t endPointID = (uint64_t)hasher(this->localApp->getAppID()+to_string(rand()));	
	endPoint = new SVCEndPoint(this, sigNot);
	endPoint->endPointID = endPointID;
	
	endPointsMutex->lock();
	endPoints.push_back(endPoint);
	endPointsMutex->unlock();

	//--	authentication variables
	string identity;
	string challengeSent;
	string challengeReceived;
	string proof;
	
	//--	send establishing request to the daemon with appropriate params			
	
	challengeSent = this->authenticator->generateChallenge();
	uint32_t serverAddress  = remoteHost->getHostAddress();
	uint32_t appID = (uint32_t)hasher(this->localApp->getAppID());
	clearParams(&params);
	params.push_back(new SVCCommandParam(challengeSent.size(), (uint8_t*)challengeSent.c_str()));
	params.push_back(new SVCCommandParam(4, (uint8_t*) &appID));
	params.push_back(new SVCCommandParam(4, (uint8_t*) &serverAddress));
	endPoint->sendCommand(SVC_CMD_CONNECT_STEP1, &params);
	
	//--	wait for SVC_CMD_CONNECT_STEP2, identity + proof + challenge, respectively. keyexchange is retained at daemon level.
	if (sigNot->waitCommand(SVC_CMD_CONNECT_STEP2, &params, SVC_DEFAULT_TIMEOUT)){
		//--	get identity, proof, challenge
		identity = string((char*)params[0]);
		proof = string((char*)params[1]);
		challengeReceived = string((char*)params[2]);
		//--	verify server's identity
		if (this->authenticator->verifyIdentity(identity, challengeSent, proof)){
			//--	ok, server's identity verified. request daemon to perform keyexchange. the daemon responds the success of encrypting process
			clearParams(&params);			
			endPoint->sendCommand(SVC_CMD_CONNECT_STEP3, &params);
			//--	wait for daemon. if the connection to this address is already secured, it will return shortly
			if (sigNot->waitCommand(SVC_CMD_CONNECT_STEP3, &params, SVC_DEFAULT_TIMEOUT)){			
				//a.3 perform SVC_CMD_CONNECT_STEP4, send identity + proof
				clearParams(&params);
				identity = this->authenticator->getIdentity();
				proof = this->authenticator->generateProof(challengeReceived);
				params.push_back(new SVCCommandParam(identity.size(), (uint8_t*)identity.c_str()));
				params.push_back(new SVCCommandParam(proof.size(), (uint8_t*)proof.c_str()));
				endPoint->sendCommand(SVC_CMD_CONNECT_STEP4, &params);				
				rs = endPoint;
			}
			/*
			else: no response from daemon, error occured or timeout
			*/								
		}
		/*
		else: server identity verification failed, exception
		*/
	}
	/*
	else: time out
	*/
	return rs;
}


SVCEndPoint* SVC::listenConnection(){

	vector<SVCCommandParam*> params;
	SVCEndPoint* rs = NULL;
	SVCEndPoint* endPoint;
	Message* message;
	uint64_t endPointID;
	SignalNotificator* sigNot;

	//--	authentication variablees	
	string identity;
	string challengeSent;
	string challengeReceived;
	string proof;
	
	sigNot = new SignalNotificator();
	endPoint = new SVCEndPoint(this, sigNot);
	
	endPointsMutex->lock();
	endPoints.push_back(endPoint);
	endPointsMutex->unlock();
	
	if (this->connectionRequest->peak(&message)){
		//--	process this connection request, this is a SVC_CMD_CONNECT_STEP1		
		clearParams(&params);
		extractParams(message->data + ENDPOINTID_LENGTH + 2, &params);
	}
	else{
	//--	no pending request		
		while (!sigNot->waitCommand(SVC_CMD_CONNECT_STEP1, &params, SVC_DEFAULT_TIMEOUT)){
			if (this->working){
				//--	remove the notificator before waiting again, otherwise we'll get exception			
				sigNot->removeNotificator(SVC_CMD_CONNECT_STEP1);
				printf("retry...\n");
			}
			else{
				//--	no more working, brake from for loop
				break;
			}
		}
	}
	
	if (this->working){
		endPointID = *((uint64_t*)(params[0]->data));
		endPoint->endPointID = endPointID;
		
		//--	read the challenge
		challengeReceived = string((char*)params[0]->data);			
		proof = this->authenticator->generateProof(challengeReceived);
		identity = this->authenticator->getIdentity();
		challengeSent = this->authenticator->generateChallenge();
		
		printf("challenge received: %s\n", challengeReceived.c_str());
		//--	send response
		clearParams(&params);
		params.push_back(new SVCCommandParam(identity.size(), (uint8_t*)identity.c_str()));
		params.push_back(new SVCCommandParam(proof.size(), (uint8_t*)proof.c_str()));
		params.push_back(new SVCCommandParam(challengeSent.size(), (uint8_t*)challengeSent.c_str()));	
		endPoint->sendCommand(SVC_CMD_CONNECT_STEP2, &params);
	
		//--	wait for SVC_CMD_CONNECT_STEP4, step3 is handled by the daemon		
		if (sigNot->waitCommand(SVC_CMD_CONNECT_STEP4, &params, SVC_DEFAULT_TIMEOUT)){
			//--	read identity + proof
			identity = string((char*)params[0]);
			proof = string((char*)params[1]);
		
			//--	verify client's identity
			if (this->authenticator->verifyIdentity(identity, challengeSent, proof)){
				rs = endPoint;
			}
		}
		//--	else: time out
	}
	//--	else: work is broken
	return rs;
}


//--	SVCENDPOINT IMPLEMENTATION	//

SVCEndPoint::SVCEndPoint(SVC* svc, SignalNotificator* sigNot){
	this->svc = svc;
	this->signalNotificator = sigNot;
	this->dataQueue = new MutexedQueue<Message*>();
};

void SVCEndPoint::sendCommand(enum SVCCommand cmd, vector<SVCCommandParam*>* params){				

	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	int pointer = 0;
	size_t bufferLength = ENDPOINTID_LENGTH + 3;	//endpointid + info + cmd + argc

	for (int i=0; i<params->size(); i++){
		//--	2 bytes for param length, then the param itself
		bufferLength += 2 + (*params)[i]->length;
	}								
			
	//--	ADD HEADER	--//				
	//--	endPointID
	memcpy(buffer + pointer, (uint8_t*) &this->endPointID, ENDPOINTID_LENGTH);
	pointer += ENDPOINTID_LENGTH;
	//--	info byte				
	buffer[pointer] = 0x00;
	buffer[pointer] = buffer[pointer] | SVC_COMMAND_FRAME;
	buffer[pointer] = buffer[pointer] | SVC_URGENT_PRIORITY; 	//commands are always urgent
	buffer[pointer] = buffer[pointer] | SVC_USING_TCP; 			//to ensure the delivery of commands				
	if (isEncryptedCommand(cmd)) buffer[pointer] = buffer[pointer] | SVC_ENCRYPTED;
	pointer += 1;
	//--	1 byte command ID				
	buffer[pointer] = cmd;
	pointer += 1;				
	//--	1 byte param length				
	buffer[pointer] = params->size();
	pointer += 1;
	
	//--	ADD PARAMS	--//
	for (int i=0; i<params->size(); i++){					
		memcpy(buffer + pointer, (uint8_t*) &((*params)[i]->length), 2);
		memcpy(buffer + pointer + 2, (*params)[i]->data, (*params)[i]->length);
		pointer += 2 + (*params)[i]->length;
	}
	
	//--	SEND	--//
	send(this->svc->svcDaemonSocket, buffer, bufferLength, 0);
	printf("endpoint send: ");
	printBuffer(buffer, bufferLength);
	//--	free params
	clearParams(params);
}

SVCEndPoint::~SVCEndPoint(){
	delete this->dataQueue;
	printf("end point %016x removed\n", this->endPointID);
}

int SVCEndPoint::sendData(const uint8_t* data, size_t dalalen, uint8_t priority, bool tcp){
}

int SVCEndPoint::readData(uint8_t* data, size_t* len){
}

