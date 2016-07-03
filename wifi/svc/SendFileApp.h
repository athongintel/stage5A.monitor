#include "svc/SVCApp.h"


class SendFileApp : SVCApp{

	public:
		SendFileApp();
		void setFile(string fileName);
		bool sendFile();
}

