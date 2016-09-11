//
//  DiffieHellmanException.hpp
//  SVC
//
//  Created by Romain CYRILLE on 05/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef DiffieHellmanException_hpp
#define DiffieHellmanException_hpp

#include <stdio.h>
#include <exception>
#include <string>
#include <string.h>
#include <errno.h>

class DiffieHellmanException: public std::exception {
    
private:
    std::string _userMessage;
    unsigned long _error;
    
public:
    /**
     *\fn DiffieHellmanException(const std::string &message, bool inclPerror = false)
     *\brief Construct a DiffieHellman with a explanatory message.
     *\param message explanatory message
     *\param inclPerror include (true) or not (false) message from perror
     *\param error define the OpenSSL error.
     */
    DiffieHellmanException(const std::string &message, bool inclPerror = false, unsigned long error = 0) throw();
    
    /**
     *\brief Destructor provived to guarantee that no exceptions ar thrown.
     */
    ~DiffieHellmanException() throw();
    
    /**
     *\brief Get the exeception message
     *\return exception message
     */
    const char *what();
};

#endif /* DiffieHellmanException_hpp */
