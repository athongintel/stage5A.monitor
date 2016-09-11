//
//  DiffieHellman.cpp
//  SVC
//
//  Created by Romain CYRILLE on 05/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "DiffieHellman.hpp"

DiffieHellman::DiffieHellman()throw(DiffieHellmanException){
    _pKey = EVP_PKEY_new();
    _peerPublicKey = EVP_PKEY_new();
    _param = EVP_PKEY_new();
    _commonSecret = NULL;
    _secretLen = NULL;
}


DiffieHellman::~DiffieHellman(){
    EVP_PKEY_free(_peerPublicKey);
    EVP_PKEY_free(_param);
    EVP_PKEY_free(_pKey);
    if(_commonSecret !=NULL)delete _commonSecret;
    if(_secretLen!=NULL)delete _secretLen;
}

void DiffieHellman::generateParameters() throw(DiffieHellmanException){
    EVP_PKEY_CTX *ctx;
    
    /*Create the context for the parameters generation*/
    if((ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, NULL)) == NULL){
        std::string err = "Error initializing parameters context";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    /* Initialise the parameter generation */
    if(EVP_PKEY_paramgen_init(ctx) != 1){
        std::string err = "Error initializing parameters generation";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    /* Setting the curve for parameters generation*/
    if(EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1) != 1){
        std::string err = "Error setting the curve parameters";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    /* Generating the parameter */
    if (!EVP_PKEY_paramgen(ctx,&_param)){
        std::string err = "Error generating parameters";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    EVP_PKEY_CTX_free(ctx);
}

void DiffieHellman::generatePkey() throw(DiffieHellmanException){
    EVP_PKEY_CTX * ctx;
    /*Create the context for the key generation*/
    if((ctx = EVP_PKEY_CTX_new(_param, NULL)) == NULL){
        std::string err = "Error initializing key generation context";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    /*Generating keys*/
    if(EVP_PKEY_keygen_init(ctx) != 1){
        std::string err = "Error generating key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    if(EVP_PKEY_keygen(ctx, &_pKey) != 1){
        std::string err = "Error generating key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    EVP_PKEY_CTX_free(ctx);
}

void DiffieHellman::generateCommonSecret() throw(DiffieHellmanException){
    EVP_PKEY_CTX * ctx;
    _secretLen = (size_t *) malloc(sizeof(size_t));
    
    /*Create the context for the secret generation*/
    if((ctx = EVP_PKEY_CTX_new(_pKey, NULL)) == NULL){
        std::string err = "Error creating secret generation context";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    /* Initialise the secret generation */
    if( EVP_PKEY_derive_init(ctx) != 1){
        std::string err = "Error initializing secret generation";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }

    
    /* Provide the peer public key */
    if(EVP_PKEY_derive_set_peer(ctx, _peerPublicKey)!= 1){
        std::string err = "Error setting the peer public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }

    
    /* Determine buffer length for shared secret */
    if(EVP_PKEY_derive(ctx, NULL, _secretLen)!= 1){
        std::string err = "Error determining the secret len";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    /* Create the buffer */
    if((_commonSecret = (unsigned char *)malloc(*_secretLen))==NULL){
        std::string err = "Error allocation the secret buffer";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    /* Derive the shared secret */
    if((EVP_PKEY_derive(ctx, _commonSecret, _secretLen))!= 1){
        std::string err = "Error generating secret";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    EVP_PKEY_CTX_free(ctx);
    
}

size_t DiffieHellman::getCommonSecret(unsigned char **secret){
    *secret = (unsigned char *) malloc(*_secretLen);
    memcpy(*secret, _commonSecret, *_secretLen);
    return *_secretLen;
}

size_t DiffieHellman::getPublicKey(unsigned char ** publicKey) throw(DiffieHellmanException){
    EC_KEY *key;
    const EC_POINT * point;
    *publicKey =  (unsigned char *)malloc(65);
    EC_GROUP * group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    size_t len;
    
    if((key=EVP_PKEY_get1_EC_KEY(_pKey)) == NULL){
        std::string err = "Error getting public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());

    }
    if((point = EC_KEY_get0_public_key(key))==NULL){
        std::string err = "Error getting public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    if((len = EC_POINT_point2oct(group, point, EC_GROUP_get_point_conversion_form(group), *publicKey, 65, NULL))<=0){
        std::string err = "Error getting public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    EC_GROUP_free(group);
    EC_POINT_free((EC_POINT*)point);
    return len;
}

void DiffieHellman::setPeerPublicKey(unsigned char *peerPublicKey,size_t len) throw(DiffieHellmanException){
    EC_KEY * key = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
    EC_GROUP * group = EC_GROUP_new_by_curve_name(NID_X9_62_prime256v1);
    EC_POINT * point = EC_POINT_new(group);
    
    if(EC_POINT_oct2point(group, point, peerPublicKey, len, NULL)<=0){
        std::string err = "Error setting peer public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    if(EC_KEY_set_public_key(key, point)<=0){
        std::string err = "Error setting peer public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    
    if(EVP_PKEY_set1_EC_KEY(_peerPublicKey, key)<=0){
        std::string err = "Error setting peer public key";
        err.append(ERR_error_string(ERR_get_error(), NULL));
        throw DiffieHellmanException(err,false,ERR_get_error());
    }
    EC_KEY_free(key);
    EC_GROUP_free(group);
    EC_POINT_free(point);
}



