//
//  TCPServerSocket.hpp
//  SVC
//
//  Created by Romain CYRILLE on 04/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef TCPServerSocket_hpp
#define TCPServerSocket_hpp

#include <stdio.h>
#include "Socket.hpp"
#include "TCPSocket.hpp"
#include "SocketException.hpp"

class TCPServerSocket: public Socket{
public:
    /**
     *\fn TCPServerSocket(int port, int queueLen)
     *\brief Constructor of a TCP socket to use as a server for incomming connexion from any address
     *\param port port of the server socket
     *\param backlog maximum queue length for incomming connexion, set to 5 by default
     *\exception SocketException thrown if unable to create 
     */
    TCPServerSocket(int port, int backlog = 5) throw(SocketException);
    
    /**
     *\fn TCPServerSocket(const string localAddress, int port, int queueLen)
     *\brief Constructor of a TCP socket to use as a server for incomming connexion from a specified address
     *\param localAddress local interface address of server socket
     *\param port port of the server socket
     *\param backlog maximum queue length for incomming connexion, set to 5 by default
     *\exception SocketException thrown if unable to create
     */
    TCPServerSocket(const std::string &localAddress, int port, int backlog = 5) throw(SocketException);
    
    /**
     *\fn TCPSocket accept() throw(SocketException)
     *\brief Block until a new connection is established on the socket     
     *\exception SocketException thrown if attempt to accept a new connection failed
     */
    TCPSocket * accept() throw(SocketException);
   
};

#endif /* TCPServerSocket_hpp */
