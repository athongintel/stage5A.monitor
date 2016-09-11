//
//  SVCAuthInterfaceException.hpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef SVCAuthInterfaceException_hpp
#define SVCAuthInterfaceException_hpp

#include <stdio.h>
#include <exception>
#include <string>
#include <errno.h>
#include <string.h>

const static int ERR_DATA_SIGN = 1;
const static int ERR_VERIFY_SIGN = 2;
const static int ERR_VERIFY_ID = 3;

class SVCAuthInterfaceException: public std::exception {
    
private:
    std::string _userMessage;
    unsigned int _error;
    
public:
    /**
     *\fn SVCException(const std::string &message, bool inclPerror = false)
     *\brief Construct a SVCException with a explanatory message.
     *\param message explanatory message
     *\param inclPerror include (true) or not (false) message from perror
     */
    SVCAuthInterfaceException(const std::string &message, bool inclPerror = false, unsigned int error = 0) throw();
    
    /**
     *\brief Destructor provived to guarantee that no exceptions ar thrown.
     */
    ~SVCAuthInterfaceException() throw();
    
    /**
     *\brief Get the exeception message
     *\return exception message
     */
    const char *what();
};


#endif /* SVCAuthInterfaceException_hpp */
