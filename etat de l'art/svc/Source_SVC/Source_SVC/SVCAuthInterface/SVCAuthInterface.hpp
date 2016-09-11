//
//  SVCAuthInterface.hpp
//  SVC
//
//  Created by Vivien GALUCHOT on 13/05/2016.
//  Copyright © 2016 Polytech Marseille. All rights reserved.
//

#ifndef AuthInterface_hpp
#define AuthInterface_hpp

#include <stdio.h>
#include <exception>
#include <string>
#include <errno.h>
#include "SVCAuthInterfaceException.hpp"

class SVCAuthInterface{
public:


    virtual ~SVCAuthInterface(){};
	/**
	 *\fn virtual size_t getIdentity(void* identity)
	 *\brief Récupère l'identité à transmettre à l'autre partie de la commnication pour l'authentification.
	 *\param identity pointeur vers un buffer qui sera alloué et ou sera stocké l’identité
	 *\return taille du buffer en nombre d’octet
	 */
	virtual size_t getIdentity(void** identity) = 0;

	/**
	 *\fn virtual bool verifyIdentity(void* identity, size_t identitylen)
	 *\brief Contrôle l’identité donnée pour vérifier si elle est valide et connue
	 *\param pointeur vers un buffer contenant l’identité à vérifier
	 *\param identitylen taille du buffer contenant l’identité en nombre d’octet
	 *\return boolean qui intique si l'identité est valide (true) ou non (false)
	 */
	virtual bool verifyIdentity(void* identity, size_t identitylen)throw(SVCAuthInterfaceException) = 0;

	/**
	 *\fn virtual size_t sign(void* buffer, size_t bufferlen, void* signature)
	 *\brief Signe le message donné avec l’identité courante
	 *\param buffer pointeur vers un buffer contenant les données à signer
	 *\param bufferlen taille du buffer en nombre d’octet
	 *\param signature pointer vers un buffer qui sera alloué afin de stocker la signature
	 *\return taille de la signature en nombre d'octets dans le buffer
	 */
	virtual size_t sign(void* buffer, size_t bufferlen, void** signature)throw(SVCAuthInterfaceException) = 0;

   	/**
     *\fn  virtual size_t getSignLen()
     *\brief Donne la taille des sigantures générées
     *\return taille de la signature en nombre d'octets
     */
    virtual size_t getSignLen() = 0;
    
	/**
	 *\fn virtual bool verifySign(void* identity, size_t identitylen, void* signature, size_t signaturelen, void* buffer, long bufferlen)
	 *\brief vérifie si la signature donnée est valide pour l’identité fournie elle aussi en paramètre
	 *\param identity pointeur vers un buffer contenant l’identité ayant effectué la signature
	 *\param identitylen taille du buffer contenant l’identité en nombre d’octet
	 *\param signature pointeur vers un buffer contenant la signature à contrôler
	 *\param signaturelen taille du buffer contenant la signature en nombre d’octet
	 *\param buffer pointer vers un buffer contenant les donnés à contrôler
	 *\param bufferlen taille du buffer contenant les données
	 *\return boolean qui indique si la signature est valide (true) ou non (false)
	 */
	virtual bool verifySign(void* identity, size_t identitylen, void* signature, size_t signaturelen, void* buffer, size_t bufferlen)throw(SVCAuthInterfaceException)  = 0;
    
    

};

#endif /* SVCAuthInterface_hpp */