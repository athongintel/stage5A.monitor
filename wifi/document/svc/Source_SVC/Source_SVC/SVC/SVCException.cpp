//
//  SVCException.cpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "SVCException.hpp"


SVCException::SVCException(const std::string &message, bool inclPerror, int error) throw():_userMessage(message){
    if(inclPerror){
        _userMessage.append(": ");
        _userMessage.append(strerror(errno));
    }
     _error = error;
}

SVCException::~SVCException() throw(){}

const char * SVCException::what(){
    return _userMessage.c_str();
}

int SVCException::getError(){
    return _error;
}