#include <iostream>
#include <sys/socket.h>
#include <string>

using namespace std;

struct sockaddr_un daemonIPC;
int daemonSocket;
int htpSocket;

string path = "/tmp/svc-daemon";



int main(int argc, char** argv){
	
	
	memset(&daemonIPC, 0, sizeof(daemonIPC));
	daemonIPC.sun_family = AF_LOCAL;
	memcpy(daemonIPC.sun_path, path.c_str(), path.size());
	
	daemonSocket = socket(AF_LOCAL, SOCK_DGRAM, 0);
	//remove in case leftover
	unlink(daemonIPC.sun_path);
	
	if (bind(daemonSocket, (struct sockaddr*) &daemonIPC, sizeof(daemonIPC) == -1) {
        perror("Error: cannot bind svc-daemon");
        exit(1);
    }
    
    //constantly read from daemonSocket, build up packet and send to lower layer (UDP)?
	*/
	
	
	
	
}


