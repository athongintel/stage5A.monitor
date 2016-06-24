#ifndef Socket_hpp
#define Socket_hpp

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdexcept>
#include <string>
#include "SocketException.hpp"


const int ERROR = -1;

class Socket{
protected:
    int _sock;
    
    /**
     *\fn Socket(int type,int protocole) throw(SocketException)
     *\brief Constructor of a simple socket contening a C socket descriptor.
     *\param type Define the type of socket used, refer to C socket documentation
     *\param protocole define the protocol used, refer to C socket documentation
     *\exception SocketException thrown if unable to instance a socket descriptor
     */
    Socket(int type,int protocol)throw(SocketException);
    
    /**
     *\fn Socket(int sock)
     *\brief Constructor of a simple socket contening a C socket descriptor.
     *\param type a C socket descriptor
     */
    Socket(int sock);
    
    /**
     *\fn ~Socket()
     *\brief Destructor, do nothing special
     */
    ~Socket();
    
public:
    
    /**
     *\fn void close()
     *\brief Close the socket
     */
    void close();
};
#endif /* Socket_hpp */