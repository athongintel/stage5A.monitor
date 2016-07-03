#include "SendFileApp.h"


SendFileApp::SendFileApp(){
	
	SVCHost* remotehost = new SVCHostIP("149.56.142.13");
	string ca = "./pki/ca.crt";
	string cert = "./pki/client.crt";	
	string pkey = "./pki/client.key";
	SVCAuthenticator* authenticator = new SVCAuthenticatorPKI(ca, cert, pkey);
	
	SVC* svc = new SVC(authenticator);
	svc.establishConnection(this, remotehost);
}

string SendFileApp::getAppID(){
	return "SEND_FILE_APP";
}

void SendFileApp::setFile(string fileName){


}

bool SendFileApp:sendFile(){



}
