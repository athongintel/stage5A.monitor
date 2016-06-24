//
//  TCPSocket.cpp
//  SVC
//
//  Created by Romain CYRILLE on 04/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "TCPSocket.hpp"


TCPSocket::TCPSocket()throw(SocketException): Socket(SOCK_STREAM,0){}

TCPSocket::TCPSocket(int sock):Socket(sock){};

TCPSocket::~TCPSocket(){};


void TCPSocket::connect(const std::string &foreignAddress, int foreignPort) throw(SocketException){
    sockaddr_in sin;
    socklen_t socklen = sizeof(sin);
    
    /* Socket Configuration */
    sin.sin_addr.s_addr = inet_addr(foreignAddress.c_str());
    sin.sin_port = htons(foreignPort);
    sin.sin_family = AF_INET;
    
    /* Connection to the Server */
    if(::connect(_sock, (sockaddr*)&sin, socklen) == ERROR){
        throw SocketException("Error Connecting to the server",true);
    }
}


void TCPSocket::send(const void * buffer, long bufferlen)throw(SocketException){
    if(::send(_sock,buffer,bufferlen,0)<0){
        throw SocketException("Send failed",true);
    }
}

std::string TCPSocket::getForeignAddress()throw(SocketException) {
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);
    
    if (getpeername(_sock, (sockaddr *) &addr,(socklen_t *) &addr_len) < 0) {
        throw SocketException("Fetch of foreign address failed", true);
    }
    return inet_ntoa(addr.sin_addr);
}

unsigned short TCPSocket::getForeignPort() throw(SocketException) {
    sockaddr_in addr;
    unsigned int addr_len = sizeof(addr);
    
    if (getpeername(_sock, (sockaddr *) &addr, (socklen_t *) &addr_len) < 0) {
        throw SocketException("Fetch of foreign port failed", true);
    }
    return ntohs(addr.sin_port);
}

long TCPSocket::recv(void * buffer, long bufferlen)throw (SocketException){
    long rtn;
    if((rtn=::recv(_sock, buffer, bufferlen, 0))<=0){
        throw SocketException("Received failed",true);
    }
    return rtn;
}