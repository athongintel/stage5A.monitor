#include "SendFileAppServer.h"

using namespace std;

int main(int argc, char** argv){
	SendFileAppServer* app = new SendFileAppServer();
	cout<<"server app initiated!"<<endl;
}

SendFileAppServer::SendFileAppServer(){
	
	SVCHost* remotehost = new SVCHostIP("0.0.0.0");
	
	SVC* svc = new SVC(this, this);
	if (!svc->establishConnection(remotehost)){
		throw "Error establishing connection";
	}
}

//interface implementation

string SendFileApp::getAppID(){
	return string("SEND_FILE_APP");
}

bool SendFileApp::isServer(){
	return true;
}

string SendFileApp::getIdentity(){
	return "IM_THE_SERVER";
}

bool SendFileApp::verifyIdentity(string identity, string challenge, string proof){
	return (identity.compare("IM_THE_CLIENT")==0 && challenge.append("OK").compare(proof)==0);
}

string SendFileApp::generateProof(string challenge){
	return challenge.append("OK");
}

string SendFileApp::generateChalenge(){
	return string("this can be another thing");
}
