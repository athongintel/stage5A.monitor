CC = gcc
CFLAGS = -I./ -MMD -DCONFIG_CTRL_IFACE -DCONFIG_CTRL_IFACE_UNIX

build: wpa_ctrl.o os_unix.o main.o

	$(CC) -Wall -Wextra -o exe wpa_ctrl.o os_unix.o main.o $(CFLAGS)
	
clean:
	rm -f *.o *.d
