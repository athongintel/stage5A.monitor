//
//  SVCConnectedClient.hpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef SVCConnectedClient_hpp
#define SVCConnectedClient_hpp

#include <stdio.h>
#include "SVCCommunication.hpp"

class SVCConnectedClient : public SVCCommunication{
public:
    /**
     *\fn SVCConnectedClient(TCPSocket * csock,SVCAuthInterface * auth)
     *\brief Constructor of the class
     *\param diff the DiffieHellman instance used to negociate
     *\param auth the authentication interface provided
     *\param sock the Socket used to communication
     */
    SVCConnectedClient(SVCAuthInterface * auth,TCPSocket * csock)throw(SVCException);
    
    /**
     *\fn ~SVCConnectedClient
     *\brief Destructor of the class, close the socket and free attributes.
     */
    ~SVCConnectedClient();
    
    /**
     *\fn void negociate()
     *\brief Negociate the connexion with the other peer, Initalise the encryption context
     *\exception SVCException thrown if the negociation fail.
     */
    void negociate()throw(SVCException);
    
    
};
#endif /* SVCConnectedClient_hpp */
