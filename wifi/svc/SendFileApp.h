#include "svc/SVC.h"
#include "svc/SVCHost.h"
#include "svc/SVCHostIP.h"
#include <string>
#include <iostream>

class SendFileApp : SVCApp, SVCAuthenticator{

	public:
		SendFileApp();
		
		string getAppID();
		bool isServer();
		std::string getIdentity();
		bool verifyIdentity(std::string identity, std::string challenge, std::string proof);
		std::string generateProof(std::string challenge);
		std::string generateChallenge();
		
};

