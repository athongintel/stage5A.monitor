//
//  SVCClient.hpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef SVCClient_hpp
#define SVCClient_hpp

#include <stdio.h>
#include "SVCCommunication.hpp"

class SVCClient: public SVCCommunication{
public:
    
    /**
     *\fn SVCClient(SVCAuthInterface auth)
     *\brief Constructor of the class, create a SVCClient with a given authentication interface
     *\param auth the authentication interface provided
     */
    SVCClient(SVCAuthInterface * auth);
    
    /**
     *\fn ~SVCClient
     *\brief Destructor of the class, close the socket and free attributes.
     */
    ~SVCClient();
    
    /**
     *\fn void negociate()
     *\brief Negociate the connexion with the other peer, Initalise the encryption context
     *\exception SVCException thrown if the negociation fail.
     */
    void negociate()throw(SVCException);
    
    /**
     *\fn void connect(const std::string foreignAddress, int foreignPort)
     *\brief Connect to the SVCServer identify with a specified address and port
     *\param foreignAddress the address of the server
     *\return foreignPort the port of the server
     *\exception SVCException thrown if unable to connect to the server
     */
    void connect(const std::string foreignAddress, int foreignPort)throw(SVCException);
    
    
};
#endif /* SVCClient_hpp */
