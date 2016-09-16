//
//  TCPServerSocket.cpp
//  SVC
//
//  Created by Romain CYRILLE on 04/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "TCPServerSocket.hpp"

TCPServerSocket::TCPServerSocket(int port, int backlog) throw(SocketException): Socket(SOCK_STREAM,0){
    sockaddr_in sin;
    socklen_t socklen = sizeof(sin);
    
    /* Server Socket Configuration */
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    
    /* Server Socket Binding */
    if(bind(_sock, (sockaddr*)&sin, socklen)== ERROR){
        throw SocketException("Error Binding Socket",true);
    }
    
    /* Server start listening */
    if(listen(_sock,backlog)==ERROR){
        throw SocketException("Error Starting Server",true);
    }
}

TCPServerSocket::TCPServerSocket(const std::string &localAddress, int port, int backlog) throw(SocketException):Socket(SOCK_STREAM,0){
    sockaddr_in sin;
    socklen_t socklen = sizeof(sin);
    
    /* Server Socket Configuration */
    sin.sin_addr.s_addr = inet_addr(localAddress.c_str());
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    
    /* Server Socket Binding */
    if(bind(_sock, (sockaddr*)&sin, socklen)== ERROR){
        throw SocketException("Error Binding Socket",true);
    }
    
    /* Server start listening */
    if(listen(_sock,backlog)==ERROR){
        throw SocketException("Error Starting Server",true);
    }
}

TCPSocket * TCPServerSocket::accept() throw(SocketException){
    int csock;
    if((csock=::accept(_sock, NULL, 0))<0){
        throw SocketException("Accept Failed",true);
    }
    return new TCPSocket(csock);
}