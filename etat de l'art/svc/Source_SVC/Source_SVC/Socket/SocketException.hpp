#ifndef SocketException_hpp
#define SocketException_hpp
#include <stdio.h>
#include <stdexcept>
#include <string>
#include <string.h>
#include <errno.h>


class SocketException: public std::exception {
    
private:
    std::string _userMessage;
    
public:
    
    /**
     *\fn SocketException(const std::string &message, bool inclPerror = false)
     *\brief Construct a SocketException with a explanatory message.
     *\param message explanatory message
     *\param inclPerror include (true) or not (false) message from perror
     */
    SocketException(const std::string &message, bool inclPerror = false) throw();
    
    /**
     *\brief Destructor provived to guarantee that no exceptions ar thrown.
     */
    ~SocketException() throw();
    
    /**
     *\brief Get the exeception message
     *\return exception message
     */
    const char *what();
};
#endif /* SocketException_hpp */
