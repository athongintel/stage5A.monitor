#include "SocketException.hpp"

SocketException::SocketException(const std::string &message, bool inclPerror) throw(): _userMessage(message){
    if(inclPerror){
        _userMessage.append(": ");
        _userMessage.append(strerror(errno));
    }
}

SocketException::~SocketException() throw(){}

const char *SocketException::what(){
    return _userMessage.c_str();
}