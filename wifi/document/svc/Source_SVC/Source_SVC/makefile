CC 		= g++
CFLAG 	= -Wall -std=c++11 -O3
ODIR 	= build/obj
SERVERDIR = Server
CLIENTDIR = Client
SOCKETDIR = Socket
CRYPTOSYMDIR = CryptoSym
DIFFIEDIR = DiffieHellman
SVCAUTHINTERFACEDIR = SVCAuthInterface
AUTHPKIDIR = AuthPKI
SVCDIR = SVC

BINDIR = bin


LIB_DIR	= -L/usr/local/opt/openssl/lib
LIBH_DIR = -I/usr/local/opt/openssl/include
LIBH_DIR += -I./$(SVCDIR)
LIBH_DIR += -I./$(SERVERDIR)
LIBH_DIR += -I./$(CLIENTDIR)
LIBH_DIR += -I./$(DIFFIEDIR)
LIBH_DIR += -I./$(SOCKETDIR)
LIBH_DIR += -I./$(AUTHPKIDIR)
LIBH_DIR += -I./$(CRYPTOSYMDIR)
LIBH_DIR += -I./$(SVCAUTHINTERFACEDIR)

LIBS	+= -lcrypto -lpthread

all: server client | $(ODIR)

##############
## Dossiers ##
##############

$(ODIR):
	mkdir -p $(ODIR)

$(BINDIR):
	mkdir -p $(BINDIR)

$(ODIR)/$(SVCDIR):
	mkdir -p $(ODIR)/$(SVCDIR)

$(ODIR)/$(SERVERDIR):
	mkdir -p $(ODIR)/$(SERVERDIR)

$(ODIR)/$(CLIENTDIR):
	mkdir -p $(ODIR)/$(CLIENTDIR)

$(ODIR)/$(SOCKETDIR):
	mkdir -p $(ODIR)/$(SOCKETDIR)

$(ODIR)/$(DIFFIEDIR):
	mkdir -p $(ODIR)/$(DIFFIEDIR)

$(ODIR)/$(AUTHPKIDIR):
	mkdir -p $(ODIR)/$(AUTHPKIDIR)

$(ODIR)/$(CRYPTOSYMDIR):
	mkdir -p $(ODIR)/$(CRYPTOSYMDIR)

$(ODIR)/$(SVCAUTHINTERFACEDIR):
	mkdir -p $(ODIR)/$(SVCAUTHINTERFACEDIR)


#################
## SERVER MAIN ##
#################
$(ODIR)/$(SERVERDIR)/main.o: $(SERVERDIR)/main.cpp	| $(ODIR)/$(SERVERDIR)
	$(CC) $(CFLAG) -c $(SERVERDIR)/main.cpp -o $(ODIR)/$(SERVERDIR)/main.o $(LIBH_DIR)


#################
## CLIENT MAIN ##
#################

$(ODIR)/$(CLIENTDIR)/main.o: $(CLIENTDIR)/main.cpp	| $(ODIR)/$(CLIENTDIR)
	$(CC) $(CFLAG) -c $(CLIENTDIR)/main.cpp -o $(ODIR)/$(CLIENTDIR)/main.o $(LIBH_DIR)


############
## SOCKET ##
############

$(ODIR)/$(SOCKETDIR)/Socket.o: $(SOCKETDIR)/Socket.cpp	| $(ODIR)/$(SOCKETDIR)
	$(CC) $(CFLAG) -c $(SOCKETDIR)/Socket.cpp -o $(ODIR)/$(SOCKETDIR)/Socket.o $(LIBH_DIR)

$(ODIR)/$(SOCKETDIR)/SocketException.o: $(SOCKETDIR)/SocketException.cpp	| $(ODIR)/$(SOCKETDIR)
	$(CC) $(CFLAG) -c $(SOCKETDIR)/SocketException.cpp -o $(ODIR)/$(SOCKETDIR)/SocketException.o $(LIBH_DIR)

$(ODIR)/$(SOCKETDIR)/TCPSocket.o: $(SOCKETDIR)/TCPSocket.cpp	| $(ODIR)/$(SOCKETDIR)
	$(CC) $(CFLAG) -c $(SOCKETDIR)/TCPSocket.cpp -o $(ODIR)/$(SOCKETDIR)/TCPSocket.o $(LIBH_DIR)

$(ODIR)/$(SOCKETDIR)/TCPServerSocket.o: $(SOCKETDIR)/TCPServerSocket.cpp	| $(ODIR)/$(SOCKETDIR)
	$(CC) $(CFLAG) -c $(SOCKETDIR)/TCPServerSocket.cpp -o $(ODIR)/$(SOCKETDIR)/TCPServerSocket.o $(LIBH_DIR)


####################
## DIFFIE HELLMAN ##
####################

$(ODIR)/$(DIFFIEDIR)/DiffieHellman.o: $(DIFFIEDIR)/DiffieHellman.cpp	| $(ODIR)/$(DIFFIEDIR)
	$(CC) $(CFLAG) -c $(DIFFIEDIR)/DiffieHellman.cpp -o $(ODIR)/$(DIFFIEDIR)/DiffieHellman.o $(LIBH_DIR)

$(ODIR)/$(DIFFIEDIR)/DiffieHellmanException.o: $(DIFFIEDIR)/DiffieHellmanException.cpp	| $(ODIR)/$(DIFFIEDIR)
	$(CC) $(CFLAG) -c $(DIFFIEDIR)/DiffieHellmanException.cpp -o $(ODIR)/$(DIFFIEDIR)/DiffieHellmanException.o $(LIBH_DIR)


#########
## SVC ##
#########

$(ODIR)/$(SVCDIR)/SVCClient.o: $(SVCDIR)/SVCClient.cpp	| $(ODIR)/$(SVCDIR)
	$(CC) $(CFLAG) -c $(SVCDIR)/SVCClient.cpp -o $(ODIR)/$(SVCDIR)/SVCClient.o $(LIBH_DIR)

$(ODIR)/$(SVCDIR)/SVCCommunication.o: $(SVCDIR)/SVCCommunication.cpp	| $(ODIR)/$(SVCDIR)
	$(CC) $(CFLAG) -c $(SVCDIR)/SVCCommunication.cpp -o $(ODIR)/$(SVCDIR)/SVCCommunication.o $(LIBH_DIR)

$(ODIR)/$(SVCDIR)/SVCConnectedClient.o: $(SVCDIR)/SVCConnectedClient.cpp	| $(ODIR)/$(SVCDIR)
	$(CC) $(CFLAG) -c $(SVCDIR)/SVCConnectedClient.cpp -o $(ODIR)/$(SVCDIR)/SVCConnectedClient.o $(LIBH_DIR)

$(ODIR)/$(SVCDIR)/SVCException.o: $(SVCDIR)/SVCException.cpp	| $(ODIR)/$(SVCDIR)
	$(CC) $(CFLAG) -c $(SVCDIR)/SVCException.cpp -o $(ODIR)/$(SVCDIR)/SVCException.o $(LIBH_DIR)

$(ODIR)/$(SVCDIR)/SVCServer.o: $(SVCDIR)/SVCServer.cpp	| $(ODIR)/$(SVCDIR)
	$(CC) $(CFLAG) -c $(SVCDIR)/SVCServer.cpp -o $(ODIR)/$(SVCDIR)/SVCServer.o $(LIBH_DIR)


########################
## SVC AUTH INTERFACE ##
########################

$(ODIR)/$(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.o: $(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.cpp	| $(ODIR)/$(SVCAUTHINTERFACEDIR)
	$(CC) $(CFLAG) -c $(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.cpp -o $(ODIR)/$(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.o $(LIBH_DIR)


################
## CRYPTO SYM ##
################

$(ODIR)/$(CRYPTOSYMDIR)/CryptoSym.o: $(CRYPTOSYMDIR)/CryptoSym.cpp	| $(ODIR)/$(CRYPTOSYMDIR)
	$(CC) $(CFLAG) -c $(CRYPTOSYMDIR)/CryptoSym.cpp -o $(ODIR)/$(CRYPTOSYMDIR)/CryptoSym.o $(LIBH_DIR)

$(ODIR)/$(CRYPTOSYMDIR)/CryptoSymException.o: $(CRYPTOSYMDIR)/CryptoSymException.cpp	| $(ODIR)/$(CRYPTOSYMDIR)
	$(CC) $(CFLAG) -c $(CRYPTOSYMDIR)/CryptoSymException.cpp -o $(ODIR)/$(CRYPTOSYMDIR)/CryptoSymException.o $(LIBH_DIR)


##############
## AUTH PKI ##
##############

$(ODIR)/$(AUTHPKIDIR)/AuthPKI.o: $(AUTHPKIDIR)/AuthPKI.cpp	| $(ODIR)/$(AUTHPKIDIR)
	$(CC) $(CFLAG) -c $(AUTHPKIDIR)/AuthPKI.cpp -o $(ODIR)/$(AUTHPKIDIR)/AuthPKI.o $(LIBH_DIR)


server: $(ODIR)/$(SERVERDIR)/main.o \
	$(ODIR)/$(AUTHPKIDIR)/AuthPKI.o \
	$(ODIR)/$(SOCKETDIR)/Socket.o \
	$(ODIR)/$(SOCKETDIR)/SocketException.o \
	$(ODIR)/$(SOCKETDIR)/TCPSocket.o \
	$(ODIR)/$(SOCKETDIR)/TCPServerSocket.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellman.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellmanException.o \
	$(ODIR)/$(SVCDIR)/SVCClient.o \
	$(ODIR)/$(SVCDIR)/SVCServer.o \
	$(ODIR)/$(SVCDIR)/SVCException.o \
	$(ODIR)/$(SVCDIR)/SVCConnectedClient.o \
	$(ODIR)/$(SVCDIR)/SVCCommunication.o \
	$(ODIR)/$(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSym.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSymException.o 		| $(ODIR) $(BINDIR)
	$(CC) $(CFLAG) -o $(BINDIR)/server $(ODIR)/$(SERVERDIR)/main.o \
	$(ODIR)/$(AUTHPKIDIR)/AuthPKI.o \
	$(ODIR)/$(SOCKETDIR)/Socket.o \
	$(ODIR)/$(SOCKETDIR)/SocketException.o \
	$(ODIR)/$(SOCKETDIR)/TCPSocket.o \
	$(ODIR)/$(SOCKETDIR)/TCPServerSocket.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellman.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellmanException.o \
	$(ODIR)/$(SVCDIR)/SVCClient.o \
	$(ODIR)/$(SVCDIR)/SVCServer.o \
	$(ODIR)/$(SVCDIR)/SVCException.o \
	$(ODIR)/$(SVCDIR)/SVCConnectedClient.o \
	$(ODIR)/$(SVCDIR)/SVCCommunication.o \
	$(ODIR)/$(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSym.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSymException.o 	 $(LIB_DIR) $(LIBS)

client: $(ODIR)/$(CLIENTDIR)/main.o \
	$(ODIR)/$(AUTHPKIDIR)/AuthPKI.o \
	$(ODIR)/$(SOCKETDIR)/Socket.o \
	$(ODIR)/$(SOCKETDIR)/SocketException.o \
	$(ODIR)/$(SOCKETDIR)/TCPSocket.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellman.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellmanException.o \
	$(ODIR)/$(SVCDIR)/SVCClient.o \
	$(ODIR)/$(SVCDIR)/SVCException.o \
	$(ODIR)/$(SVCDIR)/SVCCommunication.o \
	$(ODIR)/$(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSym.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSymException.o 		| $(ODIR) $(BINDIR)
	$(CC) $(CFLAG) -o $(BINDIR)/client $(ODIR)/$(CLIENTDIR)/main.o \
	$(ODIR)/$(AUTHPKIDIR)/AuthPKI.o \
	$(ODIR)/$(SOCKETDIR)/Socket.o \
	$(ODIR)/$(SOCKETDIR)/SocketException.o \
	$(ODIR)/$(SOCKETDIR)/TCPSocket.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellman.o \
	$(ODIR)/$(DIFFIEDIR)/DiffieHellmanException.o \
	$(ODIR)/$(SVCDIR)/SVCClient.o \
	$(ODIR)/$(SVCDIR)/SVCException.o \
	$(ODIR)/$(SVCDIR)/SVCCommunication.o \
	$(ODIR)/$(SVCAUTHINTERFACEDIR)/SVCAuthInterfaceException.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSym.o \
	$(ODIR)/$(CRYPTOSYMDIR)/CryptoSymException.o 	 $(LIB_DIR) $(LIBS)

clean:
	rm -r $(ODIR)/ $(BINDIR)/
