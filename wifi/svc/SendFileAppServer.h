#include "svc/authenticator/SVCAuthenticator.h"

class SendFileAppServer : SVCAuthenticator{

	public:
		SendFileAppServer();	
				
		//--	inherited interfaces
		std::string getIdentity();
		bool verifyIdentity(std::string identity, std::string challenge, std::string proof);
		std::string generateProof(std::string challenge);
		std::string generateChallenge();
		
}

