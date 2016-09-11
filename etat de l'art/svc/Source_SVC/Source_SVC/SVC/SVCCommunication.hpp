//
//  SVCCommunication.hpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef SVCCommunication_hpp
#define SVCCommunication_hpp

#include <stdio.h>
#include <iostream>
#include "SVCAuthInterface.hpp"
#include "DiffieHellman.hpp"
#include "SVCException.hpp"
#include "CryptoSym.hpp"
#include "TCPSocket.hpp"


const static int VERSION = 10;


class SVCCommunication{

protected:
    bool _isNegociated;
    DiffieHellman * _diff;
    SVCAuthInterface * _auth;
    TCPSocket * _sock;
    void * recvbuffer;
    size_t recvcopied;
    size_t recvlen;
    CryptoSym * _ctx;
    
    
    unsigned char * sha256(const void * data, size_t len)throw(SVCException);
    
public:

    /**
     *\fn SVCCommunication()
     *\brief Default Constructor
     */
    SVCCommunication();
    
    /**
     *\fn size_t send(const void * buffer,ssize_t bufferlen)
     *\brief Transmit message to another SVCCommunication
     *\param buffer a pointer to any data
     *\param bufferlen the size in bytes of the data to send
     *\return the number of bytes that was sent
     *\exception SVCException thrown in case of error;
     */
    size_t send(void * buffer,size_t bufferlen)throw(SVCException);
    
    /**
     *\fn size_t recv(const void * buffer,ssize_t bufferlen)
     *\brief Receive message from another SVCCommunication
     *\param buffer a pointer to any allocated memory
     *\param bufferlen the number of bytes to reveived
     *\return the number of bytes received
     *\exception SVCException thrown in case of error;
     */
    size_t recv(void * buffer,size_t bufferlen)throw(SVCException);
    
    /**
     *\fn void negociate()
     *\brief Negociate the connexion with the other peer, Initalise the encryption context
     *\exception SVCException thrown if the negociation fail.
     */
    virtual void negociate()throw(SVCException) = 0;
    
    /**
     *\fn void close()
     *\brief Close the connexion with the other peer.
     */
    void close();
    
    /**
     *\fn String getForeignAddress()
     *\brief Return a string with the client's address.
     *\exception SVCException thrown if it can't get the client's address.
     */
    std::string getForeignAddress()throw(SVCException);
    
    /**
     *\fn unsigned short getForeignPort()
     *\brief Return the client's port number.
     *\exception SVCException thrown if it can't get the client's port.
     */
    unsigned short getForeignPort()throw(SVCException);
    
};

#endif /* SVCCommunication_hpp */
