#include "SendFileApp.h"

using namespace std;

int main(int argc, char** argv){
	try{
		SendFileApp* app = new SendFileApp();
		cout<<"app initiated!"<<endl;
	}
	catch(const char* err){
		cout<<err<<endl;
	}	
}

SendFileApp::SendFileApp(){
	
	SVCHost* remotehost = new SVCHostIP("149.56.142.13");
	
	this->svc = new SVC(this, this);
	if (!svc->establishConnection(remotehost)){
		svc->~SVC();
		throw "Error establishing connection";
	}
}

//interface implementation

string SendFileApp::getAppID(){
	return string("SEND_FILE_APP");
}

bool SendFileApp::isServer(){
	return false;
}

string SendFileApp::getIdentity(){
	return "IM_THE_CLIENT";
}

bool SendFileApp::verifyIdentity(string identity, string challenge, string proof){
	return (identity.compare("IM_THE_SERVER")==0 && challenge.append("OK").compare(proof)==0);
}

string SendFileApp::generateProof(string challenge){
	return challenge.append("OK");
}

string SendFileApp::generateChallenge(){
	return string("this can be anything");
}
