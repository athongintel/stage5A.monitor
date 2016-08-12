#include "SVC-header.h"

MessageQueue::MessageQueue(int maxsize){
	this->maxsize = maxsize;
	this->array = (uint8_t**)malloc(sizeof(uint8_t*)*this->maxsize);
	this->arrayLen = (size_t*)malloc(sizeof(size_t)*this->maxsize);
	this->size=0;
	this->head=0;
	this->tail=0;
}

bool MessageQueue::notEmpty(){
	bool rs;
	this->messageMutex.lock();
	rs = this->size!=0;
	this->messageMutex.unlock();
	return rs;
}

int MessageQueue::enqueue(const uint8_t* message, size_t len){
	int rs;
	this->messageMutex.lock();
	if (this->size<this->maxsize){
		this->array[this->tail] = message;
		this->arrayLen[this->tail] = len;
		if (this->tail==this->maxsize) this->tail=0;
		this->size++;
		rs = 0;
	}
	else{
		rs = -1;
	}
	this->messageMutex.unlock();
	return rs;
}

size_t dequeue(uint8_t** returnedMessage){
	size_t rs;
	this->messageMutex.lock();
	if (this->size!=0){
		*returnedMessage = this->array[this->head];
		rs = this->arrayLen[this->head];
		if (this->head==0) this->head=this->maxsize;
		this->size--;
	}
	else{
		rs = 0;
	}
	this->messageMutex.unlock();
	return rs;
}

MessageQueue::~MessageQueue(){
	delete this->array;
}

ssize_t _sendCommand(int socket, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params){	
	size_t bufferLength = 7;
	for (int i=0; i<params->size(); i++){
		bufferLength += 2 + (*params)[i]->length; //2 bytes for param length, then the param itself
	}
	uint8_t* const buffer = (uint8_t*)malloc(bufferLength);
	uint8_t* pointer = buffer + 7;
	
	//add header
	memcpy(buffer, (uint8_t*) &sessionID, 4);
	buffer[4] = SVC_COMMAND_FRAME;
	buffer[5] = command;
	buffer[6] = params->size();
	//add params
	for (int i=0; i<params->size(); i++){
		memcpy(pointer, (uint16_t*) &((*params)[i]->length), 2);
		memcpy(pointer+2, (*params)[i]->param, (*params)[i]->length);
		pointer += 2 + (*params)[i]->length;
	}
	
	//send packet
	ssize_t result = send(socket, buffer, bufferLength, 0);	
	free(buffer);
	return result;
}

void printBuffer(const uint8_t* buffer, size_t len){

	for (int i=0; i<len; i++){
		printf("%02x ", buffer[i]);
	}
	printf("\n");
}
