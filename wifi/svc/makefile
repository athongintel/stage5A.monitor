CC = g++
CFLAGS = -pthread -I./ -std=c++11 -lrt

client:
	$(CC) -o client.exe SendFileApp.cpp ./svc/host/SVCHostIP.cpp ./svc/SVC.cpp ./svc/SVC-header.cpp $(CFLAGS)

	
server:
	$(CC) -o server.exe SendFileAppServer.cpp ./svc/host/SVCHostIP.cpp ./svc/SVC.cpp ./svc/SVC-header.cpp $(CFLAGS)

