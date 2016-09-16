//
//  CryptoSym.hpp
//  SVC
//
//  Created by Romain CYRILLE on 06/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef CryptoSym_hpp
#define CryptoSym_hpp

#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/sha.h>
#include "CryptoSymException.hpp"


const static size_t KEY_SIZE = 16;
const static size_t IV_SIZE = 12;
const static size_t TAG_SIZE = 16;

class CryptoSym{
private:
    unsigned char * _key;
    unsigned char * _iv;
    
public:
    
    /**
     *\fn CryptoSym(unsigned char * key, unsigned char * iv)
     *\brief Contructor of the class, init the cypher context with key and iv.
     *\param key the key for encryption and decryption, it need to be at least 128 bits long.
     *\param iv the iv for encryption and decryption, it need to be at least 96 bits long.
     *\exception CryptoSymException thrown if unable to init context or if key or iv are too short.
     */
    CryptoSym(unsigned char * key, unsigned char * iv);
    
    /**
     *\fn ~CryptoSym
     *\brief Destructor of the class
     */
    ~CryptoSym();

    /**
     *\fn int encrypt(void *plain, size_t plainlen, void *aad, size_t aadlen,void *cipher, void *tag)
     *\brief Encrypt any data with AES 128 bit GCM.
     *\param plain poiter to the plain data
     *\param plainlen size of the plain data in bytes
     *\param aad pointer to additionnal authenticated data
     *\param addlen size of the aad in bytes
     *\param cipher the pointer where the cipher data will be stored, must be the same size as the plain data
     *\param tag the MAC tag generated during the encryption
     *\return the size in bytes of the cipher data
     *\exception CryptoSymException if error occur during encryption.
     */
    size_t encrypt(void *plain, size_t plainlen, void *aad, size_t aadlen,void *cipher, void *tag)throw(CryptoSymException);
    
    /**
     *\fn int decrypt(void *cipher, size_t cipherlen, void *aad, size_t aadlen, void *tag, void *plain)
     *\brief Decrypt any data with AES 128 bit GCM
     *\param cipher the encrypted data
     *\param cipherlen the size of the encrypted data in bytes
     *\param aad pointer to the retrieved additionnal authenticated data
     *\param aadlen size of the retrieved aad in bytes
     *\param plain a pointer where the plain data will be stored, must be the same size as the cipher data
     *\param tag the mac tag generated during decryption
     *\return the size in bytes of the plain data
     *\exception CryptoSymException if error occur during decrytion, including if the plain authentication has failed.
     */
    size_t decrypt(void *cipher, size_t cipherlen, void *aad, size_t aadlen, void *tag, void *plain) throw(CryptoSymException);
    
    /**
     *\fn void setKey(unsigned char * key)
     *\brief Change the key used for cryptography operations
     *\param key the key
     */
    void setKey(unsigned char * key);
    
    /**
     *\fn void setIV(unsigned char * iv)
     *\brief Change the iv used for cryptography operations
     *\param iv the initialisation vector
     */
    void setIV(unsigned char * iv);
    
    };
#endif /* CryptoSym_hpp */
