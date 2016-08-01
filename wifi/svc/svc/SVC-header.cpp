#include "SVC-header.h"

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
