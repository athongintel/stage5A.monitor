CC = g++
CFLAGS = -I /usr/include/rapidjson/ -I /usr/include/libnl3/ -std=c++11 -lnl-3 -lnl-genl-3 -fpermissive

build:
	$(CC) -o exe nlutil.c nlwifi.c network.cpp geo.cpp main.cpp $(CFLAGS)
	
clean:
	rm -f *.o *.d
