//
//  SVCException.hpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef SVCException_hpp
#define SVCException_hpp

#include <stdio.h>
#include <exception>
#include <string>
#include <errno.h>
#include <string.h>

const static int ERR_SERVER_START = 1;
const static int ERR_CONNECT = 2;
const static int ERR_GET_PKEY = 3;
const static int ERR_SIGN_LEN = 4;
const static int ERR_SIGN = 5;
const static int ERR_DATA_SEND = 6;
const static int ERR_DATA_RECV = 7;
const static int ERR_DF_SECRET = 8;
const static int ERR_HASH = 9;
const static int ERR_VERSION = 10;
const static int ERR_SIGN_VERIFICATION =11;
const static int ERR_WRONG_ID = 12;
const static int ERR_AUTH = 13;
const static int ERR_ENCRYPT = 14;
const static int ERR_DECRYPT = 15;
const static int ERR_PARAM_COPY = 16;
const static int ERR_NOT_NEGOCIED = 17;
const static int ERR_ACCEPT = 17;

class SVCException: public std::exception {
    
private:
    std::string _userMessage;
    int _error;
    
public:
    /**
     *\fn SVCException(const std::string &message, bool inclPerror = false)
     *\brief Construct a SVCException with a explanatory message.
     *\param message explanatory message
     *\param inclPerror include (true) or not (false) message from perror
     */
    SVCException(const std::string &message, bool inclPerror = false,int error = 0) throw();
    
    /**
     *\brief Destructor provived to guarantee that no exceptions ar thrown.
     */
    ~SVCException() throw();
    
    /**
     *\brief Get the exeception message
     *\return exception message
     */
    const char *what();
    
    int getError();
};

#endif /* SVCException_hpp */
