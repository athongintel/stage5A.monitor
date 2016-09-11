//
//  CryptoSymException.cpp
//  SVC
//
//  Created by Romain CYRILLE on 06/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "CryptoSymException.hpp"

CryptoSymException::CryptoSymException(const std::string &message, bool inclPerror, unsigned long error):_userMessage(message){
    _error = error;
    if(inclPerror){
        _userMessage.append(": ");
        _userMessage.append(strerror(errno));
    }
}

CryptoSymException::~CryptoSymException() throw(){}

const char * CryptoSymException::what(){
    return _userMessage.c_str();
}