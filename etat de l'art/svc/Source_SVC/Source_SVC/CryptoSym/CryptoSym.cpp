//
//  CryptoSym.cpp
//  SVC
//
//  Created by Romain CYRILLE on 06/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "CryptoSym.hpp"


CryptoSym::CryptoSym(unsigned char * key, unsigned char * iv){
    _key = key;
    _iv = iv;
    
}

CryptoSym::~CryptoSym(){
    if(_key != NULL)delete _key;
    if(_iv!= NULL)delete _iv;
}


size_t CryptoSym::encrypt(void *plain, size_t plainlen, void *aad, size_t aadlen,void *cipher, void *tag)throw(CryptoSymException){
    EVP_CIPHER_CTX *ctx;
    
    int len;
    
    int cipherlen;
    //cipher = malloc(sizeof(char)*cipherlen);
    
    //Create and initialise the context
    if(!(ctx = EVP_CIPHER_CTX_new())){
        std::string err = "Error creating context ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    // Initialise the encryption operation.
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL)){
        std::string err = "Error initialising the encryption operation ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    // Initialise key and IV
    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, _key, _iv)){
        std::string err = "Error initialising key and IV ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    //Provide any AAD data. This can be called zero or more times as required
    if(1 != EVP_EncryptUpdate(ctx, NULL, &len,(unsigned char *) aad, aadlen)){
        std::string err = "Error providing AAD data ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    //Encrypting Data
    if(1 != EVP_EncryptUpdate(ctx, (unsigned char *)cipher, &len, (unsigned char *)plain, plainlen)){
        std::string err = "Error encrypting data ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    cipherlen = len;
    
    // Finalise the encryption.
    if(1 != EVP_EncryptFinal_ex(ctx, (unsigned char *)cipher + len, &len)){
        std::string err = "Error finalising encryption ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }
    cipherlen += len;
    
    //Get the tag
    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag)){
        std::string err = "Error getting tag ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    
    return cipherlen;
}

size_t CryptoSym::decrypt(void *cipher, size_t cipherlen, void *aad, size_t aadlen, void *tag, void *plain) throw(CryptoSymException)
{
    EVP_CIPHER_CTX *ctx;
    int len;
    int plainlen;
    int ret;
    
    //Create and initialise the context
    if(!(ctx = EVP_CIPHER_CTX_new())){
        std::string err = "Error initialising context ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    //Initialise the decryption operation.
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL)){
        std::string err = "Error initialising decryption operation ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }
    
      // Initialise key and IV
    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, _key, _iv)){
        std::string err = "Error setting key and iv ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }
    
    
    //Provide any AAD data.
    if(!EVP_DecryptUpdate(ctx, NULL, &len,(unsigned char *) aad, aadlen)){
        std::string err = "Error provinding AAD ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    //Decrypting Message
    if(!EVP_DecryptUpdate(ctx,(unsigned char *) plain, &len, (unsigned char *)cipher, cipherlen)){
        std::string err = "Error decrypting message ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    plainlen = len;
    
    //Set the expected tag
    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag)){
        std::string err = "Error setting tag ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());
    }

    
    //Finalising Decryption
    ret = EVP_DecryptFinal_ex(ctx, (unsigned char *)plain + len, &len);
    
    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);
    
    //Validation decryption
    if(ret > 0)
    {
        /* Success */
        plainlen += len;
        return plainlen;
    }
    else
    {
        std::string err = "Error validating decryption ";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw CryptoSymException(err,false,ERR_get_error());

    }
    
}

void CryptoSym::setIV(unsigned char * iv){
    free(_iv);
    _iv = iv;
}

void CryptoSym::setKey(unsigned char *key){
    free(_key);
    _key = key;
}
