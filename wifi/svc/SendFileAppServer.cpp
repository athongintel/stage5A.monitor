#include "SendFileAppServer.h"

//--TODO:	to be removed

using namespace std;

SVC* svcInstance;

void signal_handler(int signal){
	if (signal == SIGINT){
		svcInstance->stopWorking();
	}
}

int main(int argc, char** argv){

	//--	trap SIGINT signal
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);
	
	SendFileAppServer* app = new SendFileAppServer();
}

SendFileAppServer::SendFileAppServer(){		
	svcInstance = new SVC(this, this);
	SVCEndPoint* endPoint;
	
	printf("waiting for connection request\n");
	do{
		endPoint = svcInstance->listenConnection();
		if (endPoint!=NULL){
			printf("client connected\n");
		}
	}
	while (endPoint == NULL);
	
	delete svcInstance;
}

//interface implementation

//--	SVCApp interface

string SendFileAppServer::getAppID(){
	return string("SEND_FILE_APP");
}

/*	SVCAuthenticator interface	*/

string SendFileAppServer::getIdentity(){
	return "IM_THE_SERVER";
}

bool SendFileAppServer::verifyIdentity(string identity, string challenge, string proof){
	return (identity.compare("IM_THE_CLIENT")==0 && challenge.append("OK").compare(proof)==0);
}

string SendFileAppServer::generateProof(string challenge){
	return challenge.append("OK");
}

string SendFileAppServer::generateChallenge(){
	return string("this can be another thing");
}
