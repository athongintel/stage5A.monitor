#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>
#include <iomanip>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <mutex>
#include <signal.h>

std::mutex fileMtx;

#define PKIDIR "./PKI/"
// autorité de certification
#define CAF   PKIDIR "ca.crt"
// certificats valides
#define CERTF PKIDIR "server.crt"
#define KEYF  PKIDIR "server.key"
 // certificats invalides
#define FAKECERTF PKIDIR "serverFake.crt"
#define FAKEKEYF  PKIDIR "serverFake.key"

#define DEPTH_CHECK 5

#define BUFFER_SIZE 4096

#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s); exit(1); }
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr); exit(2); }


static int verifyCert(int preverify_ok, X509_STORE_CTX *ctx);
void handleclient(SSL_CTX* ctx, int sd, std::string * path);


int main(int argc, const char * argv[]){
	int err;
	int listen_sd;
	int sd;
	struct sockaddr_in sa_serv;
	struct sockaddr_in sa_cli;
	size_t client_len;
	SSL_CTX* ctx;
	X509*    client_cert;
	char*    str;
	char     buf [4096];
	const SSL_METHOD *meth;

    if(argc<3){
        std::cout<<"Usage Server [listening_port] [path_to_file]"<<std::endl;
        return 0;
    }
    std::string * path = new std::string(argv[2]);
    std::cout<<"Sending file " << path->c_str()<<std::endl;

    /* SSL preliminaries. We keep the certificate and key with the context. */
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
	meth = TLSv1_2_server_method();
	ctx = SSL_CTX_new (meth);
	if (!ctx) {
		ERR_print_errors_fp(stderr);
		exit(2);
	}

    std::cout<<"Chargement des certificats" <<std::endl;
	if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(3);
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(4);
	}

	if (!SSL_CTX_check_private_key(ctx)) {
		fprintf(stderr,"Private key does not match the certificate public key\n");
		exit(5);
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, verifyCert);
	SSL_CTX_set_verify_depth(ctx,DEPTH_CHECK + 1);

	/* -------------------------------------------- */
	/* Prepare TCP socket for receiving connections */
	
	listen_sd = socket (AF_INET, SOCK_STREAM, 0);   CHK_ERR(listen_sd, "socket");
	
	memset(&sa_serv, 0, sizeof(sa_serv));
	sa_serv.sin_family      = AF_INET;
	sa_serv.sin_addr.s_addr = INADDR_ANY;
	sa_serv.sin_port        = htons (atoi(argv[1]));          /* Server Port number */
	
	err = bind(listen_sd, (struct sockaddr*) &sa_serv,
			   sizeof (sa_serv));                   CHK_ERR(err, "bind");
	
	/* Receive a TCP connection. */
	err = listen (listen_sd, 5);                    CHK_ERR(err, "listen");
	client_len = sizeof(sa_cli);

	printf("Server listening...\n");
	
	while(true){
    	sd = accept(listen_sd, (struct sockaddr*) &sa_cli, (socklen_t*) &client_len);
		CHK_ERR(sd, "accept");

		new std::thread(handleclient,ctx,sd,path);
    }
    close (listen_sd);
	SSL_CTX_free (ctx);
}

void handleclient(SSL_CTX* ctx, int sd, std::string * path){
	int err;
	char buf [4096];
	FILE * fp;
	SSL* ssl;
	ssize_t bytes_read,bytes_send,offset;
    unsigned char buffer[BUFFER_SIZE];

	ssl = SSL_new (ctx);                           CHK_NULL(ssl);
	SSL_set_fd (ssl, sd);
	err = SSL_accept (ssl);                        CHK_SSL(err);

    std::cout<<"Client connected\n";

    fp = fopen(path->c_str(),"rb");
    if(fp == NULL)throw std::runtime_error("Error opening file");
    fseek(fp, 0, SEEK_END);
    size_t fileLen = ftell(fp);
    rewind(fp);
    
    SSL_write (ssl, &fileLen, sizeof(size_t));

    SSL_read (ssl, &offset, sizeof(size_t));

    fseek(fp, offset, SEEK_SET);
    std::cout<<"Offset : "<<offset<<std::endl;
    bytes_send = 0;
    err = 1;
    while (bytes_send+offset < fileLen && err>0){
        fileMtx.lock();
        bytes_read = fread(buffer,sizeof(char),BUFFER_SIZE,fp);
        fileMtx.unlock();
        if(bytes_read == 0)break;
        if(bytes_read<0)throw std::runtime_error("Error reading file");
        err = SSL_write (ssl, buffer, bytes_read);
        bytes_send += bytes_read;
    }
    std::cout<<"File sent"<<std::endl;

	SSL_free (ssl);
    fclose(fp);
    close(sd);
}

static int verifyCert(int preverify_ok, X509_STORE_CTX *ctx)
{
	char    buf[256];
	X509   *err_cert;
	int     err, depth;
	
	err_cert = X509_STORE_CTX_get_current_cert(ctx);
	err = X509_STORE_CTX_get_error(ctx);
	depth = X509_STORE_CTX_get_error_depth(ctx);
	X509_NAME_oneline(X509_get_subject_name(err_cert), buf, 256);

	if (depth > DEPTH_CHECK) {
		preverify_ok = 0;
		err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
		X509_STORE_CTX_set_error(ctx, err);
	}
	
	if (!preverify_ok) {
		X509_STORE_CTX *store_ctx = X509_STORE_CTX_new();
		X509_STORE *store = X509_STORE_new();
		
		// Initialisation du X509_STORE
		FILE* f_cert = fopen(CAF, "rb");
		X509 *ca = PEM_read_X509(f_cert, NULL, 0, NULL);
		X509_STORE_add_cert(store, ca);
		X509_STORE_CTX_init(store_ctx, store, err_cert, NULL);
		
		// Vérification
		preverify_ok = X509_verify_cert(store_ctx);
		if(preverify_ok == 1) {
			printf("Certificat valide\n");
		}
		else {
			printf("Erreur : %s\n", X509_verify_cert_error_string(store_ctx->error));
		}
		
		if (!preverify_ok) {
			printf("verify error:num=%d:%s:depth=%d:%s\n", err,
			   X509_verify_cert_error_string(err), depth, buf);
		}
	}
	
	// Erreur
	if (!preverify_ok && (err == X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT))
	{
		X509_NAME_oneline(X509_get_issuer_name(ctx->current_cert), buf, 256);
		printf("issuer= %s\n", buf);
	}
	
	return preverify_ok;
}