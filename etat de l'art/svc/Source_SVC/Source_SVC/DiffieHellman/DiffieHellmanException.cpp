 //
//  DiffieHellmanException.cpp
//  SVC
//
//  Created by Romain CYRILLE on 05/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "DiffieHellmanException.hpp"

DiffieHellmanException::DiffieHellmanException(const std::string &message, bool inclPerror, unsigned long error) throw():_userMessage(message){
    _error = error;
    if(inclPerror){
        _userMessage.append(": ");
        _userMessage.append(strerror(errno));
    }
}

DiffieHellmanException::~DiffieHellmanException() throw(){}

const char * DiffieHellmanException::what(){
    return _userMessage.c_str();
}