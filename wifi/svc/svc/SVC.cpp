#include "svc.h"

using namespace std;

SVC::SVC(SVCApp* localApp, SVCAuthenticator* authenticator){

	//local variables
	vector<SVCCommandParam*> params;	
	uint8_t buffer[3];
	string error;
	
	this->authenticator = authenticator;
	this->localApp = localApp;
	
	//0. 	check for existed socket
	hash<string> hasher;
	int hashedID = (int)hasher(localApp.getAppID());
	this->svcClientPath = SVC_CLIENT_PATH_PREFIX.append(to_string(hashedID));
	
	int unlinkResult = unlink(this->svcClientPath);
	if (unlinkResult !=0){
		error = SVC_ERROR_NAME_EXISTED;
		goto errorInit;
	}
	
	//1. 	create a socket to svc daemon
	//1.1 	daemon's endpoint
	memset(&this->daemonSocketAddress, 0, sizeof(this->daemonSocketAddress));
	this->daemonSocketAddress.sun_famimy = AF_LOCAL;
	memcpy(this->daemonSocketAddress, SVC_DAEMON_PATH, SVC_DAEMON_PATH.size());
	//1.2	svc's endpoint
	memset(&this->svcSocketAddress, 0, sizeof(this->svcSocketAddress));
	this->svcSocketAddress.sun_famimy = AF_LOCAL;
	memcpy(this->svcSocketAddress, this->svcClientPath, this->svcClientPath.size());
	//1.3 	create new diagram socket and bind to svc's endpoint
	this->svcSocket = socket(AF_UNIX, SOCK_DGRAM, 0);
	bind(this->svcSocket, (struct sockaddr*)&this->svcSocketAddress, sizeof(this->svcSocketAddress));
	//1.4	connect to daemon
	connect(this->svcSocket, (struct sockaddr*) &this->daemonSocketAddress, sizeof(this->daemonSocketAddress));
	
	//2.	register localApp to daemon
	//2.1	send command to daemon
	params.push_back(new SVCCommandParam(4, hashedID));
	_sendCommand(SVC_CMD_REGISTER_APP, params);
	
	//2.2	wait for response, 1 argument expected
	params.clear();
	if (!_waitCommand(SVC_CMD_REGISTER_APP, params, SVC_DEFAULT_TIMEOUT)){
		error = 
		goto errorInit;
	}	
	if (*(params[0]->param) == 1)
		goto success;
		
	errorInit:
		//destruct params manually
		params.clear();
		throw error;
		
	success:
		//return
}


/* SVC PRIVATE FUNCTION IMPLEMENTATION	*/
ssize_t SVC::_sendCommand(enum SVCCommand command, uint8_t argc, const SVCCommandParam* argv){
	size_t bufferLength = 3;
	for (int i=0; i<argc; i++){
		bufferLength += 2 + argv[i]->length; //2 bytes for param length, then the param itself
	}
	uint8_t* const buffer = (uint8_t*)malloc(bufferLength);
	uint8_t* pointer = buffer + 3;
	
	//add header
	buffer[0] = SVC_COMMAND_FRAME;
	buffer[1] = command;
	buffer[2] = argc;
	//add params
	for (int i=0; i<argc; i++){
		memcpy(pointer, (uint16_t)argv[i]->length, 2);
		memcpy(pointer+2, argv[i]->param, argv[i]->length);
		pointer += 2 + argv[i]->length;
	}
	
	//send packet
	ssize_t result = send(this->svcSocket, buffer, bufferLength, 0);	
	free(buffer);
	return result;
}

bool SVC::_waitCommand(enum SVCCommand command, vector<SVCCommandParam*> &params, int timeout){	
	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	recv(this->svcSocket, buffer, SVC_DEFAULT_BUFSIZ);
	
	free(buffer);
}

/*	SVC PUBLIC FUNCTION IMPLEMENTATION	*/

bool SVC::establishConnection(SVCHost* remoteHost){

	vector<SVCCommandParam*> params;

	//1.	send establishing request to the daemon with appropriate params
	//1.1	dertemine this is a client or server
	uint8_t isServer = localApp.isServer()? 1:0;
	params.push_back(new SVCCommandParam(1, &isServer));
	//1.2	add challenge from app and send to the daemon
	if (isServer == 0){
		//this app is the CLIENT, get challenge from localApp and send to daemon
		string challenge = this->authenticator.getChallenge();
		params.push_back(new SVCCommandParam(challenge.size(), (uint8_t*)challenge.c_str()));
	}
	_sendCommand(SVC_CMD_NEGOTIATION_STEP1, params);
	
	
	if (isServer == 0){
		//a.	CLIENT BUSSINESS CODE
		//a2.	wait for response from the daemon, identity + proof + challenge, respectively. keyexchange is retained at daemon level.
		recv(this->svcSocket, buffer, sizeof(buffer), 0);
		
	}
	else{
		//b.	SERVER BUSINESS CODE
		//b.2.	wait for SVC_CMD_NEGOTIATION_STEP1 from the daemon, challenge from client. version is retained at daemon level.
		recv(this->svcSocket, buffer, sizeof(buffer), 0);
		
	}
}


bool SVC::setDataReceiveHandler(SVCDataReceiveHandler handler){
	this->dataHandler = handler;
}

int SVC::sendData(const uint8_t* data, size_t dalalen, SVCPriority priority, bool tcp){
	if (this->isAuthenticated){
		size_t bufferLength = 7 + datalen;
		uint8_t* buffer = (uint8_t*)malloc(bufferLength);
	
		buffer[0] = SVC_DATA_FRAME;
		buffer[1] = priority;
		buffer[2] = tcp? (uint8_t)1 : 0;
		memcpy(buffer+3, (uint32_t)datalen, 4);	
		memcpy(buffer+7, data, datalen);
	
		//send packet
		send(this->svcSocket, buffer, bufferLength, 0);	
		free(buffer);
	}
	else{
		throw SVC_ERROR_NOT_ESTABLISHED;
	}
}


