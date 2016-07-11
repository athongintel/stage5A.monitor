

class SVCAuthenticator{

	public:
		virtual string getIdentity();
		virtual bool verifyIdentity(string identity, string proof);
		virtual string generateProof(string challenge);	
		virtual string generateChalenge(string nonce);
};
