#include "SVC-header.h"

bool isEncryptedCommand(enum SVCCommand command){
	return (command != SVC_CMD_CHECK_ALIVE 
					&& command != SVC_CMD_REGISTER_APP 
					&& command != SVC_CMD_CONNECT_STEP1
					&& command != SVC_CMD_CONNECT_STEP2
					&& command != SVC_CMD_CONNECT_STEP3);
}

/*
	This function load the command, params and necessary infos into buffer
*/

/*
void prepareCommand(uint8_t* buffer, size_t* len, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params){

	//printf("1\n");

	size_t bufferLength = 7;
	uint8_t* pointer = buffer + bufferLength;
	
	for (int i=0; i<params->size(); i++){
		bufferLength += 2 + (*params)[i]->length;
	}
	//printf("2\n");
	//1. sessionID
	memcpy(buffer, (uint8_t*) &sessionID, SESSIONID_LENGTH);
	//2. 1 info byte
	buffer[4] = 0x00;
	buffer[4] = buffer[4] | SVC_COMMAND_FRAME;
	buffer[4] = buffer[4] | SVC_URGENT_PRIORITY; 	//commands are always urgent
	buffer[4] = buffer[4] | SVC_USING_TCP; 			//to ensure the delivery of commands
	//printf("3\n");
	if (isEncryptedCommand(command)) buffer[4] = buffer[4] | SVC_ENCRYPTED;

	//printf("4\n");

	//3. 1 byte command ID
	buffer[5] = command;
	//4. 1 byte param length
	buffer[6] = params->size();
	//printf("5\n");
	//5. add params
	for (int i=0; i<params->size(); i++){
		//printf("copy len %d, data: ", (*params)[i]->length);
		//printBuffer((*params)[i]->param, (*params)[i]->length);
		memcpy(pointer, (uint8_t*) &((*params)[i]->length), 2);
		memcpy(pointer+2, (*params)[i]->data, (*params)[i]->length);
		pointer += 2 + (*params)[i]->length;
	}

	//printf("5\n");
	*len = bufferLength;
	//printf("6\n");
}
*/
/*
void sendCommand(int socket, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params){
	uint8_t* buffer = (uint8_t*)malloc(SVC_DEFAULT_BUFSIZ);
	size_t len;
	prepareCommand(buffer, &len, sessionID, command, params);
	printf("send command: ");
	printBuffer(buffer, len);	
	send(socket, buffer, len, 0);
}
*/

void extractParams(const uint8_t* buffer, vector<SVCCommandParam*>* params){
	
	int argc = buffer[0];
	int pointer = 1;
	uint16_t len;
	
	for (int i=0; i<argc; i++){		
		len = *((uint16_t*)(buffer+pointer));
		params->push_back(new SVCCommandParam(len, buffer + pointer + 2));
		pointer += len+2;
	}
}

void clearParams(vector<SVCCommandParam*>* params){
	for (int i=0; i<params->size(); i++){
		delete (*params)[i];
	}
	params->clear();
}

bool waitSignal(int waitingSignal, int timeoutSignal, int timeout){
	
	sigset_t sig;
	sigemptyset(&sig);
	sigaddset(&sig, waitingSignal);
	sigaddset(&sig, timeoutSignal);	
	
	timer_t timer;
	struct sigevent evt;
	evt.sigev_notify = SIGEV_SIGNAL;
	evt.sigev_signo = timeoutSignal;
	evt.sigev_notify_thread_id = pthread_self();
	timer_create(CLOCK_REALTIME, &evt, &timer);
	
	struct itimerspec time;
	time.it_interval.tv_sec=0;
	time.it_interval.tv_nsec=0;	
	time.it_value.tv_sec=timeout/1000;
	time.it_value.tv_nsec=(timeout - time.it_value.tv_sec*1000)*1000000;	
	timer_settime(timer, 0, &time, NULL);	
	
	/*	wait for either timeoutSignal or watingSignal	*/
	int caughtSignal;
	sigwait(&sig, &caughtSignal);
	
	return caughtSignal == waitingSignal;	
}

void printBuffer(const uint8_t* buffer, size_t len){

	for (int i=0; i<len; i++){
		printf("%02x ", buffer[i]);
	}
	printf("\n");
}
