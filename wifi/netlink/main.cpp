#include "geo.h"
#include "wpawrapper.h"


#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <stdio.h>
#include <pcap.h>
#include <signal.h>
#include <unistd.h>
#include <netinet/in.h> 
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>

using namespace std;
using namespace rapidjson;

/*void test_connect(){
	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();
	vector<WifiNetwork*> networks;
	//int networkcount=0;
	int networkcount;
	int apcount;
	for (auto &i : interfaces){
		cout<<i->getName()<<" "<<i->getDisplayableMacAddress()<<endl;
		cout<<"I'm performing a full scan..."<<endl;
		networks = i->fullNetworkScan();	
		for (auto &net : networks){
			cout<<networkcount<<": "<<(net->SSID.empty()? "Hidden network" : net->SSID)<<endl;
			networkcount++;
			apcount=0;
			/*if (strcmp(net->SSID, "(Aix*Marseille universite")==0){
				cout<<"Trying to connect to "<<net->SSID<<" access point..."<<endl;				
			}
			bool connect = false;
			for (auto &ap : net->accessPoints){
				cout<<" -- "<<apcount<<":  "<<ap->getDisplayableBSSID()<<"  "<<ap->getFrequency()<<"  "<<endl;
				apcount++;
				/*if (!connect){
					//connect to first access point
					connect = true;
					i->connect(ap);
				}
			}
		}
		i->disconnect();
		cout<<"Disconnected!"<<endl;
	}
	int selectedInterface;
	int selectedNetwork;
	int selectedAP;
	cout<<"Selected interface to connect: ";
	cin>>selectedInterface;
	cout<<"Which network do you want connect: ";
	cin>>selectedNetwork;
	cout<<"Which AP of "<<networks[selectedNetwork]->SSID<<" you want to connect: ";
	cin>>selectedAP;
	
	
	//connect to this ap
	cout<<"Trying to connect to "<<networks[selectedNetwork]->accessPoints[selectedAP]->getDisplayableBSSID()<<" of "<<networks[selectedNetwork]->SSID<<" on "<<interfaces[selectedInterface]->getName()<<endl;
	
	interfaces[selectedInterface]->connect(networks[selectedNetwork]->accessPoints[selectedAP]);	

}*/

const char* SAVED_NETWORK_JSON_PATH = "./saved-network.json";
const int MAX_RANGE = 50;
pcap_t* pcap = NULL;
ofstream reportFile;
mutex syncMutex;;

typedef void (*pcap_packet_handler)(u_char* args, const struct pcap_pkthdr* header, const u_char* packet);

int wpa_service(const WifiInterface* interface){
	
	//start wpa_supplicant, not as daemon
	string command = "wpa_supplicant";
	string params[4];
	params[0] = "";
	params[1] = "-i" + interface->getName();
	params[2] = "-C/var/run/wpa_supplicant";
	
	cout<<"Runing wpa daemon on "<<interface->getName()<<endl;
	int result;
	result = execl("/sbin/wpa_supplicant", "wpa_suppplicant", params[0].c_str(), params[1].c_str(), params[2].c_str(), (void*)0);		
	
	//only return if failed
	return result;
}

string print_hex_ascii_line(const u_char* payload, int len, int offset){

	int i;
	int gap;
	const u_char* ch;
	
	char output[1024]="";
	
	/* hex */
	ch = payload;
	for(i = 0; i < len; i++) {
		printf("%02x ", *ch);
		ch++;
		/* print extra space after 8th byte for visual aid */
		if (i == 7)
			printf(" ");
	}
	/* print space to handle line less than 8 bytes */
	if (len < 8)
		printf(" ");
	
	/* fill hex gap with spaces if not full line */
	if (len < 16) {
		gap = 16 - len;
		for (i = 0; i < gap; i++) {
			printf("   ");
		}
	}
	printf("   ");
	
	/* ascii (if printable) */
	ch = payload;
	for(i = 0; i < len; i++) {
		if (isprint(*ch))
			printf("%c", *ch);
		else
			printf(".");
		ch++;
	}

	printf("\n");

	return string(output);
}

void wifi_connection_analyser(u_char* args, const struct pcap_pkthdr* header, const u_char* packet){

	//reportFile<<"Reporting data\n";
	printf("got a packet with len: %d, caplen: %d\n", header->len, header->caplen);
	string output = print_hex_ascii_line(packet, header->caplen, 0);
	

	//reportFile<<output;
	//pcap_breakloop(pcap);
}

int packet_sniffing(WifiInterface* interface, WifiNetwork* network, pcap_packet_handler handler){
	
	
	
	char errbuf[PCAP_ERRBUF_SIZE];
	int ret;
	
	const char* devName = (interface->getName()+string("mon")).c_str();
	//filter strategy
	//--1: coming in and out interface (MAC address) wlan addr1 ehost (receiver) and wlan addr2 ehost (sender)
	string mac = interface->getDevice()->getDisplayableMacAddress();
	string filter = string("wlan addr1 ") + mac + string(" or wlan addr2 ") + mac;
  
	struct bpf_program fp;	
 	bpf_u_int32 mask= PCAP_NETMASK_UNKNOWN ;
 	
	cout<<"Sniffing on interface: "<<devName<<", filter: "<<filter<<endl;

  	pcap = pcap_open_live(devName, BUFSIZ, 1, 1000, errbuf);
  	if (pcap == NULL) {
    	cout<<"Cannot initialize "<<devName<<" because: "<<errbuf<<endl;
    	goto exit_fail;
	}
	
	int* dlt_buf;
	int n;
	n = pcap_list_datalinks(pcap, &dlt_buf);
	printf("n = %d\n",n);
	if(n == -1)
	{
		pcap_perror(pcap, "Datalink_list");
	}
	else
	{
		printf("The list of datalinks supported are\n");
		int i;
		for(i=0; i<n; i++)
		    printf("%d\n",dlt_buf[i]);
		const char *str1 = pcap_datalink_val_to_name(dlt_buf[0]);
		const char *str2 = pcap_datalink_val_to_description(dlt_buf[0]);
		printf("str1 = %s\n",str1);
		printf("str2 = %s\n",str2);
		pcap_free_datalinks(dlt_buf);
	}
    
	if (pcap_compile(pcap, &fp, filter.c_str(), 1, mask) == -1) {
		cout<<"Couldn't parse filter"<<endl;
		goto exit_fail;
	}
	
	if (pcap_setfilter(pcap, &fp) == -1) {
		cout<<"Couldn't install filter"<<endl;
		goto exit_fail;
	}

	cout<<"Starting pcap loop"<<endl;
	
	//reportFile.open(string("./") + interface->getName() + string(".report"));
	syncMutex.unlock();
	ret = pcap_loop(pcap, 0, handler, NULL);
	//reportFile.close();
	cout<<"pcap_loop return: "<<ret;
	
	return ret;
	
	exit_fail:
		syncMutex.unlock();
		return -1;
}


int report(){

	WifiController* netController = new WifiController();
	vector<WifiInterface*> interfaces = netController->getNetworkInterfaces();

	//check for wpa_supplicant
	int ret;
	/*ret = system("wpa_supplicant");
	if (ret != 0){
		cout<<"Error: wpa_supplicant is not installed. Try: sudo apt-get install wpasupplicant"<<endl;
		return -1;
	}
	ret = system("ifconfig");
	if (ret != 0){
		cout<<"Error: ifconfig not found on this system"<<endl;
		return -1;
	}*/	

	cout<<"Killing disturbing processes..."<<endl;
	//manually kill network-manager
	system("service network-manager stop");	
	system("service wpa_supplicant stop");		
	
	//run one wpa_supplicant for each interface
	int pCount = 0;
	pid_t pid = getpid();
	int cpid[interfaces.size()]; //child processes's id
	
	//fill zero to children process ids
	memset(cpid, 0, sizeof(cpid));
	
	vector<WifiInterface*> virtualInterfaces;
	
	for (WifiInterface* i : interfaces){
		cout<<"Interface: "<<i->getName()<<", MAC: "<<i->getDevice()->getDisplayableMacAddress()<<endl;
		if (pid != 0){									
			//only call fork in parent process
			pid = fork();
			if (pid == 0){
				//starting wpa process for wifi handling
				return wpa_service(i);
			}
			else{
				//this is parent process. save the pid for later closing.
				cout<<"Starting pid: "<<pid<<" as wpa process..."<<endl;
				cpid[pCount]=pid;
				pCount++;
				
				//create new virtual interface and enable monitor mode
				cout<<"Creating new virtual interface: "<<i->getName() + string("mon")<<endl;
				WifiInterface* vi = i->getDevice()->addVirtualInterface(i, i->getName() + string("mon"), NL80211_IFTYPE_MONITOR);
				
				virtualInterfaces.push_back(vi);
				//bring up this virtual interface
				string bringupcommand;
				bringupcommand = string("ifconfig ") + i->getName() + string(" up");
				cout<<"Bringing up: "<<bringupcommand<<endl;
				system(bringupcommand.c_str());				
				
				bringupcommand = string("ifconfig ") + i->getName() + string("mon") + string(" up");
				cout<<"Bringing up: "<<bringupcommand<<endl;
				system(bringupcommand.c_str());
				
				//wait for wireshark
				string ok;
				cout<<"start wireshark...";
				cin>>ok;
						
			}
		}
		else{
			//this is the next loop inside child proccess with pid overriden to 0 after a success fork
			//just break to stop the redundancy iteration.
			break;		
		}
	}
	
	//parent does this
	if (pid!=0){
	
		//1. load saved network
		ifstream file(SAVED_NETWORK_JSON_PATH);
		string contents = string(istreambuf_iterator<char>(file), istreambuf_iterator<char>());
		Document doc;
		doc.Parse(contents.c_str());	
	
		//2. get current location
		GeoTracker* geoTracker = GeoTracker::getInstance();
		struct GeoLocation* currentLocation = geoTracker->getCurrentLocation();
		//std::cout<<"Current location: "<<currentLocation->latitude<<", "<<currentLocation->longitude<<std::endl;		

	
		//4. iterate through access points and check for distance
		Value &networks = doc["networks"];
		if (networks.IsNull() || !networks.IsArray()){
			cout<<"Error: Bad saved network file!"<<endl;
			return -1;		
		}
		else{
			//test with each network interface							
			for (SizeType i=0; i<networks.Size(); i++){
				Value& net = networks[i];			
				if (!net.IsNull()){
					Value& aps = net["aps"];
					string cipherMode = net["encryption"]["method"].GetString();
					string networkName = net["ssid"].GetString();
					cout<<"Into network: "<<networkName<<endl;
					cout<<"--cipher: "<<cipherMode<<endl;

					string request;
					string response;
					string networkID;

					if (!aps.IsNull() && aps.IsArray()){
						for (SizeType j=0; j<aps.Size(); j++){
							//4.1 iterate through access point to get those in range
							Value& ap = aps[j];
							if (!ap["lat"].IsNull() && !ap["long"].IsNull()){
								GeoLocation* apLocation = new GeoLocation(ap["lat"].GetFloat(), ap["long"].GetFloat());
								float distance = geoTracker->getDistance(apLocation, currentLocation);
								if (distance <= MAX_RANGE){
									//try to connect by each interface
									for (WifiInterface* interface : interfaces){
										//create control wrapper
										WpaControlWrapper* wpaControl = new WpaControlWrapper(interface);
																			
										//add network, response contains network ID
										request = string("ADD_NETWORK");
										wpaControl->request(request, response);									
										networkID = string(response);									
									
										//set network ssid
										request = string("SET_NETWORK ") + response;
										wpaControl->request(request + string(" ssid \"") + networkName + string("\""), response);
									
										//set cipher parameters
										if (cipherMode == "WPA"){
											wpaControl->request(request + string(" psk \"") + net["encryption"]["psk"].GetString() + string("\""), response);
										}
										else if (cipherMode == "EAP"){
											wpaControl->request(request + string(" username \"") + net["encryption"]["username"].GetString() + string("\""), response);
											wpaControl->request(request + string(" password \"") + net["encryption"]["password"].GetString() + string("\""), response);
										}
										
										//start sniffing process
										WifiNetwork* wifiNetwork = new WifiNetwork(networkName);
										
										syncMutex.lock();
										//this thread will unlock the mutex
										thread sniffing(packet_sniffing, interface, wifiNetwork, wifi_connection_analyser);
										
										syncMutex.lock(); //hang here until sniffing thread ready to enter its loop										
										//enable this network then wait for result
										request = string("ENABLE_NETWORK ") + networkID;
										wpaControl->request(request, response);
										syncMutex.unlock();
										
										//join the sniffing thread
										sniffing.join();
										cout<<"In main: sniffing thread terminated\n";

										//disable this network on this interface
										request = string("DISABLE_NETWORK ") + networkID;
										wpaControl->request(request, response);
																	
									}
									//at least one ap of this network found, break
									break;
								}							
							}
						}
					}								
				}				
			}
		}

		//kill children wpa processes
		for (int i : cpid){
			if (i>0){
				cout<<"Sending sigkill to "<<i<<endl;
				kill(i, SIGKILL);
			}
		}
		
		//wait for children processes to terminate
		int status;
		while (pCount>0){
			pid = wait(&status);
			if (pid>0){
				cout<<"Process id: "<<pid<<" returned"<<endl;
			}
			else{
				cout<<"Child process returned with error"<<endl;
			}
			pCount--;
		}
		
		//remove virtual device
		for (WifiInterface* vi : virtualInterfaces){
			cout<<"Removing vitual interface: "<<vi->getName()<<endl;
			vi->getDevice()->removeVirtualInterface(vi);
		}				
	
		return EXIT_SUCCESS;
	}
}

int main(int argc, char** argv){
	return report();
}









