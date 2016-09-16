CC = g++
CFLAGS = -I /usr/include/rapidjson/ -I /usr/include/libnl3/ -std=c++11 -lnl-3 -lnl-genl-3 -lpcap -pthread -fpermissive -I./ -MMD -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX

build:
	$(CC) -o exe nlutil.c nlwifi.c wpa_ctrl.c os_unix.c pcapwifi.c network.cpp geo.cpp wpawrapper.cpp main.cpp $(CFLAGS)
	
clean:
	rm -f *.o *.d exe
