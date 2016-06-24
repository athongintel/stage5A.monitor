//
//  AuthPKI.hpp
//  SVC
//
//  Created by Ludovic POUJOL on 17/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef AuthPKI_hpp
#define AuthPKI_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>

#include "SVCAuthInterface.hpp"

class AuthPKI : public SVCAuthInterface{
private:
   
    X509 *ca_cert;
    
    X509 *my_cert;
    RSA  *my_key;
    
    X509 *peer_cert;
    RSA  *peer_key;
    
    bool checkCertificateWithCA(X509* certificate);
    
public:
    
    /**
     *\fn AuthPKI(std::string identName)
     *\brief Constructor of a simple AuthPKI class implementing the SVCAuthInterface
     *\param identName Define the identity (cert+key) to load for the authentification with the peer
     */
    AuthPKI(std::string identName);
    
    /**
     *\fn ~AuthPKI()
     *\brief Destructor, do nothing special
     */
    ~AuthPKI();
    
    /**
     *\fn size_t getIdentity(void** identity)
     *\brief Give the identity loaded by the constructor of AuthPKI for the SVC library
     *\param identity Where to write the pointer to the allocated memory containing the identity to use for auth
     *\return the size in bytes the identity given
     */
    size_t getIdentity(void** identity);
    
    /**
     *\fn bool verifyIdentity(void* identity, size_t identitylen)throw(SVCAuthInterfaceException)
     *\brief Give a way to validate the identity received from the peer
     *\param identity The identity to validate
     *\param identitylen The size of the identity
     *\return whether if the identity is valid (true) or not (false)
     *\exception SVCAuthInterfaceException thrown if the identity given isn't a PEM certificate
     */
    bool verifyIdentity(void* identity, size_t identitylen)throw(SVCAuthInterfaceException);
    
    /**
     *\fn size_t sign(void* buffer, size_t bufferlen, void** signature)throw(SVCAuthInterfaceException);
     *\brief Sign the given buffer with the identity loaded
     *\param buffer The buffer to sign
     *\param bufferlen the buffer size
     *\param signature Where to write the pointer to the allocated memory containing the signature generated
     *\return the size of the signature given
     *\exception SVCAuthInterfaceException thrown if unable to sign
     */
    size_t sign(void* buffer, size_t bufferlen, void** signature)throw(SVCAuthInterfaceException);
    
    /**
     *\fn size_t getSignLen()
     *\brief Give the signature size
     *\return the size of signature
     */
    size_t getSignLen();
    
    /**
     *\fn bool verifySign(void* identity, size_t identitylen, void* signature, size_t signaturelen, void* buffer, size_t bufferlen)throw(SVCAuthInterfaceException);
     *\brief Check if the signture given from peer is valid with the buffer received
     *\param identity the peer identity
     *\param identitylen the identity size
     *\param signature the signature received
     *\param signaturelen signature size
     *\param buffer the buffer signed
     *\param bufferlen the buffer size
     *\param signature Where to write the pointer to the allocated memory containing the signature generated
     *\return whether if the signature is valid (true) or not (false)
     */
    bool verifySign(void* identity, size_t identitylen, void* signature, size_t signaturelen, void* buffer, size_t bufferlen) throw(SVCAuthInterfaceException);
    
};

#endif /* AuthPKI_hpp */
