#include "SendFileApp.h"


int main(int argc, char** argv){

	SendFileApp* fileSender = new SendFileApp();
	fileSender.setFile(argc[1]);
	if (fileSender.sendFile()){
		printf("File sent!\n");
	else
		printf("Error!\n");
	
	return 0;

}
