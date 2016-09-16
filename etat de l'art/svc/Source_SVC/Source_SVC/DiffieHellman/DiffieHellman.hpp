//
//  DiffieHellman.hpp
//  SVC
//
//  Created by Romain CYRILLE on 05/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef DiffieHellman_hpp
#define DiffieHellman_hpp

#include <stdio.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/err.h>
#include <string>
#include "DiffieHellmanException.hpp"

class DiffieHellman{
private:
    EVP_PKEY * _pKey;
    EVP_PKEY * _peerPublicKey;
    EVP_PKEY * _param;
    unsigned char * _commonSecret;
    size_t * _secretLen;
    
public:
    /**
     *\fn DiffieHellman
     *\brief Constructor of the class, initialise the attributes.
     *\exception DiffieHellmanException thrown if initialisation of one attribute failed.
     */
    DiffieHellman()throw(DiffieHellmanException);
    
    
    /**
     *\fn DiffieHellman
     *\brief Destructor of the class, deallocates the attributes.
     */
    ~DiffieHellman();
    
    /**
     *\fn void generateParameters()
     *\brief Generate Diffie Hellman parameters for the key generation and store it in the param attribute.
     *\exception DiffieHellmanException thrown if generation of parameters failed.
     */
    void generateParameters()throw(DiffieHellmanException);
    
    /**
     *\fn void generatePkey()
     *\brief Generate Diffie Hellman public and private key for the key exchange and store it it the pKey attribute.
     *\exception DiffieHellmanException thrown if generation of parameters failed.
     */
    void generatePkey()throw(DiffieHellmanException);
    
    /**
     *\fn void setPeerPublicKey()
     *\brief Set the peer public key in the peerPublicKey attribute.
     *\param peerPublicKey the serialized peer Public Key
     *\param len the len of the serialized key
     *\exception DiffieHellmanException thrown if unable to set the PeerPublicKey
     */
    void setPeerPublicKey(unsigned char * peerPublicKey, size_t len) throw(DiffieHellmanException);
    
    /**
     *\fn unsigned char * generateCommonSecret()
     *\brief Generate the common secret from the differents keys and store it in the commonSecret attribute,
        the peer public key have to be set before.
     *\exception DiffieHellmanException thrown if secret's generation failed or if peer public key have not been set.
     */
    void generateCommonSecret()throw(DiffieHellmanException);
    
    /**
     *\fn size_t getPublicKey(unsigned char ** publicKey)
     *\brief Extract the public key in a serialized form ready to send.
     *\param publicKey a pointer where the publicKey will be stored, it is allocated by the function.
     *\return the length of serialized public key
     *\exception DiffieHellmanException thrown if unable to get the public key
     */
    size_t getPublicKey(unsigned char ** publicKey)throw(DiffieHellmanException);
    
    
    /**
     *\fn size_t getCommonSecret(unsigned char ** secret)
     *\brief Getter for the common secret.
     *\param secret a pointer to the secret
     *\return the length of the secret
     *\exception DiffieHellmanException thrown if secret have not been generated before.
     */
    size_t getCommonSecret(unsigned char ** secret);
    
};

#endif /* DiffieHellman_hpp */
