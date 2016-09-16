//
//  CryptoSymException.hpp
//  SVC
//
//  Created by Romain CYRILLE on 06/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef CryptoSymException_hpp
#define CryptoSymException_hpp

#include <stdio.h>
#include <exception>
#include <string>
#include <errno.h>
#include <string.h>

class CryptoSymException: public std::exception {
    
private:
    std::string _userMessage;
    unsigned long _error;
    
public:
    
    /**
     *\fn CryptoSymException(const std::string &message, bool inclPerror = false, unsigned long error)
     *\brief Construct a DiffieHellman with a explanatory message.
     *\param message explanatory message
     *\param inclPerror include (true) or not (false) message from perror
     *\param error define the OpenSSL error.
     */
    CryptoSymException(const std::string &message, bool inclPerror = false, unsigned long error = 0);
    
    /**
     *\brief Destructor provived to guarantee that no exceptions ar thrown.
     */
    ~CryptoSymException() throw();
    
    /**
     *\brief Get the exeception message
     *\return exception message
     */
    const char *what();
};

#endif /* CryptoSymException_hpp */
