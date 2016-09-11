//
//  SVCAuthInterfaceException.cpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "SVCAuthInterfaceException.hpp"

SVCAuthInterfaceException::SVCAuthInterfaceException(const std::string &message, bool inclPerror, unsigned int error) throw():_userMessage(message){
    if(inclPerror){
        _error = error;
        _userMessage.append(": ");
        _userMessage.append(strerror(errno));
    }
}


SVCAuthInterfaceException::~SVCAuthInterfaceException() throw(){}


const char * SVCAuthInterfaceException::what(){
    return _userMessage.c_str();
}