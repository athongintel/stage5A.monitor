#include <stdio.h>
#include <memory.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <iomanip>

#define PKIDIR "./PKI/"
// Autoritée de certification
#define CAF   PKIDIR "ca.crt"
// certificats valides
#define CERTF PKIDIR "client.crt"
#define KEYF  PKIDIR "client.key"
// certificats invalides
#define FAKECERTF PKIDIR "clientFake.crt"
#define FAKEKEYF  PKIDIR "clientFake.key"

#define DEPTH_CHECK 5


#define BUFFER_SIZE 4096

#define CHK_NULL(x) if ((x)==NULL) exit (1)
#define CHK_ERR(err,s) if ((err)==-1) { perror(s);}
#define CHK_SSL(err) if ((err)==-1) { ERR_print_errors_fp(stderr);}


static int verifyCert(int preverify_ok, X509_STORE_CTX *ctx);

int main(int argc, const char * argv[]){
	int err;
	int sd;
	struct sockaddr_in sa;
	SSL_CTX* ctx;
	SSL*     ssl;
	char     buf [4096];
	const SSL_METHOD *meth;

	bool connected;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
	
	// Init SSL
	OpenSSL_add_ssl_algorithms();
	meth = TLSv1_2_client_method();
	SSL_load_error_strings();
	ctx = SSL_CTX_new (meth);
	CHK_NULL(ctx);

    if(argc<4){
        std::cout<<"Usage : client [server_adress] [server_port] [path_save_file]";
        return 0;
    }
    const char * server_address = argv[1];
    const char * server_port = argv[2];
    const char * path = argv[3];

    FILE *fp = fopen(path,"wb");
    if(fp == NULL)throw std::runtime_error("Error opening file");

    // Chargement des certificats
	if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(3);
	}
	if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		exit(4);
	}
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER|SSL_VERIFY_CLIENT_ONCE, verifyCert);
	SSL_CTX_set_verify_depth(ctx,DEPTH_CHECK + 1);

	// Socket init
	sd = socket (AF_INET, SOCK_STREAM, 0);
	memset(&sa, 0, sizeof(sa));
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = inet_addr (server_address);
	sa.sin_port        = htons     (atoi(server_port));

	ssl = SSL_new (ctx);                         CHK_NULL(ssl);

	/* Debut du timer */
	std::cout << "Début du timer" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

	connect(sd, (struct sockaddr*) &sa, sizeof(sa));
	SSL_set_fd (ssl, sd);
	err = SSL_connect (ssl);                     CHK_SSL(err);

	/* Fin du timer */
	auto finish = std::chrono::high_resolution_clock::now();
	auto msChrono = std::chrono::duration<float, std::chrono::milliseconds::period>(finish-start);
    std::cout << "Temps de négociation : " << msChrono.count() << "ms" << std::endl;
    
    std::cout<<"Connecté au serveur"<<std::endl;    
    connected = true;
    
    size_t filelen;
    SSL_read (ssl, &filelen, sizeof(size_t));
    std::cout<<"Receiving file of "<<filelen <<" octect"<<std::endl;
    bytes_read = 0;
    SSL_write (ssl, &bytes_read, sizeof(size_t));

    while(bytes_read<filelen){
        while(!connected){
			std::cout<<"Impossible to connect, trying to reconnect in 1 second.."<<std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			
			connect(sd, (struct sockaddr*) &sa, sizeof(sa));
			SSL_set_fd (ssl, sd);
			err = SSL_connect (ssl);

			err = SSL_read (ssl, &filelen, sizeof(size_t));
			if(err == 0) connected = false;
			else connected = true;
   			err = SSL_write (ssl, &bytes_read, sizeof(size_t));
        }
        bzero(buffer, BUFFER_SIZE);
        if(bytes_read + BUFFER_SIZE >filelen){
			err = SSL_read (ssl, buffer, filelen-bytes_read);
			if(err == 0) connected = false;
			fwrite(buffer, sizeof(char), filelen-bytes_read, fp);
			bytes_read += filelen-bytes_read;
        }
        else{
			err = SSL_read (ssl, buffer, BUFFER_SIZE);
			if(err == 0) connected = false;
			fwrite(buffer, sizeof(char), BUFFER_SIZE, fp);
			bytes_read += BUFFER_SIZE;
        }
    }

    SSL_shutdown (ssl);
	//close (sd);
	SSL_free (ssl);
	SSL_CTX_free (ctx);
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