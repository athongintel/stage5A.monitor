#include "svc.h"


SVC::SVC(SVCAuthenticator* authenticator){
	this.authenticator = authenticator;
}

bool SVC::establishConnection(SVCApp* localApp, SVCHost* remoteHost){
	
	//1. create a socket to svc daemon
	socket = socket(AF_LOCAL, SOCK_DGRAM, 0);
	
	//1.1 sending localApp.getAppID()
		//-- svc daemon execute hole punching and make connection to the required host
		
		
	//2. read: version + security paramter + identity + auth data
	
	//3. verify identity
	string proof;
	string identity;
	authenticator.verifyIdentity(identity, proof);
	
	//4. send: security param
	
	//5. establish secured connection
	
	//6. send: version, identity, auth data
	
	//7. set isAuthentiicated
}
