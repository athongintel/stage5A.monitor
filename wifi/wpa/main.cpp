#include <stdio.h>
#include <string.h>

#include "wpa_ctrl.h"

const char* WLAN0 = "/var/run/wpa_supplicant/wlan0";

#define CMD_STATUS "STATUS"
#define CMD_DISCONNECT "DISCONNECT"

void test1();

int main(int argc, char** argv){

	//test
	test1();

	return 0;
	
}


void test1(){
	struct wpa_ctrl* wlan0ctrl;
	wlan0ctrl = wpa_ctrl_open(WLAN0);
	
	char reply[1000];
	size_t reply_len;
	int request_result;
	
	//request_result = wpa_ctrl_request(wlan0ctrl, CMD_DISCONNECT, strlen(CMD_DISCONNECT), reply, &reply_len, NULL);
	
	request_result = wpa_ctrl_request(wlan0ctrl, CMD_STATUS, strlen(CMD_STATUS), reply, &reply_len, NULL);
	
	/*if (request_result){
		printf("Error: %d\n", request_result);
	}
	else{*/
		printf("Receive %d bytes: %s\n", reply_len, reply);
//	}
	wpa_ctrl_close(wlan0ctrl);
}
