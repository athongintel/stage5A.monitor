#include "SendFileAppServer.h"

//--TODO:	to be removed

using namespace std;

SVC* svcInstance;
volatile bool working;

void signal_handler(int signal){
	if (signal == SIGINT){
		printf("SIGINT caught, stop working\n");
		svcInstance->stopWorking();
		working = false;
	}
}

int main(int argc, char** argv){

    //--	set thread signal mask
    sigset_t sigset;
    sigemptyset(&sigset);    
    sigaddset(&sigset, SVC_ACQUIRED_SIGNAL);
    sigaddset(&sigset, SVC_TIMEOUT_SIGNAL);
    sigaddset(&sigset, SVC_SHARED_MUTEX_SIGNAL);
    pthread_sigmask(SIG_BLOCK, &sigset, NULL);
	
	//--	trap SIGINT signal
	struct sigaction act;
	act.sa_handler = signal_handler;
	sigfillset(&act.sa_mask);
	sigdelset(&act.sa_mask, SIGINT);
	sigaction(SIGINT, &act, NULL);
	
	working = true;
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
		printf("retry...\n");
	}
	while (endPoint == NULL && working);
	
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
