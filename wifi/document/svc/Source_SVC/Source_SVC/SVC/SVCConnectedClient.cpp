//
//  SVCConnectedClient.cpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "SVCConnectedClient.hpp"

SVCConnectedClient::SVCConnectedClient(SVCAuthInterface * auth,TCPSocket * csock)throw(SVCException){
    _diff = new DiffieHellman();
    _auth = auth;
    _sock = csock;
    _isNegociated = false;
}

SVCConnectedClient::~SVCConnectedClient(){
    delete _diff;
    if(_sock !=NULL)delete _sock;
    if(_ctx !=NULL)delete _ctx;
}

void SVCConnectedClient::negociate()throw(SVCException){
    uint32_t version = htonl(VERSION), clientVersion;
    size_t publicKeyLen,idLen,idClientLen,signLen,signClientLen,datalen, peerPublicKeyLen,secretlen, tmp,cipherlen;
    void ** id, *data, **sign, *idClient, *signClient,*cipher;
    unsigned char * publicKey, *peerPublicKey, *secret, *hash;
    unsigned char *key, *iv;
    unsigned char tag[TAG_SIZE];
    size_t copyLen;
    
    //Generate public key
    try{
        _diff->generateParameters();
        _diff->generatePkey();
        publicKeyLen = _diff->getPublicKey(&publicKey);
    }catch(DiffieHellmanException e){
        std::string s = "Error during negociation: Unable to get Diffie Hellman Public Key:\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_GET_PKEY);
    }
    
    //Get identity
    id =(void **) malloc(sizeof(void *));
    idLen = _auth->getIdentity(id);
    
    //Compute datalen
    signLen = _auth->getSignLen();
    datalen = sizeof(uint32_t) + 3*sizeof(size_t)+ publicKeyLen + idLen + signLen;
    
    //Alloc buffer data
    data = malloc(datalen);
    
    //Copy data into buffer
    copyLen = 0;
    memcpy(data, &version, sizeof(uint32_t));
    copyLen = sizeof(uint32_t);
    
    tmp = htonl(publicKeyLen);
    memcpy(((char *)data)+copyLen, &tmp, sizeof(size_t));
    copyLen +=sizeof(size_t);
    memcpy(((char *)data)+copyLen, publicKey, publicKeyLen);
    copyLen +=publicKeyLen;
    
    tmp = htonl(idLen);
    memcpy(((char *)data)+copyLen, &tmp, sizeof(size_t));
    copyLen +=sizeof(size_t);
    memcpy(((char *)data)+copyLen, *id, idLen);
    copyLen +=idLen;
    
    //Sign data
    try{
        sign = (void **)malloc(sizeof(void *));
        size_t len = _auth->sign(data, copyLen, sign);
        if(len != signLen){
            throw SVCException("Error during negociation : signature len doesn't match with size provided",false,ERR_SIGN_LEN);
        }
    }catch(SVCAuthInterfaceException e){
        std::string s = "Error during negociation : sign failed\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_SIGN);
    }
    
    //Add signature to data
    tmp = htonl(signLen);
    memcpy(((char *)data)+copyLen, &tmp, sizeof(size_t));
    copyLen +=sizeof(size_t);
    memcpy(((char *)data)+copyLen, *sign, signLen);
    copyLen +=signLen;
    
    //Send data
    try{
        uint32_t sendSize = htonl(datalen);
        _sock ->send(&sendSize, sizeof(uint32_t));
        _sock ->send(data, datalen);
    }catch(SocketException e){
        std::string s = "Error during negociation : can't send data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_SEND);
    }
    
    
    //Received PublicKey from peer
    try{
        uint32_t recvSize;
        size_t bytes_recv = 0;
        while(bytes_recv<sizeof(uint32_t)){
            bytes_recv+= _sock ->recv(&recvSize+bytes_recv, sizeof(uint32_t)-bytes_recv);
        }
        peerPublicKeyLen = ntohl(recvSize);
        peerPublicKey =(unsigned char *) malloc(peerPublicKeyLen);
        bytes_recv =0;
        while(bytes_recv<peerPublicKeyLen){
           bytes_recv += _sock->recv((char *)peerPublicKey+bytes_recv, peerPublicKeyLen-bytes_recv);
        }
        
    }catch(SocketException e){
        std::string s = "Error during negociation : can't received data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_RECV);
    }
    
    try{
        _diff->setPeerPublicKey(peerPublicKey, peerPublicKeyLen);
        _diff->generateCommonSecret();
        secretlen = _diff->getCommonSecret(&secret);
    }catch(DiffieHellmanException e){
        std::string s = "Error during negociation : unable to generate DF common secret\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DF_SECRET);
    }
    
    
    //Set key and iv
    hash = sha256(secret, secretlen);
    key = (unsigned char *)malloc(sizeof(char)*KEY_SIZE);
    iv = (unsigned char *)malloc(sizeof(char)*IV_SIZE);
    memcpy(key, hash, KEY_SIZE);
    memcpy(iv, hash+KEY_SIZE, IV_SIZE);
    _ctx = new CryptoSym(key,iv);
    
    //Free data pointer to reuse
    free(data);
    
    //Receive client encrypted data
    try{
        uint32_t recvSize;
        _sock->recv(&recvSize, sizeof(uint32_t));
        cipherlen = ntohl(recvSize);
        cipher = malloc(cipherlen+TAG_SIZE);
        _sock->recv(cipher, cipherlen+TAG_SIZE);
        
    }catch(SocketException e){
        std::string s = "Error during negociation : can't received data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_RECV);
    }
    
    
    //Decrypt data
    data = malloc(sizeof(char)*cipherlen);
    memcpy(&tag,(char *) cipher+cipherlen, TAG_SIZE);
    try{
        datalen = _ctx->decrypt(cipher, cipherlen, (unsigned char *)"", 0,tag, data);
    }catch(CryptoSymException e){
        std::string s = "Error during negociation : can't decrypt data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DECRYPT);
    }
    
    copyLen = 0;
    clientVersion=*((uint32_t *) data);
    clientVersion = ntohl(clientVersion);
    copyLen +=sizeof(uint32_t);
    if(clientVersion !=VERSION){
        throw SVCException("Client version differ from Server",false,ERR_VERSION);
    }
    
    memcpy(&idClientLen, (char *)data+copyLen, sizeof(size_t));
    idClientLen = ntohl(idClientLen);
    copyLen +=sizeof(size_t);
    
    idClient = (char*)data+copyLen;
    copyLen+=idClientLen;
    
    memcpy(&signClientLen, (char *)data+copyLen, sizeof(size_t));
    copyLen +=sizeof(size_t);
    signClientLen = ntohl(signClientLen);
    signClient = (char *)data+copyLen;
    copyLen +=signClientLen;
    
    
    try{
        if(!_auth->verifyIdentity(idClient, idClientLen)){
            throw SVCException("Error wrong client identity",false,ERR_WRONG_ID);
        }
    }catch(SVCAuthInterfaceException e){
        std::string s = "Error during identity verification :\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_AUTH);
    }
    
    try{
        if(!_auth->verifySign(idClient, idClientLen, signClient, signClientLen, data, (datalen-signClientLen-sizeof(size_t)))){
            throw SVCException("Error signature doesn't verify received data",false,ERR_SIGN_VERIFICATION);
        }
    }catch(SVCAuthInterfaceException e){
        std::string s = "Error during signature verification :\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_AUTH);
    }
    
    
    _isNegociated = true;
    
    //Free all pointer
    free(*id);
    free(id);
    free(data);
    free(*sign);
    free(sign);
    free(cipher);
    free(publicKey);
    free(secret);
    free(hash);
}