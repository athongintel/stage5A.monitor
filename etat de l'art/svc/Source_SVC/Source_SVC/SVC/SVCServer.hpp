//
//  SVCServer.hpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef SVCServer_hpp
#define SVCServer_hpp

#include <stdio.h>
#include "DiffieHellman.hpp"
#include "SVCAuthInterface.hpp"
#include "SVCException.hpp"
#include "TCPServerSocket.hpp"
#include "SVCConnectedClient.hpp"


class SVCServer{
private:
    SVCAuthInterface * _auth;
    TCPServerSocket * _sock;
    
public:
    /**
     *\fn SVCServer(SVCAuthInterface * auth)
     *\brief Constructor of the class to create a SVCServer, generates Diffie Hellman parameters
     *\param auth the authentication interface provided to the Server
     */
    SVCServer(SVCAuthInterface *auth);
    
    /**
     *\fn ~SVCServer
     *\brief Destructor of the class,close the socket and free attribute.
     */
    ~SVCServer();
    
    /**
     *\fn void start(int port, int backlog)
     *\brief Start the SVC server
     *\param port port of the server socket
     *\param backlog maximum queue length for incomming connexion, set to 5 by default
     *\exception SVCException thrown if unable to start Server
     */
    void start(int port, int backlog = 5)throw (SVCException);
    
    /**
     *\fn SVCConnectedClient accept()
     *\brief Accept a connexion from a SVC client
     *\return a SVCConnectedClient contening the parameters, the auth interface and the socket of the client
     *\exception SVCException thrown if unable to accept the incomming connexion or unable to copy Diffie Hellman parameters
     */
    SVCConnectedClient * accept()throw(SVCException);
    
    /**
     *\fn void generateNewParameters()
     *\brief Generate new Diffie Hellman parametres that will be used for new connexion
     *\exception SVCException thrown if unable to generate new parameters.
     */
    void generateNewParameters(SVCException);
    
    /**
     *\fn void close()
     *\brief Close the server socket
     */
    void close();
    
};

#endif /* SVCServer_hpp */
