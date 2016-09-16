//
//  Socket.cpp
//  SVC
//
//  Created by Romain CYRILLE on 03/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "Socket.hpp"


Socket::Socket(int type, int protocol) throw(SocketException){
    if((_sock = socket(AF_INET, type, protocol))== ERROR){
        throw SocketException("Error creating socket",true);
    }
}

Socket::Socket(int sock){
    _sock = sock;
}

Socket::~Socket(){
};

void Socket::close(){
    ::close(_sock);
}

