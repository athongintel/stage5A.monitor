//
//  SVCClient.cpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "SVCClient.hpp"

#include <iostream>

SVCClient::SVCClient(SVCAuthInterface *auth){
    _auth = auth;
    _isNegociated = false;
    _diff = new DiffieHellman();
    _ctx = NULL;
}

SVCClient::~SVCClient(){
    if(_ctx !=NULL)delete _ctx;
}

void SVCClient::connect(const std::string foreignAddress, int foreignPort)throw(SVCException){
    try{
        _sock = new TCPSocket();
        _sock->connect(foreignAddress,foreignPort);
    }catch(SocketException e){
        std::string s = "Error connecting to Server:\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_CONNECT);
    }
}


void SVCClient::negociate()throw(SVCException){
    uint32_t version = htonl(VERSION), serverVersion;
    size_t publicKeyLen,idLen,idServerLen,signLen,signServerLen,datalen, peerPublicKeyLen,secretlen, tmp,cipherlen;
    void ** id, *data, **sign, *idServer, *signServer,*cipher;
    unsigned char * publicKey, *peerPublicKey, *secret, *hash;
    unsigned char *key, *iv;
    unsigned char tag[TAG_SIZE];
    size_t copyLen;
    
    //Received data
    try{
        uint32_t recvSize;
        size_t bytes_recv=0;
        while(bytes_recv<sizeof(uint32_t)){
              bytes_recv+= _sock ->recv(&recvSize+bytes_recv, sizeof(uint32_t)-bytes_recv);
        }
       
        datalen = ntohl(recvSize);
        data = malloc(datalen);
        bytes_recv =0;
        while(bytes_recv<datalen){
            bytes_recv += _sock->recv((char *)data+bytes_recv, datalen-bytes_recv);
        }
    }catch(SocketException e){
        std::string s = "Error during negociation : can't receive data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_SEND);
    }
    copyLen = 0;
    serverVersion = *((uint32_t*)data);
    copyLen +=sizeof(uint32_t);
    serverVersion = ntohl(serverVersion);
    if(serverVersion !=VERSION){
        throw SVCException("Server version differ from Client",false,ERR_VERSION);
    }
    
    memcpy(&peerPublicKeyLen, (char *)data+copyLen, sizeof(size_t));
    copyLen += sizeof(size_t);
    peerPublicKeyLen = ntohl(peerPublicKeyLen);
    peerPublicKey = (unsigned char *)data+copyLen;
    copyLen +=peerPublicKeyLen;
    
    memcpy(&idServerLen, (char *)data+copyLen, sizeof(size_t));
    copyLen += sizeof(size_t);
    idServerLen = ntohl(idServerLen);
    idServer = (char *)data +copyLen;
    copyLen += idServerLen;
    
    memcpy(&signServerLen, (char *)data+copyLen, sizeof(size_t));
    copyLen += sizeof(size_t);
    signServerLen = ntohl(signServerLen);
    
    signServer = (char *)data+copyLen;
    copyLen +=signServerLen;
    
    //Verify Identity
    try{
        if(!_auth->verifyIdentity(idServer, idServerLen)){
            throw SVCException("Error wrong server identity",false,ERR_WRONG_ID);
        }
    }catch(SVCAuthInterfaceException e){
        std::string s = "Error during identity verification :\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_AUTH);
    }
    
    try{
        if(!_auth->verifySign(idServer, idServerLen, signServer, signServerLen, data, datalen-signServerLen-sizeof(size_t))){
            throw SVCException("Error signature doesn't verify received data",false,ERR_SIGN_VERIFICATION);
        }
    }catch(SVCAuthInterfaceException e){
        std::string s = "Error during signature verification :\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_AUTH);
    }

    //Generate Public key and commonSecret
    try{
        _diff->generateParameters();
        _diff->setPeerPublicKey(peerPublicKey, peerPublicKeyLen);
        _diff->generatePkey();
        _diff->generateCommonSecret();
    }catch(DiffieHellmanException e){
        std::string s = "Error during negociation : unable to generate DF common secret\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DF_SECRET);
    }
    
    secretlen = _diff->getCommonSecret(&secret);
    publicKeyLen = _diff->getPublicKey(&publicKey);
    
    //Send public key
    try{
        uint32_t sendSize = htonl(publicKeyLen);
        _sock ->send(&sendSize, sizeof(uint32_t));
        _sock ->send(publicKey,publicKeyLen);
    }catch(SocketException e){
        std::string s = "Error during negociation : can't send data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_SEND);
    }
    
    //Generate key and iv
    hash = sha256(secret, secretlen);
    key = (unsigned char *)malloc(sizeof(char)*KEY_SIZE);
    iv = (unsigned char *)malloc(sizeof(char)*IV_SIZE);
    memcpy(key, hash, KEY_SIZE);
    memcpy(iv, hash+KEY_SIZE, IV_SIZE);
    _ctx = new CryptoSym(key,iv);
    
    //compute datalen and alloc data
    free(data);//reuse data pointer
    signLen = _auth->getSignLen();
    id = (void **)malloc(sizeof(void*));
    idLen = _auth->getIdentity(id);
    datalen = sizeof(uint32_t) + 2*sizeof(size_t)+ idLen + signLen;
    data = malloc(datalen);
    
    //Send version, id and signature
    copyLen = 0;
    memcpy(data, &version, sizeof(uint32_t));
    copyLen += sizeof(uint32_t);
    
    tmp = htonl(idLen);
    memcpy((char *)data +copyLen, &tmp, sizeof(size_t));
    copyLen += sizeof(size_t);
    memcpy((char * )data+copyLen, *id, idLen);
    copyLen += idLen;
    
    //Sign data
    try{
        sign = (void **)malloc(sizeof(void*));
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
    
    //Encrypt data
    cipher = malloc(datalen + TAG_SIZE);
    try{
        cipherlen = _ctx->encrypt(data, datalen, (unsigned char *)"", 0, cipher, tag);
    }catch(CryptoSymException e){
        std::string s = "Error during negociation : can't encrypt data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_ENCRYPT);
    }
    
    //add tag at the end of cipher
    memcpy((char *)cipher + cipherlen, tag, TAG_SIZE);
    
    //Send cipher
    try{
        uint32_t sendSize = htonl(cipherlen);
        _sock ->send(&sendSize, sizeof(uint32_t));
        _sock ->send(cipher, cipherlen+TAG_SIZE);
    }catch(SocketException e){
        std::string s = "Error during negociation : can't send data\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_SEND);
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


