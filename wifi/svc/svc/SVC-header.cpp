#include "SVC-header.h"

ssize_t _sendCommand(int socket, enum SVCCommand command, const vector<SVCCommandParam*>* params){	
	size_t bufferLength = 3;
	for (int i=0; i<params->size(); i++){
		bufferLength += 2 + (*params)[i]->length; //2 bytes for param length, then the param itself
	}
	uint8_t* const buffer = (uint8_t*)malloc(bufferLength);
	uint8_t* pointer = buffer + 3;
	
	//add header
	buffer[0] = SVC_COMMAND_FRAME;
	buffer[1] = command;
	buffer[2] = params->size();
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
