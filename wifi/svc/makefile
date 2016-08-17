CC = g++
CFLAGS = -pthread -I./ -std=c++1y -lrt

client:
	$(CC) -o client.exe SendFileApp.cpp ./svc/SVCHostIP.cpp ./svc/SVC.cpp ./svc/SVC-header.cpp $(CFLAGS)

	
server:
	$(CC) -o server.exe SendFileAppServer.cpp $(CFLAGS)

