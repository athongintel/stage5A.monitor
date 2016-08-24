#include "SendFileAppServer.h"

//--TODO:	to be removed

using namespace std;

int main(int argc, char** argv){
	SendFileAppServer* app = new SendFileAppServer();
	printf("server app initiated!\n");
}

SendFileAppServer::SendFileAppServer(){		
	
	SVC* svc = new SVC(this, this);
	SVCEndPoint* endPoint = svc->listenConnection();
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
