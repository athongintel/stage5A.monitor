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
		this->array[this->tail] = (uint8_t*)malloc(len);
		memcpy(this->array[this->tail], message, len);
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


/*
	This function will only return the address of the message inside the queue.
	The routine which calls this function can thus use the returned pointer to modify the content of the message.
	It's unneccessary (and wrongly - memory leaks) to allocate new memory.

*/
int peak(uint8_t** message, size_t& len){
	int rs;
	if (this->messageMutex.try_lock()){
		if (this->size>0){
			*message = this->array[this->head];
			*len = this->arrayLen[this->head];
			rs = 0;
		}
		this->messageMutex.unlock();
	}
	else{
		rs = -1;
	}
	return rs;
}


int dequeue(){
	int rs;
	this->messageMutex.lock();
	if (this->size>0){				
		if (this->head==0) this->head=this->maxsize;
		this->size--;
		delete this->array[this->head];
		rs = 0;
	}
	else{
		rs = -1;
	}
	this->messageMutex.unlock();
	return rs;
}

MessageQueue::~MessageQueue(){
	uint8_t[SVC_DEFAULT_BUFSIZ] buffer;	
	while (this->notEmpty()){
		this->dequeue(buffer);
	}
	delete this->array;
}

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
void prepareCommand(uint8_t* buffer, size_t* len, uint32_t sessionID, enum SVCCommand command, const vector<SVCCommandParam*>* params){
	size_t bufferLength = 7;
	uint8_t* pointer = buffer + bufferLength;
	
	for (int i=0; i<params->size(); i++){
		/*	2 bytes for param length, then the param itself	*/
		bufferLength += 2 + (*params)[i]->length; 
	}
	
	/*	add header	*/
	//1. sessionID
	memcpy(buffer, (uint8_t*) &sessionID, 4);
	//2. 1 info byte
	buffer[4] = 0x00;
	buffer[4] = buffer[4] | SVC_COMMAND_FRAME;
	buffer[4] = buffer[4] | SVC_URGENT_PRIORITY; 	//commands are always urgent
	buffer[4] = buffer[4] | SVC_USING_TCP; 			//to ensure the delivery of commands
	
	if (isEncryptedCommand(command)) buffer[4] = buffer[4] | SVC_ECRYPTED;
	
	//3. 1 byte command ID
	buffer[5] = command;
	//4. 1 byte param length
	buffer[6] = params->size();
	
	//5. add params
	for (int i=0; i<params->size(); i++){
		memcpy(pointer, (uint16_t*) &((*params)[i]->length), 2);
		memcpy(pointer+2, (*params)[i]->param, (*params)[i]->length);
		pointer += 2 + (*params)[i]->length;
	}
}

ssize_t _sendCommand(int socket, const uint8_t* buffer, size_t len){	
	return send(socket, buffer, len, 0);	
}

ssize_t _sendCommand(MessageQueue* messageQueue, const uint8_t* buffer, size_t len){
	ssize_t rs;
	messageQueue->messageMutex.lock();
	rs = messageQueue->enqueue(buffer, len);
	messageQueue->messageMutex.unlock();
	return rs;
}

void printBuffer(const uint8_t* buffer, size_t len){

	for (int i=0; i<len; i++){
		printf("%02x ", buffer[i]);
	}
	printf("\n");
}
