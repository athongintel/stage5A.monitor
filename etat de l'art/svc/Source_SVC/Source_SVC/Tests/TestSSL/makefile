CC 		= g++
CFLAG 	= -Wall -std=c++11 -pthread
ODIR 	= build
SERVERDIR = Server
CLIENTDIR = Client
DIFFIEDIR = DiffieHellman
SOCKETDIR = Socket
BINDIR = bin


LIB_DIR  = -L/usr/local/opt/openssl/lib
LIBH_DIR = -I/usr/local/opt/openssl/include
LIBH_DIR += -I./$(SERVERDIR)
LIBH_DIR += -I./$(CLIENTDIR)
LIBH_DIR += -I./$(DIFFIEDIR)
LIBH_DIR += -I./$(SOCKETDIR)


LIBS	+= -lcrypto -lssl

all: server client | $(ODIR)

$(ODIR):
	mkdir -p $(ODIR)

$(BINDIR):
	mkdir -p $(BINDIR)


$(ODIR)/server.o: ./Server.cpp  | $(ODIR)
	$(CC) $(CFLAG) -c ./Server.cpp -o $(ODIR)/server.o $(LIBH_DIR)

$(ODIR)/client.o: ./Client.cpp  | $(ODIR)
	$(CC) $(CFLAG) -c ./Client.cpp -o $(ODIR)/client.o $(LIBH_DIR)



server: $(ODIR)/server.o  | $(ODIR) $(BINDIR)
	$(CC) $(CFLAG) -o $(BINDIR)/server $(ODIR)/server.o $(LIB_DIR) $(LIBS)

client: $(ODIR)/client.o  | $(ODIR) $(BINDIR)
	$(CC) $(CFLAG) -o $(BINDIR)/client $(ODIR)/client.o $(LIB_DIR) $(LIBS)

clean:
	rm -r $(ODIR)/ $(BINDIR)/
