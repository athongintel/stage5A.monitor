//
//  SVCCommunication.cpp
//  SVC
//
//  Created by Romain CYRILLE on 15/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "SVCCommunication.hpp"

SVCCommunication::SVCCommunication(){
    recvbuffer = NULL;
    recvlen = 0;
    recvcopied = 0;
}

size_t SVCCommunication::send(void * buffer,size_t bufferlen)throw(SVCException){
    if(!_isNegociated)throw SVCException("Connexion not negociated",false,ERR_NOT_NEGOCIED);
    void * cipher;
    size_t cipherlen;
    void * tag = malloc(TAG_SIZE);
    
    //Encrypt data
    cipher = malloc(bufferlen + TAG_SIZE);
    try{
        cipherlen = _ctx->encrypt(buffer, bufferlen, (unsigned char *)"", 0, cipher, tag);
        unsigned char * iv = (unsigned char *)malloc(IV_SIZE);
        memcpy(iv, tag, IV_SIZE);
        _ctx->setIV(iv);
    }catch(CryptoSymException e){
        std::string s = "Error while sending data: can't encrypt data\n\t";
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
        std::string s = "Error: can't send data over socket\n\t";
        s.append(e.what());
        throw SVCException(s,false,ERR_DATA_SEND);
    }
    free(cipher);
    free(tag);
    return cipherlen;
}

size_t SVCCommunication::recv(void * buffer,size_t bufferlen)throw(SVCException){
    if(!_isNegociated)throw SVCException("Connexion not negociated",false,ERR_NOT_NEGOCIED);
    size_t cipherlen;
    void * cipher, *tag;
    size_t copied = 0;
    
    //If buffer is empty get data
    while(copied <bufferlen){
        
        if(recvlen==0){
            recvcopied = 0;
            tag = malloc(TAG_SIZE);
            
            //Receive client encrypted data
            try{
                uint32_t recvSize;
                int recvlen = 0;
                
                size_t bytes_recv=0;
                while(bytes_recv<sizeof(uint32_t)){
                    bytes_recv+= _sock ->recv(&recvSize+bytes_recv, sizeof(uint32_t)-bytes_recv);
                }

                cipherlen = ntohl(recvSize);
                cipher = malloc(cipherlen+TAG_SIZE);
                while(recvlen<cipherlen+TAG_SIZE){
                    recvlen +=_sock->recv((char *)cipher+recvlen, cipherlen+TAG_SIZE-recvlen);
                }
                
            }catch(SocketException e){
                std::string s = "Error: can't received data\n\t";
                s.append(e.what());
                throw SVCException(s,false,ERR_DATA_RECV);
            }
            
            
            //Decrypt data
            recvbuffer = malloc(sizeof(char)*cipherlen);
            memcpy(tag,(char *) cipher+cipherlen, TAG_SIZE);
            try{
                recvlen = _ctx->decrypt(cipher, cipherlen, (unsigned char *)"", 0,tag, recvbuffer);
                unsigned char * iv = (unsigned char *)malloc(IV_SIZE);
                memcpy(iv, tag, IV_SIZE);
                _ctx->setIV(iv);
                
            }catch(CryptoSymException e){
                std::string s = "Error while data reception: can't decrypt data\n\t";
                s.append(e.what());
                throw SVCException(s,false,ERR_DECRYPT);
            }
            free(tag);
        }
        if(bufferlen<=(recvlen+copied)){
            memcpy((char *)buffer+copied, (char *)recvbuffer+recvcopied, bufferlen-copied);
            recvcopied += bufferlen-copied;
            recvlen =recvlen - (bufferlen-copied);
            if(recvlen ==0)free(recvbuffer);
            copied = bufferlen;
        }
        else{
            memcpy((char *)buffer+copied, (char *)recvbuffer+recvcopied, recvlen-recvcopied);
            recvlen = 0;
            free(recvbuffer);
            copied +=recvlen;
        }

    }
    return copied;
}


void SVCCommunication::close(){
    _sock->close();
}

unsigned char * SVCCommunication::sha256(const void * data, size_t len)throw(SVCException){
    unsigned char * hash;
    SHA256_CTX ctx;
    hash = (unsigned char *)malloc(sizeof(char)*SHA256_DIGEST_LENGTH);
    if(SHA256_Init(&ctx)!=1){
        std::string err = "Error Init SHA256 context:  ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw SVCException(err,false,ERR_HASH);
    }
    if(SHA256_Update(&ctx, data, len)!=1){
        std::string err = "Error digest SHA256: ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw SVCException(err,false,ERR_HASH);
    }
    if(SHA256_Final(hash, &ctx)!=1){
        std::string err = "Error Final SHA256: ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw SVCException(err,false,ERR_HASH);
    }
    return hash;
}

std::string SVCCommunication::getForeignAddress()throw(SVCException){
    std::string address;
    try{
        address = _sock->getForeignAddress();
    }catch(SocketException e){
        std::string s = "Error:\n\t";
        s.append(e.what());
        throw SVCException(s,false,0);
    }
    return address;
}

unsigned short SVCCommunication::getForeignPort()throw(SVCException){
    unsigned short port;
    try{
        port = _sock->getForeignPort();
    }catch(SocketException e){
        std::string s = "Error:\n\t";
        s.append(e.what());
        throw SVCException(s,false,0);
    }
    return port;
}