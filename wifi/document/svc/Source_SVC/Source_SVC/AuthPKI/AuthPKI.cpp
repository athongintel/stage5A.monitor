//
//  AuthPKI.cpp
//  SVC
//
//  Created by Ludovic POUJOL on 17/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#include "AuthPKI.hpp"
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>


struct passwd *pw = getpwuid(getuid());
const std::string homedir = pw->pw_dir;

AuthPKI::AuthPKI(std::string identName){
    OPENSSL_add_all_algorithms_noconf();
    
    FILE* file;
    std::string fileName;

    // Load CA
    fileName = homedir + "/.PKI/ca.crt";
    file = fopen(fileName.c_str(), "rb");
    this->ca_cert = PEM_read_X509(file, NULL, 0, NULL);
    fclose(file);

    // Load cert
    fileName = homedir + "/.PKI/" + identName + ".crt";
    std::cout << "loading user cert " << fileName << " certificate"<< std::endl;
    file = fopen(fileName.c_str(), "rb");
    this->my_cert = PEM_read_X509(file, NULL, 0, NULL);
    fclose(file);

    // Load key
    fileName = homedir + "/.PKI/" + identName + ".key";
    file = fopen(fileName.c_str(), "rb");
    this->my_key = PEM_read_RSAPrivateKey(file, NULL, NULL, NULL);
    fclose(file);
}

AuthPKI::~AuthPKI(){
    X509_free(this->ca_cert);
    X509_free(this->my_cert);
    X509_free(this->peer_cert);
}

size_t AuthPKI::getIdentity(void** identity){
    BIO *bio = BIO_new(BIO_s_mem());
    
    int rc = PEM_write_bio_X509(bio, this->my_cert);
    unsigned long err = ERR_get_error();
    
    if (rc != 1){
        std::cerr << "PEM_write_bio_X509 failed, error " << err << ", ";
        std::cerr << std::hex << "0x" << err;
        exit(1);
    }
    
    BUF_MEM *mem = NULL;
    BIO_get_mem_ptr(bio, &mem);
    err = ERR_get_error();
    
    if (!mem || !mem->data || !mem->length){
        std::cerr << "BIO_get_mem_ptr failed, error " << err << ", ";
        std::cerr << std::hex << "0x" << err;
        exit(2);
    }
    
    std::string pem(mem->data, mem->length);
    //std::cout << pem << std::endl;

    *identity = malloc(pem.length()*sizeof(char));
    ::memcpy(*identity, pem.c_str(), pem.length()*sizeof(char));
    
    return pem.length()*sizeof(char);
}

bool AuthPKI::verifyIdentity(void* identity, size_t identitylen)throw(SVCAuthInterfaceException){
    BIO *bio = BIO_new(BIO_s_mem());
    X509 *cert_received;
    bool result = false;
    
    //fwrite(identity, 1, identitylen, stdout);
    
    BIO_write(bio, identity, (int) identitylen);
    
    cert_received = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    
    BIO_vfree(bio);
    
    if( cert_received == NULL ){
        throw new SVCAuthInterfaceException("Empty or invalid identity given", false);
    }
    
    result = checkCertificateWithCA(cert_received);
    
    // Save peer certificate for later user
    if(result){
        this->peer_cert = cert_received;
        this->peer_key  = EVP_PKEY_get1_RSA(X509_get_pubkey(cert_received));
    }
    
    return result;
}

size_t AuthPKI::sign(void* buffer, size_t bufferlen, void** signature)throw(SVCAuthInterfaceException){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char sign[256];
    unsigned int signLen;
    
    // Hash the buffer
    SHA256((unsigned char*) buffer, bufferlen, hash);
    
    // Sign the hash
    RSA_sign(NID_sha256, hash, SHA256_DIGEST_LENGTH, sign, &signLen, this->my_key);

    *signature = malloc(getSignLen());
    memcpy(*signature, sign, getSignLen());
    return getSignLen();
}

size_t AuthPKI::getSignLen(){
    // Size of signatures with RSA 2048bits keys
    return 256;
}

bool AuthPKI::verifySign(void* identity, size_t identitylen, void* signature, size_t signaturelen, void* buffer, size_t bufferlen)throw(SVCAuthInterfaceException){
    unsigned char hash[SHA256_DIGEST_LENGTH];
    int ret;
    
    // Hash the buffer
    SHA256((unsigned char*) (buffer), bufferlen, hash);

    // Check the signature
    ret = RSA_verify(NID_sha256, hash, SHA256_DIGEST_LENGTH, (unsigned char*)signature, (unsigned int) signaturelen, this->peer_key);
    
    return (ret == 1) ? true : false;
}

bool AuthPKI::checkCertificateWithCA(X509* certificate)
{
    int status;
    bool result = false;
    
    X509_STORE_CTX *store_ctx = X509_STORE_CTX_new();
    X509_STORE *store = X509_STORE_new();
    
    // Init the trusted store
    X509_STORE_add_cert(store, this->ca_cert);
    X509_STORE_CTX_init(store_ctx, store, certificate, NULL);
    
    // Check the certificate with the trusted store
    status = X509_verify_cert(store_ctx);
    if(status == 1) {
        result = true;
    }
    else {
        result = false;
        printf("Error : %s\n", X509_verify_cert_error_string(store_ctx->error));
    }
    
    return result;
}