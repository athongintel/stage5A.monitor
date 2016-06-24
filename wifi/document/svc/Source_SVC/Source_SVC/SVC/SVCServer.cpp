//
//  SVCServer.cpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "SVCServer.hpp"

SVCServer::SVCServer(SVCAuthInterface *auth){
    _auth = auth;
    _sock =NULL;
}

SVCServer::~SVCServer(){
    if(_sock !=NULL)delete _sock;
}

void SVCServer::start(int port, int backlog)throw (SVCException){
    try{
        _sock = new TCPServerSocket(port,backlog);
    }
    catch(SocketException e){
        std::string s = "Error starting SVCServer:\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_SERVER_START);
    }
}

SVCConnectedClient * SVCServer::accept()throw(SVCException){
    SVCConnectedClient * client ;
    try{
        TCPSocket * csock = _sock->accept();
        client = new SVCConnectedClient(_auth,csock);
    }catch(SocketException e){
            std::string s = "Error acception client:\n\t";
            s.append(e.what());
            throw SVCException(s,false,ERR_ACCEPT);
    }
    return client;
}


void SVCServer::close(){
    _sock->close();
}

