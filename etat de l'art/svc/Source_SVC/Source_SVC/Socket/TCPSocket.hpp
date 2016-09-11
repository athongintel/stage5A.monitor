//
//  TCPSocket.hpp
//  SVC
//
//  Created by Romain CYRILLE on 04/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef TCPSocket_hpp
#define TCPSocket_hpp

#include <stdio.h>
#include "Socket.hpp"
class TCPSocket: public Socket{
public:
    
    /**
     *\fn Socket(int sock)
     *\brief Simple constructor of a TCP socket
     *\exception SocketException thrown if unable to create a socket
     */
    TCPSocket()throw(SocketException);
    
    /**
     *\fn Socket(int sock)
     *\brief Constructor of a TCP socket
     *\param sock a C socket descriptor
     */
    TCPSocket(int sock);
    
    /**
     *\fn ~Socket()
     *\brief Destructor, do nothing special
     */
    ~TCPSocket();
    
    /**
     *\fn void connect(const string &foreignAddress, int foreignPort)
     *\brief Connect to the specified server.
     *\param foreignAddress a string contening the IP address of the server with the following format xxx.xxx.xxx.xxx
     *\param foreignPort the port on which connet on the server
     *\exception SocketException thrown if unable to connect to the server.
     */
    void connect(const std::string &foreignAddress, int foreignPort) throw (SocketException);
    
    /**
     *\fn String getForeignAddress()
     *\brief Return a string with the client's address.
     *\exception SocketException thrown if it can't get the client's address.
     */
    std::string getForeignAddress()throw(SocketException);
    
    /**
     *\fn unsigned short getForeignPort()
     *\brief Return the client's port number.
     *\exception SocketException thrown if it can't get the client's port.
     */
    unsigned short getForeignPort() throw(SocketException);
    
    /**
     *\fn void send(const void * buffer, long bufferlen)
     *\brief Send data on socket
     *\param buffer a pointer to the data to send
     *\param bufferlen the length of the data in bytes
     *\exception SocketException thrown if unable to send the data
     */
    void send(const void * buffer, long bufferlen) throw(SocketException);
    
    /**
     *\fn void recv(void * buffer, long bufferlen)
     *\brief Receive data on socket
     *\param buffer a pointer to an allocated space where to store the data received
     *\param bufferlen the length of the data to received in bytes
     *\exception SocketException thrown if unable to receive data
     */
    long recv(void * buffer, long bufferlen)throw(SocketException);
};

#endif /* TCPSocket_hpp */
